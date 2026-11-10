# ui-components Specification (delta)

## ADDED Requirements

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
