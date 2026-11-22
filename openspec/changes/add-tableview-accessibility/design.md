# Design: 表格可访问性

## 语义层

- `CatTableView` 作为 Table 容器，提供 `accessibleName`（默认 `qsTr("Table")`）和动态行列摘要。
- `CatTableCell` 作为 Cell，名称默认取 `display`，`Accessible.selected` 绑定 `selected`。
- `CatTableHeaderCell` 作为 ColumnHeader，名称默认取 `display`；新增 `accessibleDescription` 供应用写入排序方向。
- Gallery 排序表头通过 `sortDescription(visualColumn)` 生成 `Sorted ascending` / `Sorted descending`，不把排序状态放进库。

## 键盘层

- 新增 `__handlePageKey(event)`，只处理 `PageUp`、`PageDown`、`Ctrl+Home`、`Ctrl+End`。
- 普通视图和四个冻结分区都挂接该函数；冻结分区先处理跨页键，再回落到现有 `__handleKey` 处理方向键和 Home/End。
- 翻页行数按当前视口高度和实际行高计算，至少 1 行。
- 导航复用 `sharedSelection` 和 `__ensureCellVisible`；普通视图补 `positionViewAtCell` 滚动。

## 焦点层

- `CatTableView` 声明为可聚焦，并在任一视图获得焦点时显示主题色焦点边框。
- 不改变 TableView 自身的键盘焦点模型；跨页键只在有行、有列且非编辑态时生效。

## 测试

- `tst_tablecells.qml` 验证 Cell/ColumnHeader 角色、名称和选中状态。
- 新增 `tst_tableaccessibility.qml` 验证 Table 角色、默认名称、PageUp/PageDown、Ctrl+Home/End、RTL 和冻结。
- Gallery `test_tableSorting` 验证排序表头的 Accessible 名称和方向描述。
