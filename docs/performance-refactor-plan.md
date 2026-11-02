# 效能改善與架構重構計劃

將 Logger 調整為 producer/consumer 架構，並以 Pipe-filter pattern 組織處理流程，使程式可重用、可組合且低延遲。
修改架構之前，先定義可量測的品質屬性，並建立 benchmark 取得基準線，最後用同一套 benchmark 比較新舊架構。

## 1. 量測（改善）目標

### 1.1 日誌等級關閉時的 LOG 呼叫成本（主要目標）

| 項目 | 目標 |
|---|---|
| 每次呼叫成本，中位數（熱快取） | ≤ 2 ns |
| 每次呼叫成本，p99（熱快取） | ≤ 5 ns |
| 記憶體配置 | 0 次 |
| 上鎖 | 0 次 |
| 系統呼叫 | 0 次 |
| `<<` 後的參數 | 不求值 |

以品質屬性情境表示：

- **來源**：應用程式的執行緒
- **刺激**：呼叫一個低於目前執行期等級的 `LOG_xxx`
- **標的物**：LOG 巨集與等級檢查（producer 端）
- **環境**：正常運作、熱快取、Release 建置（`-O2`），在 `log-hare-tail:bench` container 內量測
- **回應**：不格式化、不排入佇列、不求值參數，直接返回
- **衡量**：如上表

說明：

- 目前程式沒有等級過濾，`LOG_DEBUG` 一定會走完格式化、上鎖、輸出。原生 macOS（Apple M3）實測約 6.5 µs，所以這個目標同時包含「新增執行期等級設定」這個功能。
- 等級過濾必須在 producer 端、巨集展開時完成，不能只當成 consumer 端 pipeline 的一個 filter；否則被過濾的紀錄仍要付格式化與排入佇列的成本。
- 每次呼叫只有數奈秒，比計時器本身的開銷還小，無法逐次計時。這裡的「中位數 / p99」指的是**多次 repetition 的每次呼叫平均值**的中位數與 p99（見 3.3）。

### 1.2 新架構整體延遲要低於原本的

「整體延遲」拆成兩個量測，數字門檻在第 2 階段取得基準線後填入：

| 指標 | 定義 | 目標 |
|---|---|---|
| 呼叫端延遲（等級開啟） | 呼叫 `LOG_INFO(...) << ... << std::endl` 到返回的時間，也就是應用程式執行緒被佔用的時間 | p50、p99、p99.9 皆低於基準線（待填） |
| 多執行緒吞吐量 | 1 / 2 / 4 / 8 個執行緒同時記錄時，每秒完成的 LOG 呼叫數 | 高於基準線（待填） |
| 端到端延遲 | 從呼叫 LOG 到紀錄寫入檔案的時間 | 不高於基準線的 p99（待填） |

非同步架構通常是用「端到端延遲」換取「呼叫端延遲」，因此端到端延遲只要求不變差，而不要求變好。

### 1.3 確定性條件（以 gtest 把關）

奈秒數字容易受雜訊影響，下列條件改用單元測試驗證，作為 CI 的關卡：

- 被過濾的 LOG 呼叫不配置記憶體（以自訂 `operator new` 計數驗證）。
- 被過濾的 LOG 呼叫不求值參數（以帶副作用的參數驗證）。
- 正常負載下，不遺失任何紀錄；程式正常結束時，佇列中的紀錄都會被寫出。

## 2. 流程

### 階段 1：引入與建立 benchmark

目標：建立修改前後都能編譯、執行的同一套 benchmark，並且**在重構前單獨合併進 main**，否則修改前的版本沒有 benchmark 可以量測。

- [x] 以 `FetchContent` 引入 Google Benchmark，並用 `URL_HASH` 固定版本（與 googletest 相同做法）。
- [x] 新增 CMake 選項 `LOGGER_BUILD_BENCHMARKS`（預設 OFF）與 `Logger/bench/` 目錄。
- [x] Benchmark 只透過公開的 `LOG_xxx` 巨集呼叫 logger：
  - `BM_DisabledDebug`：被關閉等級的呼叫。
  - `BM_DisabledDebugExpensiveArgs`：參數含 `std::to_string(...)`，驗證參數未被求值。
  - `BM_EnabledInfo`：等級開啟、已 `initialize()`、寫入暫存目錄的檔案，stdout 導向 `/dev/null`。
  - `BM_EnabledInfo` 多執行緒版本：`->Threads(1)->Threads(2)->Threads(4)->Threads(8)`。
  - `BM_Baseline`：空迴圈，用來扣除量測本身的開銷。
- [ ] 新增呼叫端延遲分布的量測程式（逐次計時並記錄到 histogram，輸出 p50 / p99 / p99.9），以及端到端延遲的量測。Google Benchmark 只提供 repetition 之間的統計，無法取得逐次呼叫的百分位數。
- [x] 新增 `bench` CMake preset：`CMAKE_BUILD_TYPE=Release`、`LOGGER_BUILD_BENCHMARKS=ON`、`LOGGER_BUILD_TESTS=OFF`，不使用 `-march=native`。
- [x] Dockerfile 新增 `bench` stage（以 `dev` 為基礎，加裝 `python3-scipy` 給 `compare.py` 使用）。
- [ ] base image 改用 digest 固定（`ubuntu:24.04@sha256:...`），避免編譯器版本悄悄改變。
- [x] 新增 `scripts/bench.sh`，固定執行參數：`--benchmark_repetitions=20 --benchmark_enable_random_interleaving=true --benchmark_out_format=json`。

完成條件：`docker run ... log-hare-tail:bench scripts/bench.sh` 能在目前的 main 上產出 JSON 結果。

