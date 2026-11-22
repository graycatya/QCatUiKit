## ADDED Requirements

### Requirement: R15 TableAccessibility

`CatTableView` 及其公开单元格/表头基类 SHALL 暴露 QML `Accessible` 语义，使辅助技术能够识别表格、列头、单元格和选中状态；表格键盘导航 SHALL 在普通、冻结和 RTL 布局下保持一致。

#### Scenario: 表格与单元格语义

- WHEN 创建 `CatTableView`
- THEN 根对象 SHALL 使用 `Accessible.Table` 角色，并提供可配置的 `accessibleName` 和行列摘要
- WHEN 默认或自定义 delegate 使用 `CatTableCell`
- THEN 单元格 SHALL 使用 `Accessible.Cell` 角色
- THEN `Accessible.name` SHALL 默认使用显示文本，`Accessible.selected` SHALL 跟随单元格选中状态

#### Scenario: 表头语义和排序描述

- WHEN 默认或自定义 delegate 使用 `CatTableHeaderCell`
- THEN 表头 SHALL 使用 `Accessible.ColumnHeader` 角色
- THEN `Accessible.name` SHALL 默认使用表头显示文本
- WHEN 应用提供排序状态
- THEN 应用 SHALL 通过 `accessibleDescription` 或 `Accessible.description` 暴露当前排序方向
- THEN 排序状态变化 SHALL 更新辅助技术可读描述

#### Scenario: 键盘跨页导航

- WHEN 表格或冻结分区获得焦点且存在行
- THEN `PageUp` / `PageDown` SHALL 按当前行高移动至少一行，并保持在有效行范围
- THEN `Ctrl+Home` SHALL 移动到可视首行首列，`Ctrl+End` SHALL 移动到可视末行末列
- THEN RTL 下 `Ctrl+Home` / `Ctrl+End` 的列方向 SHALL 与视觉列顺序一致
- THEN 导航 SHALL 更新当前选择并把目标单元格滚动到可见区域
- WHEN 正在编辑
- THEN 跨页导航 SHALL NOT 改变选择或关闭编辑器

#### Scenario: 默认行为

- WHEN 未设置 `accessibleName` 或未提供排序描述
- THEN 表格 SHALL 使用默认名称和空描述
- THEN 现有选择、编辑、冻结、RTL 和排序行为 SHALL 保持不变
