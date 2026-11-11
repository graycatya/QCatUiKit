# Tasks

## 1. 实现

- [x] 1.1 溢出按钮由 `CatDropDownButton` 改为扁平 `CatButton`，自行打开并定位 `CatMenu`。
- [x] 1.2 宽度改为 `max(controlHeight, implicitContentWidth)`，左右内边距归零。
- [x] 1.3 为溢出按钮与菜单增加 `objectName`。

## 2. 测试

- [x] 2.1 新增 `tests/auto/qml/data/tst_commandbar.qml`：可用宽度为正、无次要操作隐藏、点击后菜单打开且项数正确。
- [x] 2.2 在 Linux／Qt 6.12.0／Release／offscreen 环境执行该用例：5 passed、0 failed、0 skipped，结果见 validation.md；此前 Windows 运行器崩溃记录保留。

## 3. 验证

- [x] 3.1 `QCatUiKit_qmllint` 通过，测试文件 `qmllint` 通过。
- [x] 3.2 Release 增量构建后截取 Gallery 命令栏页面，亮色、暗色、触摸密度与 420 窄屏下省略号均居中可见。
- [x] 3.3 `python tools/check_docs.py` 通过。
- [x] 3.4 `openspec validate fix-commandbar-overflow-affordance --strict --no-interactive` 通过。

## Workflow follow-up

- 归档时把 R09 同步进 `ui-components` 主规格。
- 若 2.2 长期无法在 CI 覆盖，需要在 `VALIDATION.md` 记录该用例的可执行环境。
