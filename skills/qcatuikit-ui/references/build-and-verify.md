# Build and verify a generated interface

## Installed-package starter

Copy assets/starter from this skill to the requested application directory. The starter
uses installed shared Qt/QCatUiKit libraries and loads QML plugins at runtime;
NO_IMPORT_SCAN avoids trying to link an installed plugin as a source-tree target.
Run from the app directory, replacing prefixes with the actual installations:

```sh
cmake -S . -B build -GNinja \
  -DCMAKE_PREFIX_PATH="/absolute/qcat-stage;/path/to/Qt/6.x/gcc_64" \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
cmake --build build --target qcat-ui-starter_qmllint
LD_LIBRARY_PATH=/absolute/qcat-stage/lib:/path/to/Qt/6.x/gcc_64/lib \
  ./build/qcat-ui-starter --qml-import-path /absolute/qcat-stage/lib/qml
```

Use the actual lib directory (possibly lib64). The runtime Qt must match the build;
finding a CMake package alone does not set every indirect shared-library search path.
On Windows, use the matching Qt/compiler kit, add Qt/bin and qcat-stage/bin to PATH,
and use --config Debug for a multi-configuration generator. Do not mix Debug and Release
runtime libraries on Windows.

The starter's optional --smoke-test exits after loading. --screenshot takes an actual
window capture and exits. For a headless local check on Linux:

```sh
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software \
  LD_LIBRARY_PATH=/absolute/qcat-stage/lib:/path/to/Qt/6.x/gcc_64/lib \
  ./build/qcat-ui-starter --qml-import-path /absolute/qcat-stage/lib/qml \
  --screenshot /tmp/qcat-ui.png
```

The screenshot is the current viewport. To check narrow widths, theme changes or input
states, exercise those states in the app or use the app's Qt Quick tests. Software rendering
does not validate a hardware backend or GPU effects.

## Existing applications and library work

Use the existing build directory, Qt kit and integration method when available. A source
consumer can add_subdirectory(QCatUiKit) and link QCatUiKit::QCatUiKit, but must also
expose the built QML module directory to its engine. Do not silently change global build
options or add the Gallery module as an application dependency.

For library changes, repository targets are QCatUiKit_qmllint and qcat-gallery_qmllint;
the test groups are documented in src/uikit/doc/VALIDATION.md. Run checks appropriate
to the behavior changed. For a page composed solely in an app, verify that app rather
than requiring the entire library suite or an ABI audit.

Useful observable checks include: valid/invalid submission; model count and changed
values; selection after deletion; focus and text after density changes; history position
while appending; stop behavior; readable wrapping and reachable actions at narrow widths.
