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
                   *(ROOT / "examples").rglob("*.md"),
                   ROOT / "adapters/qcatthorvgqrhi/README.md"})


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


def duplicate_paragraphs(path, lines, errors):
    """Reject repeated prose in current guides."""
    seen = {}
    paragraph = []

    def check():
        if not paragraph:
            return
        number = paragraph[0][0]
        value = " ".join(line.strip() for _, line in paragraph)
        # Headings, tables and lists can legitimately reuse standard labels.
        if len(value) >= 30 and not re.match(r"^(?:[#|>]|[-*+]\s|\d+[.)]\s)", value):
            if value in seen:
                errors.append(f"{path.relative_to(ROOT)}:{number}: duplicate prose paragraph "
                              f"(first at line {seen[value]})")
            else:
                seen[value] = number
        paragraph.clear()

    for number, line in enumerate(lines, 1):
        if line.strip():
            paragraph.append((number, line))
        else:
            check()
    check()


def public_api_mentions(cmake, errors):
    """Check name coverage in topic guides, not behavioral or signature correctness."""
    headers = re.search(r"set\(qcat_public_headers\s+(.*?)\)", cmake, re.S)[1].split()
    property_count = method_count = 0
    for name in headers:
        code = (ROOT / "src/uikit" / name).read_text(encoding="utf-8")
        name = Path(name).name
        if name.startswith("qcatchart") or name == "qcatwaterfall.h":
            topics = ["CHARTS.md", "CHARTS3D.md", "CHARTEXTENSIONS.md",
                      "CHARTSTATISTICS.md", "COMPONENTS.md"]
        elif name.startswith("qcatled") or name == "qcatlightingframe.h":
            topics = ["LEDPREVIEW.md", "LEDWORKFLOW.md"]
        elif name == "qcatthemepalette.h":
            topics = ["THEMING.md"]
        elif name in ("qcattablemodel.h", "qcatchatmodel.h"):
            topics = ["MODELS.md"]
        elif name == "qcatvectoritem.h":
            topics = ["VECTOR.md"]
        elif name == "qcattiltcontroller.h":
            topics = ["ATTITUDE.md"]
        elif name == "qcatdockcontroller.h":
            topics = ["DOCKING.md"]
        else:
            topics = ["VERSIONING.md", "ABI.md"]
        text = "\n".join((DOC / topic).read_text(encoding="utf-8") for topic in topics)
        properties = []
        for macro in re.findall(r"Q_PROPERTY\((.*?)\)", code, re.S):
            match = re.search(r"\b(\w+)\s+(?:READ|MEMBER)\b", macro)
            if match:
                properties.append(match[1])
        methods = re.findall(r"Q_INVOKABLE\s+[\w:*<> ]+?\s+(\w+)\s*\(", code)
        property_count += len(properties)
        method_count += len(methods)
        missing = sorted({name for name in properties + methods
                          if not re.search(r"\b" + re.escape(name) + r"\b", text)})
        if missing:
            errors.append(f"{name}: API names missing from topic guides: {', '.join(missing)}")
    return property_count, method_count


def main():
    errors = []
    paths = markdown_files()
    texts = {p.resolve(): prose(p, errors) for p in paths}
    headings = {p: anchors(lines) for p, lines in texts.items()}
    links = 0
    for path, lines in texts.items():
        duplicate_paragraphs(path, lines, errors)
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
    property_count, method_count = public_api_mentions(cmake, errors)
    types = set(re.findall(r"qml/(Cat\w+)\.qml", cmake))
    for header in (ROOT / "src/uikit").rglob("*.h"):
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
    print(f"Checked {len(paths)} Markdown files, {links} local links, code fences, duplicate prose, and {len(types)} public Cat types.")
    print(f"Topic guides mention {property_count} public C++ Q_PROPERTY and "
          f"{method_count} Q_INVOKABLE declarations (name coverage only).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
