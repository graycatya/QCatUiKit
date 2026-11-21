# Table sorting integration design

## Architecture

```
TableModel (source)
  └─ GalleryTableSortModel (QSortFilterProxyModel)
       └─ CatTableView.model
            └─ ColumnReverseProxyModel (RTL only)
                 └─ four pane views
```

- 排序代理位于最靠近源模型的一层，只改变行顺序。
- CatTableView 的 RTL 列反转仍是最外层代理；`mapToSource` 从视觉列映射到排序代理列。
- 表头点击使用：

```qml
const visual = table.view.model.index(0, column)
const sourceColumn = visual.valid ? table.mapToSource(visual).column : column
sortModel.toggleSort(sourceColumn)
```

- 不能在 RTL 下直接使用表头 `column` 作为排序代理列。

## Model

`GalleryTableSortModel` 是 Gallery 内部类型：

- `sourceTableModel`：包装 `demo.tableModel`。
- `toggleSort(int sourceColumn)`：同列切换升/降序，不同列从升序开始。
- `clearSort()`：恢复源顺序。
- `sortColumn` / `sortOrder` / `sorted`：只读状态，供表头指示。
- 排序只影响代理行顺序，不修改 `TableModel` 的数据顺序。

## Selection and editing

- 排序前关闭编辑器。
- 保存当前源行主键（Gallery 使用源行号）并在排序后通过 `mapFromSource` 恢复；找不到时清除当前索引。
- 点击行时从视图索引映射回源行，避免把排序后的代理行号直接传给 `demo.select`。
- 冻结四区共享代理选择模型，排序后行顺序天然一致。

## Header delegate

- 扩展现有 Gallery 自定义表头：
  - 点击触发 `control.requestSort(column)`。
  - 指示文本根据 `sortModel.sortColumn` 和 `sortOrder` 显示 `▲`/`▼`。
- 不把排序状态放进 CatTableView；表头只是调用 Gallery 控制器。

## Tests

- 排序代理单元测试：升序、降序、切列、清除、源模型行数。
- Gallery QML：
  - LTR 点击第 1 列表头，检查升序/降序和指示。
  - RTL 点击最右表头，检查源列 0 排序。
  - 冻结 1/1 下四区行顺序一致。
  - 排序后编辑、选择仍写回正确源行。
- 完整 qml_contracts 与 Gallery 套件回归。

## Risks

- 外部排序代理会改变 `table.model` 的行顺序，应用必须通过映射方法访问源行。
- 如果未来在 CatTableView 外层再叠加代理，`mapToSource` 只映射一层；需在排序文档中明确分层顺序。
