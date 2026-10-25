# QCatUiKit ThorVG QRhi adapter

Both CPU-only and QRhi builds download the official ThorVG `v1.1.3` archive
from https://github.com/thorvg/thorvg/tree/v1.1.3 and verify its pinned SHA-256.
QRhi builds then apply `overlay/`, which supplies our experimental backend.
The upstream release itself does not need to contain QRhi.

The three overlaid Meson entry points are based on official v1.1.3, with only
the QRhi engine option, configuration and source integration added. The version
remains 1.1.3; the overlay adds no newer core renderer sources.

The overlay preserves the experimental QRhi implementation developed in the
adjacent ThorVG working tree. It is checked in here so a fresh clone does not
need that directory. `PROVENANCE.json` records the base revision, archive hash
and copied-file hashes. The packaging adaptation uses Meson's running Python
interpreter for shader embedding instead of searching for `python` on PATH.

ThorVG and these experimental implementation files use the MIT license in
`LICENSE`. Upstream notices remain in the downloaded source. The surrounding
QCatUiKit CMake and launcher code use the repository's Apache-2.0 license.

CMake fingerprints the overlay and prepares a separate managed source directory
for each implementation revision. Debug and Release use separate Meson build
and install directories. See [build instructions](../../src/uikit/doc/BUILDING.md).

The module directory and owned C/C++ files follow QCatUiKit's lowercase `qcat`
naming convention. `qcatthorvgqrhicanvas.h` declares
`QCatUiKit::ThorvgQrhiCanvas`; implementation types live in
`QCatUiKit::ThorvgQrhiPrivate`, with `qcatthorvgqrhirenderer_p.h` as the private
header. Generated shader headers also use the `qcatthorvgqrhi` prefix and `_p.h`.

The adapter entry point is `qcatthorvgqrhicanvas.h` and
`QCatUiKit::ThorvgQrhiCanvas`. QCatUiKit links this implementation privately;
it is not an installed public QCatUiKit component API.

The overlay's upstream Meson entry points retain their paths. Meson source lists
and includes refer to the adapter's new filenames. Original file hashes are kept
in `PROVENANCE.json` as `original_files`; `files` describes the maintained overlay.
The renamed and formatted implementation retains its MIT license.
