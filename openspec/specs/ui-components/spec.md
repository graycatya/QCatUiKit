# ui-components Specification

## Purpose

为 Qt Quick 应用提供主题化控件。此基线记录已核查的基础按钮、文本输入、复选框、消息提交、叠加滚动条和窗口标题栏按钮行为；专用控件的业务契约归属对应模块。

## Dependencies

- Qt Quick、Qt Quick Templates、Qt Quick Controls、Qt Quick Layouts 和 Shapes。
- Theme/ThemePalette；相关模型及专用功能模块。

## Requirements

### Requirement: R01 ThemeBinding

已核查的基础控件 SHALL 暴露 theme 并从主题获取 tokens。

#### Scenario: 主题回退

- WHEN CatButton、CatTextField 或 CatCheckBox 的 theme 为空
- THEN 控件 SHALL 使用默认 Theme tokens。

#### Scenario: 密度更新

- WHEN 未覆盖尺寸绑定且主题密度变化
- THEN 控件隐式尺寸 SHALL 随 controlHeight 等 tokens 更新。

### Requirement: R02 ButtonLoading

CatButton SHALL 在 loading 时禁用交互并显示忙碌状态。

#### Scenario: 开始加载

- WHEN loading=true
- THEN 按钮 SHALL 禁用并显示忙碌指示器，普通图标 SHALL 隐藏。

#### Scenario: 结束加载

- WHEN loading 从 true 改为 false
- THEN 按钮 SHALL 恢复加载前的 enabled 值或绑定。

#### Scenario: 图标选择

- WHEN iconSource 非空且不处于 loading
- THEN 按钮 SHALL 优先显示 iconSource 而非 icon.source。

### Requirement: R03 TextField

CatTextField SHALL 使用标准文本输入行为并展示主题化占位和无效状态。

#### Scenario: 默认对齐

- WHEN 未覆盖 verticalAlignment
- THEN 输入和占位文字 SHALL 垂直居中。

#### Scenario: 占位隐藏

- WHEN 存在正文或输入法预编辑文字
- THEN 占位文本 SHALL 隐藏。

#### Scenario: 无效提示

- WHEN invalid=true
- THEN 控件 SHALL 展示无效状态外观，SHALL NOT 因该标记自行修改输入文本。

### Requirement: R04 CheckBox

CatCheckBox SHALL 使用 Qt 复选状态并提供主题化指示。

#### Scenario: 选中

- WHEN checkState 为 Checked
- THEN 控件 SHALL 显示勾选标记。

#### Scenario: 键盘焦点

- WHEN 控件可用且通过键盘聚焦
- THEN 控件 SHALL 支持 StrongFocus 的焦点交互。

### Requirement: R05 ChatSubmission

CatChatInput SHALL 对消息去除首尾空白后提交。

#### Scenario: 有效提交

- WHEN 调用 submit() 且去空白后非空
- THEN 控件 SHALL 发出 submitted(trimmedText) 并清空输入。

#### Scenario: 空白提交

- WHEN 输入仅为空白且调用 submit()
- THEN 控件 SHALL NOT 发出 submitted。

#### Scenario: 提交入口

- WHEN 用户按输入框回车或点击可用发送按钮
- THEN 控件 SHALL 执行相同提交行为。

### Requirement: R06 OverlayScrollBars

CatScrollBar SHALL 暴露 `revealed` 属性并默认 `true`。当 `revealed` 为 `false` 且滑块未按下、未滚动、未悬停时，滚动条 SHALL 淡出到不可见；淡出状态 SHALL 仍能响应指针进入并重新显示，SHALL 仍可拖动。`AlwaysOn` 策略的滚动条 SHALL 始终可见。

CatScrollView SHALL 在指针位于视图范围内、滚动中、拖动中或悬停滑块时显示水平和垂直滚动条；指针离开且滚动停止后 SHALL 隐藏。滚动条 SHALL 以叠加方式呈现，SHALL NOT 因显示或隐藏改变内容可用宽度或高度。

#### Scenario: 指针进入与离开

- WHEN 指针进入 CatScrollView 视图范围
- THEN 水平和垂直滚动条 SHALL 按主题动效时长淡入
- WHEN 指针离开且没有滚动、拖动或滑块悬停
- THEN 两个滚动条 SHALL 淡出到不可见

#### Scenario: 视图外滚动

- WHEN 指针位于视图外时通过滚轮、拖动或触摸使视图滚动
- THEN 滚动条 SHALL 在滚动期间显示；滚动停止后 SHALL 恢复隐藏

#### Scenario: 独立使用与 AlwaysOn

- WHEN CatScrollBar 独立使用或策略为 AlwaysOn
- THEN 滚动条 SHALL 不受指针位置影响保持可见；`revealed` SHALL 默认为 `true`

#### Scenario: 减少动效

- WHEN 主题启用 reducedMotion
- THEN 滚动条的显示与隐藏 SHALL 直接切换，不产生动画过渡

### Requirement: R07 WindowCaptionButtons

CatTitleBar SHALL 以绘制图标提供最小化、最大化／还原和关闭按钮，并为正常、悬停、按下、禁用和键盘焦点状态提供可区分外观。关闭按钮悬停 SHALL 使用主题危险色背景和对比色叉号，按下 SHALL 使用更深的危险色；最小化和最大化按钮悬停 SHALL 使用 `surfaceHover`。禁用按钮 SHALL 使用禁用前景色且不执行窗口操作，键盘焦点 SHALL 显示主题主色描边。关闭按钮静止时 SHALL 使用标题栏表面色，SHALL NOT 显示常驻的危险色边框。

最大化按钮图标 SHALL 随 `targetWindow.visibility` 在最大化与还原之间切换，点击 SHALL 执行对应操作。

