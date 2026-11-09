# Design

## Context

滚动条、标题栏和停靠标签都是 QML 模板控件：`CatScrollBar` 直接继承 `T.ScrollBar`，`CatScrollView` 继承 `T.ScrollView`，`CatTitleBar` 继承 `T.Control`，停靠标签复用 `DockTabButton`。Gallery 同时消费这些组件，因此改动必须保持现有公共 C++ API、滚动策略和停靠布局格式不变。

`CatScrollView` 依赖 `Component.onCompleted` 中创建的 `MouseScrollImpl` 来处理按住拖动；Qt 的 `ScrollView` 会延迟创建 contentItem 并在此时连接滚动条。任何在 contentItem 建立前读取它的声明都会绕过该连接（实测滚动条 `size` 保持 0）。

## Goals / Non-Goals

**Goals:**

- 用组件自身状态表达滚动条显隐，保证独立滚动条、`AlwaysOn` 和视图内滚动条行为一致。
- 让标题栏按钮和停靠标签的悬停、按下、焦点、禁用状态在亮暗主题下可区分。
- 保持纯 QML 改动，不修改 C++、布局 JSON 或滚动策略枚举。

**Non-Goals:**

- 不新增标题栏窗口能力或平台适配。
- 不改变停靠控制器的注册、停靠、恢复和权限语义。
- 不为各平台提供像素级一致的图标栅格化保证。

## Decisions

### 1. 滚动条显隐放在 `CatScrollBar.revealed`，视图只提供指针状态

`revealed` 默认 `true`，独立使用和既有调用保持常驻；`CatScrollView` 把视口指针状态绑定到两个 attached 滚动条。显示状态由 `pressed || active || hovered || revealed || AlwaysOn` 决定，用根节点 `opacity` 淡出而不修改 `visible`，从而保留隐藏时的悬停和拖动命区。

备选方案：`visible: false` 会移除命区，隐藏后无法再通过悬停唤出；仅用 `active` 会在静止时完全不可发现。淡出动画使用 `tokens.motionDuration`，`reducedMotion` 下该 token 为 0，无需额外分支。

### 2. 视口悬停跟踪在 contentItem 创建后动态安装

在 `CatScrollView` 的 `Component.onCompleted` 中先创建既有 `MouseScrollImpl`，再把 `HoverHandler` 安装到 `contentItem`。这样既避免提前读取 contentItem 破坏 ScrollView 的延迟连接，也让处理器覆盖整个视口并接收子项冒泡的悬停。

备选方案：在 ScrollView 上声明 `HoverHandler` 会使滚动条 `size` 变为 0（已用 QML 测试复现）；覆盖 `MouseArea` 会拦截内容交互；遍历子项安装处理器不可靠。

### 3. 标题栏按钮用绘制图元与 `Shape`，不使用字符

最小化使用细矩形，最大化使用圆角方框，还原使用双框，关闭使用 `Shape` + `CurveRenderer` 的圆头叉号。字符字形受字体和平台影响，无法稳定表达悬停/按下状态；图标字体和位图资源分别引入字形覆盖和 DPI 变体成本。

关闭按钮的静止背景使用标题栏表面色，悬停使用 `danger`，按下使用加深的 `danger`；叉号颜色由背景决定。`flat` 危险按钮不再显示常驻红边，避免误读为持续告警。

### 4. 停靠标签背景上移到单元格，关闭入口成为单元格内的紧凑工具按钮

主工作区和浮动窗口的标签 delegate 改为 `Item`：背景、活动下划线、悬停和焦点描边由单元格统一绘制，标题按钮 `background: null`，关闭按钮 26×24 贴靠右端。这样单元格宽度由标题和关闭入口共同决定，下划线自然覆盖两者，关闭悬停不会出现脱离标签的方块。

备选方案：把关闭入口放进标题按钮的 contentItem 会让关闭语义耦合到标签激活按钮，并复杂化键盘焦点；保留独立 Row 则继续出现背景断裂。

`DockTabButton` 的工具按钮背景改为内缩圆角矩形，关闭叉号改用 `Shape`，并降低标签最小宽度，使四字以内标题与关闭入口保持紧凑。

## Risks / Trade-offs

- [滚动条隐藏后仍可命中] 用户可能在不可见区域开始拖动 → 指针进入视图即淡入，滑块自身 `hovered`/`pressed` 也会显示；命区是预期行为。
- [动态 HoverHandler 与新 contentItem] 若宿主替换 `contentItem`，处理器仍指向旧对象 → ScrollView 的 contentItem 默认稳定；自定义替换属于未覆盖场景。
- [软件光栅化的 Shape 边缘] 叉号在无抗锯齿后端可能出现阶梯 → 使用 `CurveRenderer` 和圆头，已检查 software 截图；不承诺所有后端像素一致。
- [自定义标题栏背景] 静止按钮使用 `surface`，宿主完全替换标题栏背景时可能不匹配 → 由宿主同时覆盖按钮背景；本次不扩展主题 API。
- [触摸密度无悬停] `hoverEnabled` 为 false 时关闭入口不显示悬停色，但按下反馈和视口滚动条显示仍有效 → 保持现有密度语义，不在本次引入触摸专用外观。

## Migration Plan

纯 QML 行为调整，无数据迁移。依赖滚动条静止常驻的调用方可设置 `revealed: true` 或 `AlwaysOn` 恢复旧行为；停靠布局快照和控制器调用不变。
