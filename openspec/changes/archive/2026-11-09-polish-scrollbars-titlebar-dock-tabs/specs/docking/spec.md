# Spec Delta

## ADDED Requirements

### Requirement: R05 TabChrome

每个停靠标签 SHALL 在一个连续单元格内同时呈现标题和可选的关闭入口；单元格背景 SHALL 覆盖标题与关闭入口，SHALL NOT 为关闭入口保留脱离标签的独立区域。

活动标签的单元格 SHALL 使用表面色背景和贯穿整个单元格的 2px 主题主色下划线；非活动标签悬停时 SHALL 使用 `surfaceHover` 整格高亮。关闭入口悬停 SHALL 在整格高亮基础上显示圆角危险色底和危险色叉号，按下 SHALL 加深该反馈。键盘焦点位于标题或关闭入口时 SHALL 显示主题主色描边。

浮动窗口中的标签 SHALL 使用与主工作区一致的单元格、活动下划线和关闭入口外观。

#### Scenario: 活动标签

- WHEN 标签为活动标签
- THEN 单元格 SHALL 显示表面色背景，2px 主色下划线 SHALL 覆盖标题与关闭入口
- AND 关闭入口 SHALL 位于标题右侧且同属该单元格背景

#### Scenario: 非活动标签悬停

- WHEN 指针位于非活动标签的标题或关闭入口上
- THEN 整个单元格 SHALL 显示 `surfaceHover` 背景

#### Scenario: 关闭入口悬停与按下

- WHEN 指针位于关闭入口上
- THEN 关闭入口 SHALL 显示圆角危险色底和危险色叉号，单元格 SHALL 保持高亮
- WHEN 按下关闭入口
- THEN 危险色底 SHALL 加深

#### Scenario: 浮动窗口一致

- WHEN 标签组位于浮动窗口
- THEN 标签单元格、活动下划线和关闭入口 SHALL 使用与主工作区相同的布局和状态外观