#### Scenario: 关闭按钮悬停与按下

- WHEN 指针位于关闭按钮上
- THEN 按钮背景 SHALL 变为主题危险色，叉号 SHALL 使用与背景对比的颜色
- WHEN 按下关闭按钮
- THEN 背景 SHALL 使用更深的危险色，松开后 SHALL 请求关闭窗口

#### Scenario: 最大化状态

- WHEN targetWindow 进入最大化
- THEN 最大化按钮 SHALL 显示还原图标，点击 SHALL 还原窗口
- WHEN targetWindow 退出最大化
- THEN 按钮 SHALL 显示单框最大化图标，点击 SHALL 最大化窗口

#### Scenario: 禁用与焦点

- WHEN targetWindow 为空
- THEN 三个按钮 SHALL 显示禁用色且 SHALL NOT 执行窗口操作
- WHEN 通过键盘将焦点移动到任一按钮
- THEN 按钮 SHALL 显示主题主色焦点描边

### Requirement: R08 TabCell

`CatTabView` 的每个页签 SHALL 是单一单元格：标题与关闭目标共享同一背景和边框，关闭目标 SHALL 完全位于页签边界内，且 SHALL NOT 改变当前页签。

#### Scenario: 单元格边界

- WHEN `tabsClosable=true`
- THEN 每个页签的标题与关闭目标 SHALL 位于同一圆角背景与边框内，关闭目标 SHALL 完全落在该边界内。

#### Scenario: 关闭符号对齐

- WHEN `tabsClosable=true`
- THEN 关闭符号 SHALL 与页签标题垂直居中于同一基线，SHALL NOT 因关闭目标尺寸小于单元格高度而向上偏移。

#### Scenario: 关闭不切换

- WHEN 点击非当前页签的关闭目标
- THEN 控件 SHALL 发出 `closeRequested(索引)`，`selectedIndex` SHALL NOT 改变。

#### Scenario: 选中外观

- WHEN 页签被选中
- THEN 单元格背景 SHALL 使用 `surfaceHover`，边框 SHALL 使用主题 `primary` 且宽度为 1。

#### Scenario: 键盘焦点

- WHEN 页签标题通过键盘获得焦点
- THEN 单元格 SHALL 显示主题 `primary` 边框。

#### Scenario: 悬停

- WHEN 指针位于页签单元格的非关闭区域
- THEN 单元格背景 SHALL 使用 `surfaceHover`。
- WHEN 指针位于关闭目标
- THEN 关闭目标 SHALL 显示圆角危险色背景和对比色符号，单元格背景 SHALL NOT 因此改变选中状态。

#### Scenario: 不可关闭

- WHEN `tabsClosable=false`
- THEN 关闭目标 SHALL NOT 显示，且单元格宽度 SHALL NOT 为关闭目标预留空间。

#### Scenario: 关闭目标无障碍名称

- WHEN `tabsClosable=true`
- THEN 每个关闭目标 SHALL 提供带页签序号的辅助名称。

### Requirement: R09 CommandBarOverflow

`CatCommandBar` 在有次要操作时 SHALL 显示可辨识的溢出按钮；按钮 SHALL 容纳自身字形，SHALL 与主操作采用一致的扁平样式，并 SHALL 提供非空辅助名称。

#### Scenario: 字形可用

- WHEN 存在至少一个次要操作且密度为 desktop、touch 或 mobile
- THEN 溢出按钮的可用内容宽度 SHALL 大于零，省略号 SHALL 可见。

#### Scenario: 无次要操作

- WHEN `secondaryActions` 为空
- THEN 溢出按钮 SHALL NOT 可见。

#### Scenario: 打开菜单

- WHEN 按下溢出按钮
- THEN SHALL 打开包含全部次要操作的菜单，菜单项数量 SHALL 等于 `secondaryActions.length`。

#### Scenario: 样式一致

- WHEN 溢出按钮与主操作同时显示
- THEN 溢出按钮 SHALL 使用扁平背景，SHALL NOT 显示下拉箭头或独立边框。

#### Scenario: 辅助名称

- WHEN 辅助技术读取溢出按钮
- THEN 名称 SHALL 非空，且 SHALL 表达“更多命令”含义。

## Data Model

本模块无独立持久化数据模型。已核查属性：CatButton primary/danger/loading 默认 false，iconSource 默认空；CatTextField invalid 默认 false，implicitWidth=220；CatChatInput text/placeholderText 为字符串别名，submitted 参数为 string；CatCheckBox 使用 Qt checkState；CatScrollBar.revealed 默认 true；CatTitleBar.maximized 为只读派生状态。窗口按钮和滚动条状态不写入持久化数据。

## Known Deviations / Technical Debt

- 当前规约不是全部 Cat 类型的逐项 API 覆盖；组件目录仍包含更多公开控件。
- Qt 基类提供的接口未在本基线中逐条复制。
- 显式 height、布局约束和应用绑定可覆盖主题推荐尺寸，主题尺寸不是强制下限。
- 叠加滚动条的指针显隐目前由 CatScrollView 绑定；直接为 ListView、GridView 等视图创建 CatScrollBar 时仍默认常驻，需调用方自行设置 revealed。
- 标题栏按钮的静止背景使用主题 surface；宿主完全替换标题栏背景时需同时覆盖按钮背景。

## Non-Goals

- 本模块 SHALL NOT 将内部子对象树、id 或 SurfaceImpl 作为公共接口。
- 本模块 SHALL NOT 自动发送消息、持久化输入或决定应用布局。

## Assumptions

- 宿主 SHALL 提供可用 QML 引擎、字体和主题；专用组件 SHALL 满足各自模块的外部前提。
