# Proposal

## Why

QCatUiKit 的交互控件通过 `Accessible.*` 暴露辅助语义，但没有任何自动检查。近期 `CatCommandBar` 溢出按钮的可见字形被内边距挤成 0 宽，而它的辅助名称、角色和按压动作始终正确，说明视觉呈现与语义信息是两类独立回归，需要分别守门。

本次 C++ 检查只验证 Qt 进程内无障碍接口，不验证操作系统桥接或外部辅助技术。真正验证“辅助技术能读到并操作控件”还需要平台侧客户端和独立的端到端检查，不属于本次通过结果。

## What Changes

- 新增离屏控件语义检查 `tests/auto/qml/data/tst_accessibility.qml`，随 `qml_contracts` 运行：文本派生控件的名称随文本、超链接角色为 Link、文本更新后名称跟随。
- 新增可选 Qt 进程内接口检查 `tests/auto/accessibility/tst_accessibility`：查询 `QAccessible` 树，断言名称、角色与动作，并执行 Press 验证溢出菜单能打开。
- 新增构建选项 `QCATUIKIT_TEST_ACCESSIBILITY`，默认 OFF；只影响接口检查，离屏语义检查始终开启。
- 接口检查在不支持的平台自动跳过并说明原因，CTest 通过专用跳过标记将其记为 Skipped，不把“未执行”记为通过。
- 文档：`BUILDING.md` 选项表、`VALIDATION.md` 平台矩阵与记录要求。

## Capabilities

### New Capabilities

无。

### Modified Capabilities

- `validation-infrastructure`: 增加可选的 Qt 进程内无障碍接口测试组，以及其注册、跳过和记录要求。

## Impact

新增 `tests/auto/qml/data/tst_accessibility.qml`、`tests/auto/accessibility/`；修改 `tests/auto/CMakeLists.txt`、`src/uikit/doc/BUILDING.md`、`src/uikit/doc/VALIDATION.md`。

不改变公开属性、信号、C++ 接口或 ABI，不修改控件实现。语义检查若发现缺失名称或角色，按缺陷单独立项，不在本次顺手修改。

不涉及：库级翻译下发。辅助名称使用 `qsTr()`，但库不随安装包提供 `.qm`，跨语言可用性另行立项。
