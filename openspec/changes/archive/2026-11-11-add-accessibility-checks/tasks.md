# Tasks

## 1. 控件语义检查（离屏，默认开启）

- [x] 1.1 新增 `tests/auto/qml/data/tst_accessibility.qml`：按钮、复选框、单选框、开关、菜单项的名称随文本；超链接角色为 Link。
- [x] 1.2 增加“文本变化后名称跟随”的用例。
- [x] 1.3 用不依赖 Qt Quick Test 运行器的外部检查执行 `collectSemanticFailures()`：Windows 11、Qt 6.12.0、offscreen、Release，结果 `RESULT: PASS (0 failure(s))`。

## 2. Qt 进程内接口检查（可选）

- [x] 2.1 新增 `tests/auto/accessibility/tst_accessibility.cpp`：创建包含 `CatCommandBar` 的场景，查询 `QAccessible` 树并按名称查找溢出按钮。
- [x] 2.2 在 `tests/auto/CMakeLists.txt` 与 `tests/auto/accessibility/CMakeLists.txt` 增加 `QCATUIKIT_TEST_ACCESSIBILITY`（默认 OFF）并注册 `accessibility_interfaces`。
- [x] 2.3 平台白名单门控：非 `windows`／`cocoa`／`xcb`／`wayland` 时 `QSKIP` 并输出平台名。
- [x] 2.4 断言溢出按钮角色为 Button、存在 press 动作；执行 press 后等待菜单 `opened`。

## 3. 文档

- [x] 3.1 `src/uikit/doc/BUILDING.md` 选项表补 `QCATUIKIT_TEST_ACCESSIBILITY`。
- [x] 3.2 `src/uikit/doc/VALIDATION.md` 增加无障碍检查小节：分平台命令、依赖、跳过语义与记录要求。

## 4. 验证

- [x] 4.1 `QCatUiKit_qmllint` 通过；`tst_accessibility.qml` 单独 `qmllint` 通过。
- [x] 4.2 `python tools/check_docs.py` 通过（38 个 Markdown 文件，528 个本地链接）。
- [x] 4.3 `openspec validate add-accessibility-checks --strict --no-interactive` 通过。
- [x] 4.4 无桥接平台行为：Windows 11、offscreen、Release 运行 `tst_accessibility`，报告 `1 skipped`，未崩溃。
- [x] 4.5 Windows：原生 windows 窗口运行 `accessibility_interfaces`，CTest 1/1 通过；仅验证 Qt 进程内接口，详见 validation.md。
- [x] 4.6 Linux：XCB 原生会话运行 `accessibility_interfaces`，CTest 1/1 Passed、无跳过；offscreen 为 Skipped，详见 validation.md。
- [x] 4.7 macOS：2026-11-11 补验 Cocoa 原生会话，`accessibility_interfaces` 为 1/1 Passed、无跳过；offscreen 为 Skipped，完整 `qml_contracts` 通过。仅验证 Qt 进程内接口，详见 validation.md。

## 5. 审查修正

- [x] 5.1 为不支持平台增加专用跳过标记与 CTest 跳过识别，并通过实际 CTest 验证 Skipped：offscreen、minimal 均为 skipped=1。
- [x] 5.2 将测试命名与文档、proposal、design、delta specs 对齐为 Qt 进程内接口检查，明确平台桥接未覆盖。
- [x] 5.3 运行文档检查、OpenSpec 严格校验与差异检查，记录验证结果；全部通过。
- [x] 5.4 原生窗口复验发现普通 Item 根节点不能作为接口树查询入口；改为查询 QQuickView 窗口接口后，名称、角色、press 动作与菜单打开均通过。

## Workflow follow-up

- 归档时把 R05 同步进 `validation-infrastructure` 主规格。
- 语义检查发现的缺失名称按缺陷单独立项。
- 库级翻译下发另行立项。
