# scripts/

所有腳本都從 repo 根目錄執行。

| 腳本 | 用途 | 在哪裡執行 | 附註 |
|---|---|---|---|
| `format.sh` | 用 clang-format 排版 `include/ src/ tests/ bench/ examples/` 的 C++ 原始碼；加 `--check` 只檢查不改檔 | Docker dev image | 不同版本的 clang-format 排出的結果可能不同，用 dev image 才和 CI 一致。CI job：`linux / format` |
| `coverage.sh` | 以 `--coverage` 建置並跑測試，用 gcovr 統計 `src/`、`include/` 的覆蓋率 | Docker dev image | 需要 gcc 和 gcovr。行覆蓋率低於 85% 就失敗，可用環境變數 `MIN_LINE_COVERAGE` 調整。報告在 `build/Linux/coverage/report/`。CI job：`linux / coverage` |
| `bench.sh` | 建置並執行 Google Benchmark（重複 20 次、隨機穿插順序），結果寫成 JSON | Docker bench image | 第一個參數是輸出路徑（預設 `build/<OS>/bench/result.json`），其餘參數傳給 benchmark。只比較同一台機器、同一個 image 的結果。CI 只確認能編譯，不會執行 |
| `check_readme_tree.py` | 檢查根目錄 README 的 Directory Structure 和實際資料夾是否一致 | 本機直接執行 | 只需要 python3 和 git。增加、移動或刪除檔案後執行。CI job：`readme / directory structure` |

## 指令

```
# 第一次先建 image
$ docker build --target dev -t log-hare-tail:dev .
$ docker build --target bench -t log-hare-tail:bench .

$ docker run --rm -v "$PWD:/workspace" log-hare-tail:dev scripts/format.sh
$ docker run --rm -v "$PWD:/workspace" log-hare-tail:dev scripts/format.sh --check
$ docker run --rm -v "$PWD:/workspace" log-hare-tail:dev scripts/coverage.sh
$ docker run --rm -v "$PWD:/workspace" log-hare-tail:bench scripts/bench.sh bench/results/before.json
$ scripts/check_readme_tree.py
```

比較兩次 benchmark 結果的方法見根目錄 README 的 Benchmarks 段落。
