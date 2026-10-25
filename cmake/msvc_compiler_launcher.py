#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Keep MSVC diagnostics readable and Ninja dependency records encoding-independent."""
import ctypes
import locale
import os
import re
import subprocess
import sys

# MSVC's include diagnostic has two localized labels before an absolute or relative path.
# Do not mistake a source location or an error/warning for an include record.
INCLUDE = re.compile(rb"^[^:\r\n()/\\]+:[ \t]+([^:\r\n]+):[ \t]+(.+)$")

def main():
    command = sys.argv[1:]
    if command and command[0] == "--":
        command.pop(0)
    if not command:
        return 2
    encoding = locale.getpreferredencoding(False)
    if os.name == "nt":
        codepage = ctypes.windll.kernel32.GetConsoleOutputCP() or ctypes.windll.kernel32.GetACP()
        encoding = "cp%d" % codepage
    env = dict(os.environ, VSLANG="1033")
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, env=env)
    for line in iter(process.stdout.readline, b""):
        content = line.rstrip(b"\r\n")
        match = INCLUDE.match(content)
        if match and not re.search(rb"\b(?:error|warning)\b", match.group(1), re.I):
            # Include records are ASCII-prefixed UTF-8 even on a Chinese-only
            # MSVC install launched by Qt Creator without a console.
            path = match.group(2).decode(encoding, errors="replace")
            output = "Note: including file: " + path + "\n"
        else:
            output = line.decode(encoding, errors="replace")
        sys.stdout.buffer.write(output.encode("utf-8"))
        sys.stdout.buffer.flush()
    return process.wait()

if __name__ == "__main__":
    sys.exit(main())
