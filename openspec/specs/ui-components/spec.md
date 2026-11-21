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

### Requirement: R10 ToolTipPointerPlacement

`CatToolTip` 在悬停交互启用时 SHALL 在指针位置附近显示，并在显示后保持位置；提示 SHALL 按指针、输入和窗口状态关闭，且关闭后同一次悬停 SHALL NOT 自动重新显示。

#### Scenario: 指针定位

- WHEN 指针在父项内停稳并达到 delay
- THEN 提示 SHALL 在指针右下方显示，水平偏移 12、垂直偏移 16 逻辑像素。

#### Scenario: 显示后固定位置

- WHEN 提示已显示且指针在父项内继续移动
- THEN 提示位置 SHALL 保持不变。

#### Scenario: 移动重置延迟

- WHEN 提示尚未显示且指针移动
- THEN delay SHALL 重新计时，提示 SHALL NOT 在重新计时完成前显示。

#### Scenario: 点击关闭与取消待显示

- WHEN 用户在 delay 等待期间点击父项
- THEN 提示 SHALL NOT 显示。
- WHEN 提示已显示且用户点击
- THEN 提示 SHALL 关闭。

#### Scenario: Esc 与窗口失活

- WHEN 提示已显示且用户按 Esc
- THEN 提示 SHALL 关闭。
- WHEN 提示显示期间窗口失去激活状态
- THEN 提示 SHALL 关闭。

#### Scenario: 移出与重新进入

- WHEN 指针移出父项
- THEN 提示 SHALL 关闭。
- WHEN 指针重新进入父项并达到 delay
- THEN 提示 SHALL 再次显示。

#### Scenario: 关闭后同次悬停不重开

- WHEN 提示因点击、Esc 或 timeout 关闭且指针仍停留在父项内
- THEN 指针移动 SHALL NOT 使提示重新显示；指针移出后重新进入并达到 delay 后 SHALL 恢复显示。

#### Scenario: 窗口边缘避让

- WHEN 指针下方空间不足以容纳提示及 8 逻辑像素窗口边距
- THEN 提示 SHALL 显示在指针上方，垂直偏移 16 逻辑像素。
- WHEN 指针靠近窗口右侧
- THEN 提示 SHALL 保持在窗口内容区域内。

#### Scenario: 无悬停时回退定位

- WHEN 指针不在父项内而提示被打开（例如键盘焦点触发）
- THEN 提示 SHALL 显示在父项下方并水平居中。

#### Scenario: 应用控制模式

- WHEN `hoverInteractionEnabled=false`
- THEN 提示 SHALL 保留应用设置的可见性和 x/y，SHALL NOT 因指针移动重置延迟，也 SHALL NOT 因点击、移出或窗口失活被自动关闭；Esc 快捷关闭 SHALL 不生效。

### Requirement: R13 TableRtlColumns

`CatTableView` SHALL 在 RTL 布局方向下按起始侧（右侧）显示列，并通过列反转代理保持选择、编辑、键盘和列宽行为正确；LTR 行为 SHALL 保持不变。RTL 模式由继承的 `LayoutMirroring.enabled` 决定，`frozenColumnCount` / `frozenRowCount` 仍表示源模型起始侧的连续数量。

#### Scenario: 无冻结 RTL 列顺序

- WHEN `LayoutMirroring.enabled=true` 且 `frozenColumnCount=0`、`frozenRowCount=0`
- THEN 源列 0 SHALL 显示在最右侧，源列 N-1 SHALL 显示在最左侧。
- THEN 初始滚动位置 SHALL 位于内容右端，水平滚动条滑块初始在右端。
- WHEN 用户向左滚动
- THEN 水平滚动条 SHALL 按 RTL 方向镜像移动。

#### Scenario: 四区与表头镜像

- WHEN RTL 且 `frozenColumnCount>0`
- THEN 主体区 SHALL 在左、冻结列区 SHALL 在右；冻结行仍在顶部。
- THEN 表头 SHALL 分为左侧主体段和右侧冻结段，两段列宽、文本与单元格一致。
- THEN 垂直滚动条 SHALL 位于左侧并跨四区高度，避免覆盖右侧冻结列。

#### Scenario: 选择与索引映射

- WHEN 用户在任一区点击单元格
- THEN 选择 SHALL 写入共享代理选择模型，视觉高亮 SHALL 与点击单元格一致。
- WHEN 读取 `view.selectionModel.currentIndex`
- THEN 返回的 SHALL 是代理索引；`mapToSource` SHALL 返回源索引，`mapFromSource` 可往返。
- WHEN 应用用源索引定位或设置当前项
- THEN 应经 `mapFromSource` 转换后仍定位到同一源单元格。

#### Scenario: 编辑写回

- WHEN RTL 下通过 `editDelegate` 或 `commitEdit(value)` 提交编辑
- THEN 写入 SHALL 经代理映射到源索引，`dataChanged` SHALL 正常发出。
- WHEN 提交失败或用户取消
- THEN 编辑态 SHALL 保留或关闭，SHALL NOT 写错列。
- THEN 进入编辑时关闭其他区编辑器的行为 SHALL 与 LTR 一致。

#### Scenario: 键盘方向

- WHEN RTL 下焦点位于主体区并向右移动到冻结边界
- THEN 按 `Right` SHALL 进入冻结区，按 `Left` SHALL 返回主体区。
- THEN `Home` / `End` 的视觉方向 SHALL 与列顺序一致。
- WHEN 在冻结区继续移动到最右边界
- THEN 当前索引 SHALL 停在源列 0，SHALL NOT 越界。

