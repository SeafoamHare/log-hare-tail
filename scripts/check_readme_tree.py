#!/usr/bin/env python3
# Checks that the tree under "## Directory Structure" in README.md matches the repository:
# - every path it lists exists (directories end with "/")
# - a directory it expands (lists anything inside) lists every file and directory in it,
#   while a directory it does not expand only has to exist
# - every top-level directory is listed (top-level files are not required)
# Dotfiles and dot-directories are skipped, and so is anything .gitignore ignores.
# CI runs it on every push and pull request; locally, from the repository root:
#   scripts/check_readme_tree.py
import os
import re
import subprocess
import sys

README = "README.md"
HEADING = "## Directory Structure"
# One tree level is 4 characters: "│   ", "    ", "├── " or "└── ".
# The comment starts at the first "#" after whitespace, so it may contain "#include"
LINE = re.compile(r"^(?P<prefix>(?:│   |    |├── |└── )*)(?P<name>\S+)(?:\s+#.*)?$")


def read_tree():
    """Returns {path: is_dir} for every entry in the README tree, intermediate directories included."""
    with open(README, encoding="utf-8") as f:
        text = f.read()
    start = text.find(HEADING)
    if start < 0:
        sys.exit(f'{README}: no "{HEADING}" section')
    block = re.search(r"^```[^\n]*\n(.*?)^```", text[start:], re.M | re.S)
    if not block:
        sys.exit(f"{README}: no code block under {HEADING}")

    entries = {}
    parents = []  # parents[depth] is the path of the last entry seen at that depth
    for line in block.group(1).splitlines():
        if not line.strip():
            continue
        m = LINE.match(line)
        if not m:
            sys.exit(f"{README}: cannot read tree line: {line}")
        depth = len(m.group("prefix")) // 4
        if depth > len(parents):
            sys.exit(f"{README}: tree line is indented deeper than its parent: {line}")
        name = m.group("name")
        is_dir = name.endswith("/")
        path = "/".join(filter(None, parents[depth - 1 : depth] + [name.rstrip("/")]))
        del parents[depth:]
        parents.append(path)
        entries[path] = is_dir
        # An entry such as include/log_hare_tail/ also lists include/
        parent = os.path.dirname(path)
        while parent:
            entries.setdefault(parent, True)
            parent = os.path.dirname(parent)
    return entries


def repo_files():
    """Tracked and untracked files that are not ignored and still exist."""
    out = subprocess.run(
        ["git", "ls-files", "--cached", "--others", "--exclude-standard", "-z"],
        check=True, capture_output=True, text=True,
    ).stdout
    return {f for f in out.split("\0") if f and os.path.isfile(f)}


def children(files, directory):
    """{name: is_dir} for the visible files and directories directly inside directory."""
    prefix = directory + "/" if directory else ""
    result = {}
    for f in files:
        if not f.startswith(prefix):
            continue
        rest = f[len(prefix):].split("/")
        if not rest[0].startswith("."):
            result[rest[0]] = len(rest) > 1
    return result


def main():
    os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
    entries = read_tree()
    files = repo_files()
    dirs = set()
    for f in files:
        d = os.path.dirname(f)
        while d:
            dirs.add(d)
            d = os.path.dirname(d)
    errors = []

    for path, is_dir in sorted(entries.items()):
        if is_dir and path not in dirs:
            errors.append(f"{path}/ is listed but is not a directory in the repository")
        elif not is_dir and path not in files:
            errors.append(f"{path} is listed but is not a file in the repository")

    expanded = {os.path.dirname(p) for p in entries}
    for directory in sorted(expanded):
        if directory and directory not in dirs:
            continue  # already reported above
        for name, is_dir in sorted(children(files, directory).items()):
            path = f"{directory}/{name}" if directory else name
            if directory == "" and not is_dir:
                continue
            if path not in entries:
                where = f"under {directory}/" if directory else "at the top level"
                errors.append(f"{path}{'/' if is_dir else ''} exists but is not listed {where}")

    if errors:
        print(f"{README} {HEADING} does not match the repository:", file=sys.stderr)
        for e in errors:
            print(f"  {e}", file=sys.stderr)
        sys.exit(1)
    print(f"{README}: directory structure matches the repository")


if __name__ == "__main__":
    main()
