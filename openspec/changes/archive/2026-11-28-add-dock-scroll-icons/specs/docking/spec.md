# docking Specification (delta)

## MODIFIED Requirements

### Requirement: R06 TabOverflowScroll

主工作区和浮动窗口的标签 SHALL 单行排列。标签超出栏宽时，左右两端 SHALL 提供滚动按钮，仅可继续滚动的方向可用。标签 SHALL 保持滚轮浏览能力。

滚动按钮图标 SHALL 可由应用通过 `CatDockWorkspace.tabScrollLeftIcon` 和 `tabScrollRightIcon` 覆盖；未提供时 SHALL 使用内置绘制。覆盖图标 SHALL NOT 改变按钮宽度、可用状态、滚动步长或拖动自动滚动行为。

#### Scenario: 浏览溢出标签

- **WHEN** 标签超出可见宽度且用户点击可用的滚动按钮
- **THEN** 标签栏 SHALL 向对应方向滚动，原有标签顺序和活动面板 SHALL 保持。

#### Scenario: 拖动越过可见范围

- **WHEN** 用户拖动可移动标签并停留在可继续滚动的边缘或同侧滚动按钮上
- **THEN** 标签栏 SHALL 持续向该侧滚动，被拖动的标签 SHALL 留在可见边缘附近，并随滚动更新组内插入预览。
- **WHEN** 用户移回中部、到达滚动极限或结束拖动
- **THEN** 自动滚动 SHALL 停止。用户在栏内松手时，布局 SHALL 采用预览顺序。

#### Scenario: 临时离开原栏

- **WHEN** 拖动明显离开原标签栏
- **THEN** 系统 SHALL 停止该栏的自动滚动并进入停靠预览。
- **WHEN** 用户在同一次拖动中返回原栏
- **THEN** 系统 SHALL 恢复组内换序预览。取消拖动 SHALL 保留原顺序。

#### Scenario: Wayland 离栏后立即接管拖动

- **WHEN** 原生 Wayland 支持顶层拖动协议且用户将可移动标签拖离原标签栏换序区域，指针仍位于软件窗口内
- **THEN** 系统 SHALL 立即启动原生拖动，由合成器显示拖动图像，无需先越过整个软件窗口边界。
- **WHEN** 用户拖动已有浮窗的可移动标签离开原标签栏换序区域
- **THEN** 系统 SHALL 启动原生拖放以支持回停或合并，无需先离开浮窗边界。

#### Scenario: Wayland 无顶层拖动协议的预览

- **WHEN** 原生 Wayland 不支持顶层拖动协议，用户在主窗口中拖动面板离开原标签栏
- **THEN** 局部拖动预览 SHALL 不受工作区及祖先滚动容器裁剪；拖出工作区后 SHALL 保留创建浮窗并请求系统移动的降级行为。
- **WHEN** 工作区销毁
- **THEN** 位于窗口覆盖层的拖动预览 SHALL 一并释放。

#### Scenario: 保留其他平台拖动行为

- **WHEN** 应用使用 xcb／XWayland、macOS 或 Windows 平台插件
- **THEN** 标签拖动 SHALL 保留原有鼠标拖动与随动缩略窗路径。

#### Scenario: 自定义滚动图标

- **WHEN** 应用提供 `tabScrollLeftIcon` 或 `tabScrollRightIcon`
- **THEN** 对应滚动按钮 SHALL 显示该组件并隐藏内置绘制图标
- **WHEN** 未提供组件
- **THEN** 按钮 SHALL 继续显示内置绘制图标
- **THEN** 按钮宽度、可用状态和滚动行为 SHALL 保持不变

#### Scenario: 浮窗一致

- **WHEN** 标签位于浮动窗口并发生溢出
- **THEN** 滚动按钮 SHALL 使用与主工作区相同的图标配置
