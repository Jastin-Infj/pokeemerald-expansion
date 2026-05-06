#!/usr/bin/python3

"""
Checks that all documentation pages that should be mentioned in
`docs/SUMMARY.md` are mentioned the file
"""

import glob
import re
import os
import sys
from pathlib import Path

errorLines = []
warnOnly = False

for arg in sys.argv[1:]:
    if arg == "--warn-only":
        warnOnly = True
    else:
        errorLines.append(f"Unexpected argument: {arg}")

if not os.path.exists("Makefile"):
    errorLines.append("Please run this script from your root folder.")

summaryFile = Path("docs/SUMMARY.md")
allowlistFile = Path(".github/docs_validate/summary_allowlist.txt")


def read_allowlist():
    allowed = set()
    if not allowlistFile.is_file():
        return allowed

    with open(allowlistFile, 'r', encoding='UTF-8') as file:
        lineNo = 0
        for rawLine in file:
            lineNo = lineNo + 1
            line = rawLine.split("#", 1)[0].strip()
            if not line:
                continue

            path = Path(line)
            if path.is_absolute() or ".." in path.parts:
                errorLines.append(
                    f"{allowlistFile}:{lineNo}: invalid docs path `{line}`"
                )
            else:
                allowed.add(path)

    return allowed


summaryAllowlist = set()
if not errorLines:
    if not summaryFile.is_file():
        errorLines.append("docs/SUMMARY.md missing")
    else:
        summaryAllowlist = read_allowlist()

summaryContents = []
if not errorLines:
    with open(summaryFile, 'r', encoding='UTF-8') as file:
        entry_pattern = re.compile(r" *\- \[[^\]]*\]\(([^\)]*)\)\n")
        lineNo = 0
        while line:=file.readline():
            lineNo = lineNo + 1
            if line == "# Summary\n" or line == "\n":
                pass
            elif match:=entry_pattern.match(line):
                if "" != match.group(1):
                    summaryContents.append(Path(match.group(1)))
            else:
                if not errorLines:
                    errorLines.append("## Unexpected lines in docs/SUMMARY.md")
                errorLines.append(f"- {lineNo}: {line.strip()}")

if not errorLines:
    for path in sorted(summaryAllowlist):
        if not (Path("docs") / path).is_file():
            errorLines.append(
                "summary allowlist points at a missing docs file: " + str(path)
            )

if not errorLines:
    for pathName in glob.glob("**/*.md", root_dir="docs", recursive=True):
        path = Path(pathName)
        if path == Path("SUMMARY.md"):
            pass
        elif path == Path("changelogs/template.md"):
            pass
        elif path in summaryContents:
            pass
        elif path in summaryAllowlist:
            pass
        else:
            if not errorLines:
                errorLines.append("## `docs/**/*.md` files present but not mentioned in `docs/SUMMARY.md`")
            errorLines.append("- " + str(path))

if errorLines:
    for line in errorLines:
        print(line)

    if 'GITHUB_STEP_SUMMARY' in os.environ:
        with open(os.environ['GITHUB_STEP_SUMMARY'], 'w', encoding='UTF-8') as file:
            for line in errorLines:
                file.write(line)
                file.write('\n')

    if warnOnly:
        print("docs_validate: warning-only mode; not failing this branch check.")
        quit(0)

    quit(1)
