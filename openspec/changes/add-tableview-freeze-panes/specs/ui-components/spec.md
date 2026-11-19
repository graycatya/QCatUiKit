# Spec Delta

## ADDED Requirements

### Requirement: R11 TableFreezePanes

`CatTableView` SHALL 支持从起始侧连续冻结列和行，冻结区与主体 SHALL 共享同一模型、选择和编辑状态；冻结数量 SHALL 可由属性、方法和表头抓手修改，越界时 SHALL 静默钳制。

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

#### Scenario: 抓手拖动

- WHEN 用户拖动表头抓手
- THEN 冻结数量 SHALL 实时更新，列宽调整热区 SHALL 保持原有行为。
- WHEN 用户拖动中按 Esc 或拖回起点
- THEN 冻结数量 SHALL 恢复为拖动前的值。
- WHEN 用户松开抓手
- THEN 新数量 SHALL 通过属性通知发出，滚动位置和选中项 SHALL 保持不变。

#### Scenario: RTL

- WHEN 布局方向为从右向左
- THEN 冻结列 SHALL 位于右侧，抓手和水平滚动条 SHALL 镜像，冻结数量 SHALL 仍表示起始侧连续列数。

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
