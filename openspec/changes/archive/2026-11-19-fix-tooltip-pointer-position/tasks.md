# Tasks

## 1. 实现

- [x] 1.1 在 `CatToolTip` 实现指针定位、显示后固定、底部翻转和窗口右侧约束，并通过 `tst_tooltip.qml` 的定位、右／下边缘用例验证。
- [x] 1.2 实现移动重置 delay、点击（含等待期间）、Esc、移出和窗口失活关闭，以及关闭后同次悬停不重开；通过 `tst_tooltip.qml` 对应用例验证。
- [x] 1.3 新增 `hoverInteractionEnabled` 并让 `SliderHandleImpl` 数值提示退出悬停交互，通过滑块 QML 回归验证显隐与定位不受影响。

## 2. 文档与回归

- [x] 2.1 同步 `COMPONENTS.md` 的组件说明和 `VALIDATION.md` 的回归入口，运行 `python tools/check_docs.py` 通过。
- [x] 2.2 完成构建、库／Gallery qmllint、QML 契约与 Gallery 启动回归，并在 `validation.md` 记录环境、命令和未覆盖范围。

## Validation

本轮验证在 Windows 11、Qt 6.12.0 MSVC Debug 的 offscreen／software 环境完成，证据见 [validation](validation.md)：

- 提示定向回归 10 passed、0 failed；滑块回归 16 passed、0 failed。
- 完整 Debug 构建、库／Gallery qmllint、文档检查和差异空白检查通过。
- `ctest -R '^(qml_contracts|gallery_smoke)$'` 2/2 通过。
- 首次整组测试暴露的拖动干扰已修复；首轮桌宠回归中 `lottieRendering` 与 `largePackageKeepsEventLoopResponsive` 超出 30000ms 限制，未修改桌宠代码，不宣称整套测试通过。
- 未验证真实桌面窗口失活、多屏及其他平台。

## Workflow follow-up

- 归档本变更，并核对 `ui-components` 主规约已合并 R10。
