# ui-components Specification (delta)

## ADDED Requirements

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
