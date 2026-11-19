# Design

## Context

`CatToolTip` 是 `T.ToolTip` 的样式化子类，组件本身不拥有被提示元素的悬停状态：调用方通过 `visible` 绑定（例如 `visible: button.hovered`）请求显示，Qt ToolTip 负责 delay、timeout 与弹窗窗口。改动前组件只提供主题、内边距和文本外观，位置由 Qt 默认规则决定。本次需要在不改变调用方绑定方式的前提下增加指针定位和关闭语义，并保留应用完全控制提示显隐的场景。动机见 [proposal](proposal.md)。

## Goals / Non-Goals

**Goals:**

- 复用 Qt ToolTip 的 delay／timeout 与弹窗窗口约束，不引入第二套计时器。
- 同时处理鼠标、点击、Esc、移出和窗口失活，不额外占用父项的输入。
- 调用方继续用 `visible` 绑定表达“是否应该显示”，组件只做抑制与定位。

**Non-Goals:**

- 不改用自定义 Popup，不改变提示内容模型、主题 tokens 和默认 delay／timeout。
- 不处理触控长按、绘图板悬停、多屏跨屏避让和提示的无障碍播报策略。
- 不为 `hoverInteractionEnabled=false` 提供自动定位。

## Decisions

### 用被动 Handler 观察父项，而不是 MouseArea

`HoverHandler` 和 `PointHandler` 的 `parent` 设为 `control.parent`，只观察指针，不抢事件，因此按钮按下、拖动和父项自身的鼠标处理保持原样。`MouseArea` 会参与事件竞争，并在拖动场景吞掉或延迟父项输入；`hoverEnabled` 的 Item 也无法同时观察点击。两者坐标空间与弹窗父项相同，可直接把 `point.position` 用作 x/y。

- 备选：在 `CatToolTip` 上处理 `hovered`。ToolTip 自身通常是覆盖层，指针不在其内，无法覆盖“提示显示前指针移动”的计时重置。

### 用 Binding 暂时抑制 visible，而不是修改 visible 或自建 Timer

调用方的 `visible` 绑定必须保留。`Binding` 在 `when` 成立时把 `visible` 压到 `false`，`restoreMode: RestoreBindingOrValue` 在结束后恢复原绑定；恢复瞬间 Qt ToolTip 重新开始内部 delay，因此“移动重置计时”不需要额外计时器，也不会出现双计时器竞态。

- 备选：直接 `control.visible = false` 会破坏调用方绑定。
- 备选：自建 Timer 复制 delay 逻辑，需要同时同步 timeout、隐式可见性和 ToolTip 内部状态，易出现两套状态不一致。

### 显示时记录位置，之后固定

位置在 `onAboutToShow` 时从 Handler 取一次并标记 `positionedAtPointer`。显示后不再跟随指针，避免提示追逐光标影响阅读；指针在父项内移动只更新计时，不更新位置。下一次显示会重新采样。

### 关闭状态只在指针移出时清除

`dismissed` 在点击、Esc、timeout（`onClosed` 且指针仍悬停）时置位，在 `hovered` 变为 false 时清除。这样“同一次悬停不重开、移出后再进入恢复”不需要额外状态机，也覆盖了等待期间点击取消显示的情况。

### Esc 用窗口级 Shortcut，失活用 Window.active 监听

ToolTip 是覆盖层，通常没有键盘焦点，Esc 不能依赖弹窗 `closePolicy`；`Qt.WindowShortcut` 在窗口内生效，并与 `control.visible` 绑定，避免影响其他控件。窗口失活通过父项所属 `Window` 的 `activeChanged` 观察，切换应用时不会留下悬浮提示。

### `hoverInteractionEnabled=false` 作为应用控制模式的出口

滑块拖动数值等提示由应用在拖动期间常显并自行定位。该模式关闭两个 Handler、Esc 快捷关闭和窗口失活监听，`closePolicy` 设为 `T.Popup.NoAutoClose`，组件只保留主题和内容；可见性与 x/y 完全由调用方决定。默认 `true` 保持既有调用方无需改动。

## Risks / Trade-offs

- 指针行为依赖 `control.parent` 作为悬停锚点；没有父项时 Handler 不生效，提示回退到父项下方定位，属于可接受的降级 → 文档说明提示应作为被提示项的父级或同父项子项创建。
- 同一父项下挂多个提示会同时观察同一指针 → 调用方应把提示挂到各自的锚点；本次不为多提示去重。
- `Binding` 抑制期间若调用方解除或替换 `visible` 绑定，恢复结果是最近的值而不是新绑定 → 应用控制模式应改用 `hoverInteractionEnabled=false`，文档已说明。
- 底部翻转基于父项映射到 `Window.contentItem` 的位置；父项不在窗口内时不翻转，保持指针下方定位。
- 验证仅覆盖 offscreen 自动化与 480×260 离屏截图；真实桌面窗口失活、多屏和其他平台未验证（见 [validation](validation.md)）。

## Migration Plan

新增属性默认 `true`，已有调用方无需修改，但原先依赖 Qt 默认定位的提示会改变位置，属于本次预期行为。库内拖动数值提示显式设为 `false`。仅 QML 变更，无 ABI、构建或安装影响，回退只需还原 QML 文件。