#### Scenario: 列宽调整

- WHEN RTL 下表头显示列宽手柄
- THEN 手柄 SHALL 位于表头单元格视觉左边界，拖动方向 SHALL 与 LTR 相反。
- THEN 调整的 SHALL 是该表头对应的源列，四区宽度 SHALL 同步。
- WHEN `resizableColumns=false`
- THEN 手柄 SHALL 禁用。

#### Scenario: 模型列变化

- WHEN RTL 下源模型插入、删除或重置列
- THEN 代理列数、冻结侧列范围、表头和当前索引 SHALL 重新映射；滚动位置 SHALL 按新范围钳制。
- WHEN 模型为空
- THEN 表格 SHALL 显示空态，SHALL NOT 访问越界代理列。

#### Scenario: LTR 回归

- WHEN `LayoutMirroring.enabled=false`
- THEN 列顺序、滚动、选择、编辑、键盘和列宽 SHALL 与现有 LTR 行为一致。
- THEN `table.model` 与 `view.model` SHALL 保持同一源模型，映射方法为恒等映射。

#### Scenario: 性能

- WHEN 10 万行模型开启 RTL
- THEN 代理映射 SHALL 为 O(1)，SHALL NOT 复制行/列数据；存活 delegate 数量 SHALL 仍与视口相关，而非行数。

### Requirement: R11 TableFreezePanes

`CatTableView` SHALL 支持从起始侧连续冻结列和行，冻结区与主体 SHALL 共享同一模型、选择和编辑状态；冻结数量 SHALL 可由属性和方法修改，越界时 SHALL 静默钳制。

#### Scenario: 四区布局与滚动

- WHEN `frozenColumnCount>0` 且 `frozenRowCount>0`
- THEN 表格 SHALL 呈现左上、顶部、左侧和主体四区。
- WHEN 用户横向滚动主体
- THEN 顶部区和主体 SHALL 同步横向移动，左侧区和左上区 SHALL 保持水平位置不变。
- WHEN 用户纵向滚动主体
- THEN 左侧区和主体 SHALL 同步纵向移动，顶部区和左上区 SHALL 保持垂直位置不变。

#### Scenario: 滚动条

- WHEN `frozenColumnCount>0`
- THEN 水平滚动条 SHALL 只覆盖主体区宽度，垂直滚动条 SHALL 覆盖四区总高度。
- WHEN 冻结列数等于列数或冻结行数等于行数
- THEN 对应的水平或垂直滚动条 SHALL 隐藏。

#### Scenario: 表头分段

- WHEN `frozenColumnCount>0`
- THEN 冻结列表头 SHALL 固定显示，主体列头 SHALL 随主体横向滚动。
- THEN 两段表头 SHALL 使用与主体一致的列宽和表头文本。

#### Scenario: 选择与编辑一致

- WHEN 用户点击冻结区单元格
- THEN 选择 SHALL 写入与主体相同的 `selectionModel`，选中高亮 SHALL 跨区域一致。
- WHEN 冻结区单元格进入编辑并提交
- THEN `editDelegate` SHALL 与主体行为一致，提交 SHALL 写回同一模型索引。

#### Scenario: 键盘跨区导航

- WHEN 焦点位于主体区且用户按方向键移动到冻结列边界
- THEN 焦点 SHALL 能进入冻结区，并保持行列索引连续。
- WHEN 焦点位于冻结区且用户按方向键回到边界
- THEN 焦点 SHALL 能返回主体区。

#### Scenario: 列宽与行高一致

- WHEN 用户调整冻结区或主体区的列宽
- THEN 同一列在四个区域 SHALL 使用相同宽度。
- WHEN `rowHeightProvider` 对某行返回高度
- THEN 冻结区和主体区中同一行 SHALL 使用相同高度。

#### Scenario: 越界与模型变化

- WHEN `frozenColumnCount` 或 `frozenRowCount` 为负数或超过当前列数／行数
- THEN 数值 SHALL 钳制到 `[0, 列数]` 或 `[0, 行数]`，SHALL NOT 输出告警。
- WHEN 模型列数或行数减少到低于冻结数量
- THEN 冻结数量 SHALL 自动收敛，滚动位置和选中项 SHALL 保持并按新的可滚范围钳制。

#### Scenario: RTL

- WHEN 布局方向为从右向左
- THEN 冻结列 SHALL 位于右侧，水平滚动条 SHALL 镜像，冻结数量 SHALL 仍表示起始侧连续列数。

#### Scenario: 方法入口

- WHEN 调用 `freezeTo(row, column)`
- THEN 行为 SHALL 等价于设置 `frozenRowCount=row+1` 和 `frozenColumnCount=column+1`。
- WHEN 调用 `clearFreeze()`
- THEN `frozenRowCount` 和 `frozenColumnCount` SHALL 均变为 0。

#### Scenario: 冻结区超过视口

- WHEN 冻结区总宽或总高超过视口
- THEN 冻结区 SHALL 按自然尺寸占据，主体可被压缩到 0。
- THEN 冻结区 SHALL NOT 提供内部滚动。

#### Scenario: 无冻结

- WHEN `frozenColumnCount=0` 且 `frozenRowCount=0`
- THEN 布局、滚动和交互 SHALL 与现有 `CatTableView` 一致。

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
