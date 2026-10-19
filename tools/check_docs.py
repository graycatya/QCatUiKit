#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Check repository documentation locally, without Qt or third-party packages."""

from pathlib import Path
import re
import sys
import unicodedata
from urllib.parse import unquote, urlsplit


ROOT = Path(__file__).resolve().parents[1]
DOC = ROOT / "src/uikit/doc"


def markdown_files():
    # Deliberately exclude local build/stage trees and editor state.
    return sorted({*ROOT.glob("*.md"), *DOC.rglob("*.md"),
                   *(ROOT / "skills").rglob("*.md"),
                   *(ROOT / "examples").rglob("*.md")})


def prose(path, errors):
    """Keep line numbers while excluding fenced code from link checks."""
    result = []
    fence = None
    opening = 0
    for number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        match = re.match(r"^\s{0,3}(`{3,}|~{3,})(.*)$", line)
        if match:
            marks, tail = match.groups()
            if fence is None:
                fence, opening = marks, number
            elif marks[0] == fence[0] and len(marks) >= len(fence) and not tail.strip():
                fence = None
            result.append("")
        else:
            result.append("" if fence else line)
    if fence:
        errors.append(f"{path.relative_to(ROOT)}:{opening}: unclosed code fence")
    return result


def anchors(lines):
    found = set()
    counts = {}
    for line in lines:
        match = re.match(r"^#{1,6}\s+(.+?)\s*#*\s*$", line)
        if not match:
            continue
        title = re.sub(r"\[([^]]+)\]\([^)]*\)", r"\1", match[1]).lower()
        slug = "".join(c for c in title if c in "_- " or
                       unicodedata.category(c)[0] in "LN").replace(" ", "-")
        count = counts.get(slug, 0)
        counts[slug] = count + 1
        found.add(f"{slug}-{count}" if count else slug)
    return found


def main():
    errors = []
    paths = markdown_files()
    texts = {p.resolve(): prose(p, errors) for p in paths}
    headings = {p: anchors(lines) for p, lines in texts.items()}
    links = 0
    for path, lines in texts.items():
        for number, line in enumerate(lines, 1):
            targets = re.findall(r"!?\[[^]\n]*\]\(\s*(<[^>]+>|[^\s)]+)", line)
            targets += re.findall(r"\b(?:src|href)=[\"']([^\"']+)[\"']", line)
            # Reference definitions, if a document uses them.
            definition = re.match(r"^\s*\[[^]]+\]:\s*(<[^>]+>|\S+)", line)
            if definition:
                targets.append(definition[1])
            for raw in targets:
                target = raw.strip("<>")
                parts = urlsplit(target)
                if parts.scheme or parts.netloc:
                    continue
                links += 1
                dest = (path.parent / unquote(parts.path)).resolve() if parts.path else path
                message = None
                if not dest.exists():
                    message = f"missing local target: {target}"
                elif parts.fragment and dest.suffix.lower() == ".md":
                    if dest not in headings:
                        headings[dest] = anchors(prose(dest, errors))
                    if unquote(parts.fragment) not in headings[dest]:
                        message = f"missing heading anchor: {target}"
                if message:
                    errors.append(f"{path.relative_to(ROOT)}:{number}: {message}")

    cmake = (ROOT / "src/uikit/CMakeLists.txt").read_text(encoding="utf-8")
    types = set(re.findall(r"qml/(Cat\w+)\.qml", cmake))
    for header in (ROOT / "src/uikit").glob("*.h"):
        if header.name.endswith("_p.h"):
            continue
        types.update(re.findall(r"QML_NAMED_ELEMENT\((Cat\w+)\)", header.read_text(encoding="utf-8")))
    catalog = (DOC / "COMPONENTS.md").read_text(encoding="utf-8")
    missing = types - set(re.findall(r"\bCat\w+\b", catalog))
    if missing:
        errors.append("COMPONENTS.md: undocumented public types: " + ", ".join(sorted(missing)))
    for path in [ROOT / "README.md", ROOT / "README.en.md", DOC / "README.md", DOC / "COMPONENTS.md"]:
        content = path.read_text(encoding="utf-8")
        match = re.search(r"\b(\d+) (?:个 )?Cat", content)
        if not match or int(match[1]) != len(types):
            errors.append(f"{path.relative_to(ROOT)}: expected Cat type count {len(types)}")
    if errors:
        print("\n".join(errors))
        return 1
    print(f"Checked {len(paths)} Markdown files, {links} local links, code fences, and {len(types)} public Cat types.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
