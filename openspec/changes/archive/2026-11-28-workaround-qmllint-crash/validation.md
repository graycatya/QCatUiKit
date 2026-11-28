# Validation

执行环境：Windows 11 x64，Qt 6.12.0 msvc2022_64，Release 单配置，Ninja，MSVC 19.39，`build-led-release`。

## 现象与定位

`cmake --build build-led-release --target QCatUiKit_qmllint` 失败，`qmllint` 无 stdout/stderr，退出码 -1。直接运行响应文件同样静默退出。

逐文件运行后确认只有 `src/uikit/qml/CatTableView.qml` 会崩溃；同一响应参数下其他 133 个 QML/JS 文件可以完成分析。

对该文件做副本二分：

- 完整副本：崩溃。
- 删除冻结内容区 `frozenContentArea` 整块：正常完成并输出警告。
- 只保留 `plainView` 或只保留 `frozenTopRightView`：正常。
- 同时保留 `frozenTopLeftView` 与 `frozenTopRightView`：崩溃。
- 将第二个视图的 `delegate` 改为 `null`：正常。

尝试两种不改行为的重构：各视图持有局部 `Component` 属性、以及用 `onDelegateChanged` 命令式赋值。两者在完整文件上仍崩溃，已回退，未保留到仓库。

## 处理

`src/uikit/CMakeLists.txt` 在 Windows + Qt 6.12.0 时过滤 `QT_QML_LINT_FILES`：

- 重新生成后响应文件从 134 个 QML/JS 文件变为 133 个，不再包含 `CatTableView.qml`。
- 其他平台和其他 Qt 版本不受影响。

## 检查

```sh
cmake --build build-led-release --target QCatUiKit_qmllint
ctest --test-dir build-led-release -R "^(models|qml_contracts|gallery_contracts)$" --output-on-failure
python tools/check_docs.py
git diff --check
openspec validate workaround-qmllint-crash --strict --no-interactive
```

结果：

- `QCatUiKit_qmllint` 通过（退出码 0）。
- `models` 0.51 s、`qml_contracts` 43.18 s、`gallery_contracts` 143.57 s，全部通过。
- 文档、空白与变更结构检查通过。

## 未覆盖

- `CatTableView.qml` 在 Windows + Qt 6.12.0 下没有得到静态检查结果；该文件的行为仍由 `qml_contracts`、`gallery_contracts` 和表格相关用例覆盖，但不等于 lint 通过。
- 未在 Linux 或更高 Qt 版本复测该文件是否仍崩溃；升级后应删除条件并重新运行。
- 没有继续定位 Qt 工具内部缺陷，未提交上游报告。
