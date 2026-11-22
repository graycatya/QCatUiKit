## ADDED Requirements

### Requirement: R16 TableColumnControls

应用 SHALL 可以通过列配置代理控制 `CatTableView` 的可见列和列顺序；`CatTableView` SHALL NOT 内置列配置状态。视觉列 SHALL 经列配置代理和 RTL 代理映射到排序代理及源列。

#### Scenario: 隐藏和恢复列

- WHEN 应用隐藏一个可见源列
- THEN 表格可见列数 SHALL 减少，隐藏列 SHALL 不再接收选择、编辑或列宽操作
- THEN 剩余列 SHALL 保持源顺序和当前排序结果
- WHEN 应用恢复该列
- THEN 该列 SHALL 回到配置顺序中的原位置
- WHEN 只剩一列可见
- THEN 应用 SHALL NOT 允许再隐藏该列

#### Scenario: 调整列顺序

- WHEN 应用把某个源列向前或向后移动
- THEN 可见列 SHALL 按新顺序显示
- THEN 表头、单元格、列宽和冻结边界 SHALL 使用同一列映射
- THEN RTL 下显示顺序 SHALL 在配置顺序基础上再反转

#### Scenario: 排序和选择

- WHEN 列顺序或可见性变化后点击表头
- THEN 视觉列 SHALL 映射到列配置代理，再映射到排序代理列
- THEN 排序指示器 SHALL 显示在该源列对应的当前视觉表头
- WHEN 排序或列配置变化后恢复选中行
- THEN 应用 SHALL 用可见列中的首列恢复行选择
- THEN 所有列都隐藏的状态 SHALL 被拒绝

#### Scenario: 默认行为

- WHEN 未设置列配置代理
- THEN `CatTableView` 的布局、滚动、选择、编辑、冻结和排序行为 SHALL 保持不变
- THEN 库 SHALL NOT 因本变更新增公开列配置 API
