# Table column state validation

日期：2026-11-22。环境：Windows 11、Qt 6.12.0 MSVC Debug/Release、offscreen/software。

## 列宽 key

- `CatTableView.columnWidthKeyProvider` 将宽度映射到应用定义的稳定 key。
- Gallery 使用 `tableColumns.sourceColumn(column)` 作为 key；重排列、隐藏/恢复列后宽度跟随源列。
- 普通视图 provider 保留显式列宽和 delegate 隐式宽度兜底；未设置 key provider 时行为不变。
- RTL key 映射在普通视图 provider 和冻结切换路径中使用视觉列到源列的映射。

## 列配置

- `GalleryTableColumnModel` 保存源 header key、显示顺序 key 和隐藏 key。
- 行数据 reset 且 header key 不变时，不重建顺序/隐藏状态。
- 列新增/删除时保留仍存在的列并追加新列。
- 不涉及任何文件、`QSettings` 或应用重启恢复。

## 测试

- `gallery_data_demo`：
  - 隐藏/移动列后清空/恢复行，`visibleColumns` 保持
  - 列映射、最后一列保护、重置
- Gallery QML：
  - 拖动列宽后移动列，宽度按源列跟随
  - 清空/恢复数据后列顺序和隐藏状态保持
- Debug `qml_contracts`：485 passed、0 failed、4 skipped。
- Debug 完整 Gallery：868 passed、0 failed、3 skipped。
- Debug `gallery_data_demo`：通过。
- Release `qml_contracts`：485 passed、0 failed、4 skipped；完整 Gallery：868 passed、0 failed、3 skipped。
- `qmllint`、`check_docs.py`、`git diff --check`、OpenSpec 严格校验：通过。

## 已知边界

- 仅运行期内存状态，不写文件。
- Gallery 使用 header key（包含源列号）作为稳定标识；真实应用应使用自己的稳定列 ID。
- 应用重启后的恢复由应用负责。
- 列宽 key 当前由 Gallery 应用层提供，库不持久化。
