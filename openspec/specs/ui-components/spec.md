# ui-components Specification

## Purpose

为 Qt Quick 应用提供主题化控件。此基线记录已核查的基础按钮、文本输入、复选框和消息提交行为；专用控件的业务契约归属对应模块。

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

## Data Model

本模块无独立持久化数据模型。已核查属性：CatButton primary/danger/loading 默认 false，iconSource 默认空；CatTextField invalid 默认 false，implicitWidth=220；CatChatInput text/placeholderText 为字符串别名，submitted 参数为 string；CatCheckBox 使用 Qt checkState。

## Known Deviations / Technical Debt

- 当前规约不是全部 Cat 类型的逐项 API 覆盖；组件目录仍包含更多公开控件。
- Qt 基类提供的接口未在本基线中逐条复制。
- 显式 height、布局约束和应用绑定可覆盖主题推荐尺寸，主题尺寸不是强制下限。

## Non-Goals

- 本模块 SHALL NOT 将内部子对象树、id 或 SurfaceImpl 作为公共接口。
- 本模块 SHALL NOT 自动发送消息、持久化输入或决定应用布局。

## Assumptions

- 宿主 SHALL 提供可用 QML 引擎、字体和主题；专用组件 SHALL 满足各自模块的外部前提。