### 階段 2：量測目前在 container 環境下的延遲（基準線）

- [ ] 在 container 內執行 benchmark，產出 `Logger/bench/results/baseline.json`。
- [ ] 記錄量測環境：主機 CPU 與作業系統、Docker 版本、image digest、編譯器版本、commit SHA。
- [ ] 將基準線數字填入 1.2 的表格，訂出新架構的具體門檻。

量測注意事項：

- 修改前後必須在**同一台機器、同一個 image、同一段時間**內量測，只比較相對差異。
- Linux 主機：以 `--cpuset-cpus` 將 benchmark 綁在單一核心，並在主機上把 CPU governor 設成 `performance`。
- Mac（Docker Desktop）：container 跑在 Linux VM 內，`--cpuset-cpus` 綁的是虛擬 CPU，效果有限；主要依靠 20 次 repetition、random interleaving 與中位數降低雜訊。原生 macOS 的數字不可與 container 內的數字混用。

### 階段 3：新架構設計

架構方向：

```
[Producer 端：應用程式執行緒]          [Consumer 端：背景執行緒]
LOG 巨集 → 等級檢查 → 擷取紀錄 → 佇列 → Filter → Formatter → Sink(s)
            (≤ 2 ns 被過濾)               (Pipe-filter pipeline，可組合)
```

- **Producer 端**：巨集內做等級檢查（`std::atomic` + `memory_order_relaxed`，等級變數以 `alignas(64)` 獨佔一條 cache line），只擷取紀錄所需的資料，不做格式化。另提供編譯期門檻（例如 `LOGGER_ACTIVE_LEVEL`），低於門檻的呼叫直接編譯為空。
- **佇列**：連接 producer 與 consumer。
- **Consumer 端**：以 Pipe-filter 組成處理流程，每個 filter 有一致的介面，可重用、可替換、可組合（例如 Level filter、JSON / Plain formatter、Console / File sink）。

需要決策的設計議題：

| 議題 | 選項 |
|---|---|
| 佇列型式 | 單一 MPSC 佇列 / 每個執行緒一個 SPSC ring buffer |
| 佇列滿時的策略 | 阻塞等待 / 丟棄並計數 / 可設定 |
| 格式化時機 | Producer 端先轉成字串 / Consumer 端延後格式化 |
| 程式結束與當機時的 flush | 解構時排空佇列 / 提供 `flush()` API / 處理 signal |
| 多 sink 的等級門檻 | 全域單一門檻 / 每個 sink 各自的 Level filter |
| C++ 標準 | README 承諾函式庫支援 C++11，新設計需維持或明確調整這個承諾 |

- [ ] 撰寫設計文件，對上表每個議題做出決策並說明取捨。
- [ ] 補上 1.3 的確定性測試。
- [ ] 實作新架構，並通過既有的 gcc / clang / tsan / asan / tidy / coverage 檢查。
- [ ] 檢查既有測試中依賴同步輸出的部分（例如寫完立即讀檔），改為先 flush 再驗證。

### 階段 4：新舊架構比較

- [ ] 在同一個 container、同一台機器上，分別量測基準線版本與新架構版本（以 `git worktree` 取得基準線版本）。
- [ ] 以 `compare.py benchmarks before.json after.json` 比較，確認差異通過 U 檢定（p < 0.05）。
- [ ] 逐項對照第 1 節的目標，產出比較報告：數字、是否達標、未達標的原因與後續處理。
- [ ] （選擇性）CI 加入 `benchmark-action/github-action-benchmark` 追蹤趨勢，門檻放寬為退步超過 2 倍才警告；共用 runner 雜訊大，不以奈秒數字作為 CI 關卡。

## 3. 量測方法

### 3.1 工具

- **Google Benchmark**：微基準測試，量測每次呼叫的平均成本與吞吐量。
- **`tools/compare.py`**（Google Benchmark 內附）：比較兩份 JSON 結果，輸出變化比例 `(new - old) / |old|` 與 Mann-Whitney U 檢定。官方建議至少 9 次 repetition，本計劃使用 20 次。
- **自訂延遲量測程式**：逐次計時，取得呼叫端與端到端延遲的百分位數。

### 3.2 參考

- spdlog `bench/latency.cpp`：使用 Google Benchmark，包含 `disabled-at-runtime` 與 `disabled-at-compile-time` 的寫法，可作為 benchmark 的範本與橫向參考。
- odygrd/logger_benchmarks：量測多個 logger 等級開啟時的呼叫端延遲百分位數，可作為 1.2 量測方法的參考。

### 3.3 百分位數的定義

| 量測 | 單次成本 | 百分位數的計算方式 |
|---|---|---|
| 等級關閉的呼叫 | 數奈秒，小於計時器開銷 | 每次 repetition 量測大量呼叫取平均，再對 20 次 repetition 的平均值取中位數與 p99 |
| 等級開啟的呼叫 | 數百奈秒到數微秒 | 逐次計時，直接取 p50 / p99 / p99.9 |

## 4. 風險

| 風險 | 因應 |
|---|---|
| Mac 上 container 跑在 VM 內，雜訊大、絕對數字失真 | 只做相對比較；需要絕對數字時改在 Linux 主機量測 |
| 非同步後，LOG 返回時紀錄尚未落地，當機可能遺失紀錄 | 在設計階段決定 flush 與 signal 處理策略 |
| 佇列滿時阻塞會讓呼叫端延遲的 p99.9 飆高 | 在 benchmark 中加入突發大量記錄的情境 |
| benchmark 被編譯器最佳化掉，量到不真實的數字 | 使用 `benchmark::DoNotOptimize`，並以 `BM_Baseline` 交叉檢查 |
