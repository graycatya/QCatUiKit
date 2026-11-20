# Spec Delta

## ADDED Requirements

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
- THEN `Home` / `End`、`Tab` 和 `Enter` 的视觉方向 SHALL 与列顺序一致。
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
