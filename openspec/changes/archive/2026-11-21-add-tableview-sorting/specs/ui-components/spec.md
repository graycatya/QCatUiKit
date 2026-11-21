## ADDED Requirements

### Requirement: R14 TableSortingIntegration

应用 SHALL 可以通过 `QSortFilterProxyModel` 或等价排序代理为 `CatTableView` 提供行排序；`CatTableView` SHALL NOT 内置排序状态。视觉列 SHALL 通过 `mapToSource` 映射为排序代理列，RTL 和冻结布局 SHALL 共享同一排序结果。

#### Scenario: 表头点击排序

- WHEN `headerDelegate` 收到表头点击
- THEN 视觉列 SHALL 经 `CatTableView.mapToSource` 映射为排序代理列
- THEN 排序代理 SHALL 按该列和当前方向排序，并在同一列再次点击时切换方向
- THEN 表头 SHALL 显示当前排序列和方向，非排序列不显示指示

#### Scenario: RTL 列映射

- WHEN RTL 生效且用户点击某个视觉表头
- THEN 排序 SHALL 使用映射后的源列，SHALL NOT 使用视觉列号直接排序
- WHEN 源列 0 显示在最右
- THEN 点击最右侧表头 SHALL 排序源列 0

#### Scenario: 冻结与选择

- WHEN 冻结 1/1 或 RTL 四区生效
- THEN 四区 SHALL 共享排序代理和同一行顺序
- THEN 排序后 SHALL 尽量保持当前源行选中；源行不存在时清除选择
- THEN 排序 SHALL 关闭正在进行的编辑，避免编辑索引跨排序失效

#### Scenario: 默认行为

- WHEN 未接入排序代理
- THEN `CatTableView` 的布局、滚动、选择、编辑和现有 delegate 行为 SHALL 保持不变
- THEN 库 SHALL NOT 因本变更新增公开排序 API
