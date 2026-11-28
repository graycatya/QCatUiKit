# Tasks

## 1. 构建规避

- [x] 1.1 在 `src/uikit/CMakeLists.txt` 中，Windows + Qt 6.12.0 时把 `CatTableView.qml` 从 `QT_QML_LINT_FILES` 移除。
- [x] 1.2 在 `VALIDATION.md` 记录原因、条件和升级 Qt 后的复核要求。

## 2. 验证

- [x] 2.1 重新配置后确认生成的 lint 响应文件不再包含 `CatTableView.qml`，其余 QML/JS 文件仍保留。
- [x] 2.2 `cmake --build build-led-release --target QCatUiKit_qmllint` 通过。
- [x] 2.3 `models`、`qml_contracts`、`gallery_contracts` 通过，确认运行时行为未变。
- [x] 2.4 `python tools/check_docs.py`、`git diff --check`、`openspec validate workaround-qmllint-crash --strict --no-interactive` 通过。
- [x] 2.5 在 `validation.md` 记录二分过程、排除范围和未覆盖内容。
