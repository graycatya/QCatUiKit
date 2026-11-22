# Design: 表格列宽与列配置内存状态

## 列宽 key

- `CatTableView.columnWidthKeyProvider(column)` 返回应用定义的稳定 key。
- `__columnWidths` 从按模型列号索引改为按 key 索引。
- `__setColumnWidth`、`__seedWidths`、`__captureImplicitSizes` 和冻结切换的 `__applyWidths` 都使用 key。
- 普通视图 provider 的查找顺序：
  1. key 对应内存宽度
  2. 外部 `columnWidthProvider`
  3. Qt 显式列宽
  4. delegate 隐式列宽
  5. 默认宽度
- 未设置 key provider 时 key 就是模型列号，现有行为不变。

## Gallery key

- `columnWidthKeyProvider: column => tableColumns.sourceColumn(column)`。
- key 是列配置代理的源列（排序代理列），不随可视列顺序变化。
- 列隐藏时宽度仍留在映射；恢复后复用。

## 列配置保留

- 列代理保存 source header keys、当前顺序 keys、隐藏 keys。
- `modelReset` 只比较 source keys：
  - 相同：行数据重置，不改配置。
  - 不同：按 key 保留仍存在的列，追加新列，删除失效列。
- `columnsInserted/Removed` 走同一套 key 重建。
- 索引只作为当前映射缓存，不作为长期状态。

## 边界

- 应用重启恢复由应用完成，库不做文件 I/O。
- header key 依赖列名稳定；商用模型应使用稳定列 ID 或字段名。
- 列宽 key 在 Gallery 固定 schema 示例中使用源列号。

## 测试

- Gallery：拖动宽度后重排列，宽度跟随源列；清空/恢复数据后顺序、隐藏和宽度保持。
- C++：列代理行重置后保留配置；新增/删除列时保留有效列。
- 回归：自定义 delegate 宽度、冻结切换、RTL、完整 QML/Gallery。
