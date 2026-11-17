# Tasks

## 1. 实施和验证

- [x] 1.1 为共享标签栏实现溢出滚动按钮与拖拽自动滚动，补充停靠说明及回归测试，执行检查并记录结果。
- [x] 1.2 修复原生 Wayland 离栏后启动拖动过晚和局部预览裁剪，保留无协议降级与其他平台路径，补充分支、坐标及生命周期测试。
- [x] 1.3 完成构建、lint、完整回归与原生 Wayland 定向验证，并记录用户实际拖动验收结果。

## Validation

- Windows Qt 6.12.0 离屏运行工作区和浮动窗口的 QML 专项。用现有 Release 库加载本次修改的 QML，40 项通过。新增用例覆盖滚动按钮、静止指针靠边滚动、拖离再返回及释放提交顺序。
- `python tools/check_docs.py`、`git diff --check` 和 OpenSpec 严格校验通过。
- 独立检查中 DockDragArea 通过 qmllint，DockTabStrip 的 qmllint 进程退出且未给出诊断。完整重编译和原生 Wayland 拖放未验证。
- 补充两侧滚动按钮的悬停、按下和禁用反馈后，Windows 离屏专项仍为 40 项通过。回归用例同时验证悬停与按下颜色、边界禁用和点击滚动。
- macOS Qt 6.10.2 修复加载循环：`DockTabStrip` 创建 `DockDragArea`，后者的 `tabStrip` 改用 `var` 保存引用，避免反向引用具体 QML 类型。Debug 完整构建及库／Gallery qmllint 通过；离屏运行 `tst_dock.qml` 15 项、`tst_dockfloating.qml` 25 项全部通过；CTest 的 `gallery_smoke`、`dock_controller`、`dock_native` 全部通过。未进行原生窗口拖放的人工验证。
- 本轮全套 CTest 前 13 项通过（含 `qml_contracts`）；`gallery_contracts` 长时间未结束，手动中断，未宣称全套通过。`python3 tools/check_docs.py` 及 `git diff --check` 通过。

### 2026-11-17 Wayland 修复后的补充验证

- 上述未完整运行的记录保留为历史结果；此次 Qt 6.12.0 Linux Debug／Release 完整构建、库与 Gallery qmllint 均通过，Release CTest 21/21 通过。
- 原生 Wayland 定向测试 11 passed、0 failed、0 skipped；offscreen 定向测试 7 passed、0 failed、4 skipped。细节见 [修复验证记录](wayland-drag-preview-validation.md)，更早的 Wayland 验证见 [验证记录](validation.md)。
- 用户在修复后确认原生 Wayland 拖动效果正常，并再次确认 Dock 测试通过；记录为用户实测验收，不扩展推断为所有合成器、GPU、多屏或不同 DPI 已验收。
- 用户此前验证 xcb、macOS、Windows 正常；修复后未重新进行这三端实机验证。本次交互调整限定在原生 Wayland 分支。
