#!/usr/bin/env python3
"""Normalize and sort includes across the Mantle codebase.

Scans unguarded include blocks (no #if/#ifdef/#else/#elif/#endif between
the includes). Preprocessor-guarded includes are left untouched.

You can temporarily disable processing for specific sections:

    // fix-includes off
    #include "this/will/not/be/sorted.h"
    #include <will/not/be/sorted.hpp>
    // fix-includes on

Usage:
    python3 scripts/fix_includes.py --fix      # rewrite files in-place
    python3 scripts/fix_includes.py --check    # exit 1 if any file needs fixing
"""

import argparse
import re
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

STD_HEADERS = {
    "algorithm", "any", "array", "atomic", "barrier", "bit", "bitset",
    "cassert", "cctype", "cerrno", "cfenv", "cfloat", "charconv",
    "chrono", "cinttypes", "ciso646", "climits", "clocale", "cmath",
    "codecvt", "compare", "complex", "concepts", "condition_variable",
    "coroutine", "csetjmp", "csignal", "cstdarg", "cstdbool", "cstddef",
    "cstdint", "cstdio", "cstdlib", "cstring", "ctime", "cuchar",
    "cwchar", "cwctype", "deque", "exception", "execution", "expected",
    "filesystem", "flat_map", "flat_set", "forward_list", "fstream",
    "functional", "future", "generator", "initializer_list", "iomanip",
    "ios", "iosfwd", "iostream", "istream", "iterator", "latch",
    "limits", "list", "locale", "map", "mdspan", "memory", "memory_resource",
    "mutex", "new", "numbers", "numeric", "optional", "ostream",
    "print", "queue", "random", "ranges", "ratio", "regex",
    "scoped_allocator", "semaphore", "set", "shared_mutex", "span",
    "sstream", "stack", "stacktrace", "stdexcept",
    "streambuf", "string", "string_view", "strstream", "syncstream",
    "system_error", "thread", "tuple", "type_traits", "typeindex",
    "typeinfo", "unordered_map", "unordered_set", "utility", "valarray",
    "variant", "vector", "version",
}

THIRD_PARTY_PREFIXES = [
    "spdlog", "glm", "flecs", "SDL", "SDL3",
    "imgui", "tlsf", "vma", "vulkan", "Vulkan",
    "lua", "LuaJIT", "sol", "Jolt",
]

INCLUDE_RE = re.compile(r'#include\s+([<"])([^>"]+)([>"])')
GUARD_RE = re.compile(r'#\s*(if|ifdef|ifndef|else|elif|endif)\b')

BLANK_OR_COMMENT_RE = re.compile(r'^\s*(//|/\*|\*|$)')


def get_source_files() -> list[Path]:
    result = subprocess.run(
        ["git", "-C", str(REPO_ROOT), "ls-files", "-c", "-o", "--exclude-standard"],
        capture_output=True, text=True, check=True,
    )
    files = [f.strip() for f in result.stdout.split("\n") if f.strip()]
    return [REPO_ROOT / f for f in files
            if f.endswith((".h", ".hpp", ".cpp", ".c"))
            and not f.startswith("third_party")]


def classify_include(path_str: str, is_angle: bool) -> str:
    first = path_str.split("/")[0]
    for prefix in THIRD_PARTY_PREFIXES:
        if first == prefix or path_str.startswith(prefix + "/"):
            return "third_party"
    if is_angle:
        if "/" not in path_str:
            return "std"
        return "third_party"
    return "project"


def normalize_path(path_str: str) -> str:
    parts = path_str.split("/")
    result = []
    for p in parts:
        if p == "..":
            if result:
                result.pop()
        elif p == ".":
            continue
        else:
            result.append(p)
    return "/".join(result)


