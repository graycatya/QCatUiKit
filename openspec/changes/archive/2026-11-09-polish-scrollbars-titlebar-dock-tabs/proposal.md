# Proposal

## Why

Gallery 的侧栏滚动条在指针离开后仍常驻，窗口标题栏使用字符字形且有异常的红色关闭边框，停靠标签的关闭按钮看起来像独立方块并与标题脱节。这些差异削弱了主题一致性和可发现的悬停反馈。需要把叠加滚动条、标题栏按钮和标签关闭行为固化为可验证的组件契约，避免后续控件改动再次偏离。

## What Changes

- `CatScrollView` 与 `CatScrollBar` 增加叠加式滚动条显示契约：指针进入视图、滚动中、拖动或悬停滑块时显示，其余时间淡出；`CatScrollBar.revealed` 默认 `true`，独立使用的滚动条保持常驻；`AlwaysOn` 策略始终可见。
- `CatTitleBar` 用绘制图标替换 `−`、`□`、`×` 等字符，提供最小化、最大化／还原、关闭的正常、悬停、按下、禁用和键盘焦点外观；关闭按钮使用主题危险色悬停并显示浅色叉号，去掉 flat 危险按钮的常驻红色边框。
- 停靠工作区与浮动窗口的标签改为单一单元格：标题与关闭按钮共享单元格背景和 2px 活动下划线，关闭按钮紧凑贴靠标题，并具有圆角危险色悬停反馈。
- 更新 `src/uikit/doc/COMPONENTS.md` 中 `CatScrollBar`、`CatScrollView` 的行为说明。

## Capabilities

### New Capabilities

无。

### Modified Capabilities

- `ui-components`: 增加叠加滚动条与标题栏窗口按钮的可观察外观/状态要求。
- `docking`: 增加停靠标签单元格、关闭按钮和悬停/活动外观要求。

## Impact

影响 `src/uikit/qml/CatScrollBar.qml`、`CatScrollView.qml`、`CatTitleBar.qml`、`DockTabButton.qml`、`CatDockWorkspace.qml`、`DockFloatingWindow.qml` 及 `src/uikit/doc/COMPONENTS.md`。不改变公共 C++ API、滚动条策略枚举、停靠控制器方法或布局 JSON；新增 QML 只读属性 `CatScrollBar.revealed`、`CatTitleBar.maximized`，不涉及 ABI。Gallery 的滚动区域、窗口示例和停靠示例用于视觉核查。

本次变更的代码与验证已在工作区完成，本 change 用于补建 OpenSpec 记录并保留验证证据。
