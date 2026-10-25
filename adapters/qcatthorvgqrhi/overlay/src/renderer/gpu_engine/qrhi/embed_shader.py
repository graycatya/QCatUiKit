"""Embed qsb output so runtime shader lookup does not depend on a file path."""
import pathlib
import sys

data = pathlib.Path(sys.argv[1]).read_bytes()
pathlib.Path(sys.argv[2]).write_text(
    '#pragma once\nstatic const unsigned char ' + sys.argv[3] + '[] = {\n' +
    ','.join(str(value) for value in data) + '\n};\n', encoding='ascii')
