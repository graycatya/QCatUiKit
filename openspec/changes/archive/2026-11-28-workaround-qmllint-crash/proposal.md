# Proposal

## Why

`QCatUiKit_qmllint` 在 Windows + Qt 6.12.0 上始终失败，但没有任何诊断输出。逐文件二分后确认 `src/uikit/qml/CatTableView.qml` 会让 `qmllint` 静默退出（退出码 -1）。该文件包含五个共享同一 `delegate` 和 `syncView` 关系的 `TableView`；删除任意一部分视图或改用命令式 delegate 赋值，要么不能绕过，要么会把已验证的绑定行为改为运行时赋值，风险大于收益。

这是工具缺陷，不是 QML 语法错误：同一文件在项目记录的 Linux 环境中可以正常 lint。当前 Windows 上无法执行该文件的静态检查，导致整个库 lint 目标长期红灯。

## What Changes

- 在 `src/uikit/CMakeLists.txt` 中，当平台为 Windows 且 Qt 为 6.12.0 时，把 `CatTableView.qml` 从 `QT_QML_LINT_FILES` 中移除；其他平台和后续 Qt 版本继续检查该文件。
- 在 `VALIDATION.md` 记录该条件和升级 Qt 后的复核要求。

不修改 QML 或 C++ 行为，不放宽其他文件的检查，不把该规避描述为完整 lint 通过。

## Capabilities

### New Capabilities

无。

### Modified Capabilities

无行为规约变化。

## Impact

影响 `src/uikit/CMakeLists.txt` 和 `src/uikit/doc/VALIDATION.md`。Windows + Qt 6.12.0 下该文件不再被 lint；升级 Qt 或换平台后应删除条件并复核。运行时行为、构建产物和安装接口不变。
