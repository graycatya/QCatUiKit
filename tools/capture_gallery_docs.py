#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Render documentation screenshots from the built Gallery and its real models."""

import argparse
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", required=True, type=Path)
    parser.add_argument("--config", default="Debug")
    parser.add_argument("--output-dir", type=Path)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    build = args.build_dir.resolve()
    output = (args.output_dir or root / "src/uikit/doc/images").resolve()
    suffix = ".exe" if os.name == "nt" else ""
    candidates = [
        build / "tests/auto/gallery" / ("tst_gallery" + suffix),
        build / "tests/auto/gallery" / args.config / ("tst_gallery" + suffix),
        build / "bin" / args.config / ("tst_gallery" + suffix),
        build / "bin" / ("tst_gallery" + suffix),
    ]
    runner = next((p for p in candidates if p.is_file()), None)
    if runner is None:
        parser.error("Build tst_gallery with BUILD_TESTING=ON and QCATUIKIT_BUILD_EXAMPLES=ON first.")
    template = (root / "tools/gallery_docs/tst_capture.qml.in").read_text(encoding="utf-8")
    env = dict(os.environ, QT_QPA_PLATFORM="offscreen", QT_QUICK_BACKEND="software")
    with tempfile.TemporaryDirectory(prefix="qcat-gallery-docs-") as temporary:
        work = Path(temporary)
        fixture = work / "tst_capture.qml"
        fixture.write_text(template.replace("@CAPTURE_DIRECTORY@", json.dumps(work.as_posix())),
                           encoding="utf-8")
        command = [str(runner), "-import", str(build / "qml"), "-input", str(fixture)]
        subprocess.run(command, env=env, check=True, timeout=180)
        screenshots = sorted(work.glob("gallery-*.png"))
        if len(screenshots) != 9:
            raise RuntimeError(f"Expected 9 screenshots, got {len(screenshots)}")
        # Publish only a complete successful capture set. No image manipulation.
        sizes = []
        for screenshot in screenshots:
            header = screenshot.read_bytes()[:24]
            if header[:8] != b"\x89PNG\r\n\x1a\n":
                raise RuntimeError(f"Invalid PNG: {screenshot.name}")
            sizes.append((screenshot, struct.unpack(">II", header[16:24])))
        output.mkdir(parents=True, exist_ok=True)
        for screenshot, (width, height) in sizes:
            shutil.copyfile(screenshot, output / screenshot.name)
            print(f"Saved {screenshot.name}: {width} x {height}")


if __name__ == "__main__":
    main()
