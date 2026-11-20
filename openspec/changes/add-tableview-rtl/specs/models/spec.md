# Spec Delta

## ADDED Requirements

### Requirement: R06 ColumnReverseProxyModel

`ColumnReverseProxyModel` SHALL 是一个公开的列反转代理模型：行索引与源模型一致，列索引按 `proxyColumn = sourceColumnCount - 1 - sourceColumn` 映射；`data`、`flags`、`headerData`、`setData`、`index`、`parent`、`mapToSource` 和 `mapFromSource` SHALL 正确转换。代理 SHALL NOT 复制行或列数据。

#### Scenario: 列映射

- WHEN 源模型有 N 列
- THEN 代理列 0 SHALL 对应源列 N-1，代理列 N-1 SHALL 对应源列 0。
- WHEN 代理收到有效代理索引
- THEN `mapToSource` SHALL 返回对应源索引，`mapFromSource` 往返 SHALL 回到原代理索引。

#### Scenario: 数据、角色与表头

- WHEN 视图通过代理读取 `display`、`edit` 或其他角色
- THEN 返回值 SHALL 与源模型对应行列一致。
- WHEN 读取代理的 `headerData`
- THEN 文本 SHALL 与源模型对应源列一致，列方向反转。

#### Scenario: 编辑写回

- WHEN 通过代理调用 `setData(proxyIndex, value, role)`
- THEN 写入 SHALL 落在对应源索引，并转发源模型的 `dataChanged`。
- WHEN 源索引只读或写入失败
- THEN 代理 SHALL 返回 `false`，SHALL NOT 修改源数据。

#### Scenario: 列结构变化

- WHEN 源模型插入、删除或移动列
- THEN 代理列数和映射 SHALL 更新，代理 SHALL 发出对应的列变化通知；第一版可在移动/重置时使用模型重置。
- WHEN 源模型重置
- THEN 代理 SHALL 重置并恢复列反转映射。

#### Scenario: 空模型与非法索引

- WHEN `sourceModel` 为空或列数为 0
- THEN `rowCount` 和 `columnCount` SHALL 返回 0。
- WHEN 收到无效索引
- THEN 映射 SHALL 返回无效索引，SHALL NOT 崩溃或告警。

#### Scenario: 复杂度

- WHEN 映射任意代理/源索引
- THEN 时间复杂度 SHALL 为 O(1)，SHALL NOT 遍历模型或复制数据。
