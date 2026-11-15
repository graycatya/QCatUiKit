# Tasks

## 1. 实施和验证

- [x] 1.1 为共享标签栏实现溢出滚动按钮与拖拽自动滚动，补充停靠说明及回归测试，执行检查并记录结果。

## Validation

- Windows Qt 6.12.0 离屏运行工作区和浮动窗口的 QML 专项。用现有 Release 库加载本次修改的 QML，40 项通过。新增用例覆盖滚动按钮、静止指针靠边滚动、拖离再返回及释放提交顺序。
- `python tools/check_docs.py`、`git diff --check` 和 OpenSpec 严格校验通过。
- 独立检查中 DockDragArea 通过 qmllint，DockTabStrip 的 qmllint 进程退出且未给出诊断。完整重编译和原生 Wayland 拖放未验证。
- 补充两侧滚动按钮的悬停、按下和禁用反馈后，Windows 离屏专项仍为 40 项通过。回归用例同时验证悬停与按下颜色、边界禁用和点击滚动。