def format_includes(inc_tuples, own_stem):
    """Format a list of (path_str, is_angle_original, raw_line) into sorted groups.

    Groups are emitted in order: own, std, third_party, project.
    A blank line is placed between std and third_party if both are present.
    """
    grouped = {"own": [], "std": [], "third_party": [], "project": []}

    for path_str, is_angle, _ in inc_tuples:
        norm = normalize_path(path_str)
        if own_stem is not None and Path(norm).stem == own_stem:
            grouped["own"].append((norm, False, path_str))
            continue

        cat = classify_include(path_str, is_angle)
        want_angle = cat in ("std", "third_party")
        grouped[cat].append((norm, want_angle, path_str))

    out = []
    prev = False

    # Own (matching the current file's stem)
    if grouped["own"]:
        for norm, angle, orig in sorted(grouped["own"]):
            q = "<" if angle else '"'
            out.append(f'#include {q}{orig}{">" if angle else '"'}')
        prev = True

    # Standard library includes
    if grouped["std"]:
        if prev:
            out.append("")
        for norm, angle, orig in sorted(grouped["std"]):
            q = "<" if angle else '"'
            out.append(f'#include {q}{orig}{">" if angle else '"'}')
        prev = True

    # Third-party includes (blank line before if std was present)
    if grouped["third_party"]:
        if prev:
            out.append("")
        for norm, angle, orig in sorted(grouped["third_party"]):
            q = "<" if angle else '"'
            out.append(f'#include {q}{orig}{">" if angle else '"'}')
        prev = True

    # Project includes (everything else)
    if grouped["project"]:
        if prev:
            out.append("")
        for norm, angle, orig in sorted(grouped["project"]):
            q = "<" if angle else '"'
            out.append(f'#include {q}{orig}{">" if angle else '"'}')

    return out


def process_file(file_path: Path, fix: bool) -> bool:
    original = file_path.read_text(encoding="utf-8")
    lines = original.split("\n")
    n = len(lines)
    own_stem = file_path.stem

    changed = False
    result: list[str] = []

    i = 0
    skip_includes = False

    while i < n:
        line = lines[i]
        stripped = line.strip()

        # Check for special markers that toggle include processing
        if "fix-includes off" in line:
            skip_includes = True
            result.append(line)
            i += 1
            continue
        if "fix-includes on" in line:
            skip_includes = False
            result.append(line)
            i += 1
            continue

        if skip_includes:
            result.append(line)
            i += 1
            continue

        m = INCLUDE_RE.match(stripped)
        if not m:
            result.append(line)
            i += 1
            continue

        start = i
        incs = [(m.group(2), m.group(3) == ">", line)]
        last_inc = i

        j = i + 1
        block_interrupted = False
        while j < n:
            l = lines[j]
            s = l.strip()

            if "fix-includes off" in l or "fix-includes on" in l:
                if incs:
                    old_text = "\n".join(inc[2] for inc in incs)
                    formatted = format_includes(incs, own_stem)
                    new_text = "\n".join(formatted)

                    if old_text.strip() != new_text.strip():
                        changed = True

                    if result and result[-1] != "":
                        result.append("")
                    result.extend(formatted)

                    gap_end = j
                    while gap_end > last_inc + 1 and not lines[gap_end - 1].strip():
                        gap_end -= 1
                    gap_count = j - gap_end
                    if gap_count > 0 and result[-1] != "":
                        result.append("")

                i = j
                block_interrupted = True
                break

            m2 = INCLUDE_RE.match(s)
            if m2:
                incs.append((m2.group(2), m2.group(3) == ">", l))
                last_inc = j
                j += 1
            elif s == "" or s.startswith("//") or s.startswith("/*"):
                j += 1
            elif GUARD_RE.match(s):
                incs = None
                break
            else:
                break

        if block_interrupted:
            # The outer loop will re-enter with i pointing to the marker line
            continue

        if incs is None:
            for k in range(start, j):
                result.append(lines[k])
            i = j
            continue

        end = j

        old_text = "\n".join(inc[2] for inc in incs)
        formatted = format_includes(incs, own_stem)
        new_text = "\n".join(formatted)

        if old_text.strip() != new_text.strip():
            changed = True

        if result and result[-1] != "":
            result.append("")
        result.extend(formatted)

        gap_end = end
        while gap_end > last_inc + 1 and not lines[gap_end - 1].strip():
            gap_end -= 1
        gap_count = end - gap_end
        if gap_count > 0 and result[-1] != "":
            result.append("")

        i = end

    new_text = "\n".join(result)

    if original.endswith("\n") and not new_text.endswith("\n"):
        new_text += "\n"
    elif not original.endswith("\n") and new_text.endswith("\n"):
        new_text = new_text.rstrip("\n")

    if new_text == original:
        return False

    if fix:
        file_path.write_text(new_text, encoding="utf-8")
        print(f"FIXED  {file_path.relative_to(REPO_ROOT)}")
    else:
        print(f"ISSUE  {file_path.relative_to(REPO_ROOT)}")

    return True


def main() -> None:
    parser = argparse.ArgumentParser(description="Normalize and sort includes")
    parser.add_argument("--fix", action="store_true", help="Rewrite files in-place")
    parser.add_argument("--check", action="store_true", help="Check-only (default)")
    args = parser.parse_args()

    files = get_source_files()
    files.sort()

    changed = 0
    for f in files:
        if process_file(f, fix=args.fix):
            changed += 1

    if changed:
        print(f"\n{changed} file(s) {'fixed' if args.fix else 'need fixing'}")
    else:
        print("All clean!")

    if not args.fix and changed > 0:
        sys.exit(1)


if __name__ == "__main__":
    main()
