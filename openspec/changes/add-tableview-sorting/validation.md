# Table sorting integration validation

日期：2026-11-21。环境：Windows 11、Qt 6.12.0 MSVC Debug、offscreen/software。

- `GalleryTableSortModel` 作为 Gallery 内部 `QSortFilterProxyModel`：支持升/降序切换、切列、清除、源行/代理行映射和按源行选择；不进入库 ABI。
- Gallery 表格改为 `TableModel → GalleryTableSortModel → CatTableView`；RTL 由 CatTableView 再叠加列反转代理。
- `CatTableView` 新增 `sourceColumn(visualColumn)`：LTR 恒等，RTL 返回 `N - 1 - visualColumn`；排序表头用它映射视觉列，避免直接使用表头列号。
- Gallery `Sortable header` 开启自定义表头，点击表头排序并显示 `▲`/`▼`；排序前关闭编辑器，排序后按源行恢复选择。
- 行选择通过 C++ 行映射 + `QItemSelectionModel` 设置，避免 QML 直接传递 `QModelIndex` 触发 Qt 6.12 注解告警。

## 测试

- `gallery_data_demo`：排序代理升序、降序、切列、清除、映射、空源和源数据顺序不变。
- `GalleryDataExamples::test_tableSorting`：LTR 升/降序、表头指示、RTL 最右表头映射源列 0、冻结 1/1 表头排序。
- `tst_tablertl.qml` 增加 `sourceColumn` 的 LTR/RTL 断言。
- `ctest -R '^qml_contracts$'`：475 passed、0 failed、4 skipped。
- 完整 Gallery：864 passed、0 failed、3 skipped。
- Gallery/qml `qmllint`、`check_docs.py`、`git diff --check`、OpenSpec 严格校验：通过。

## 已知边界

- 排序是 Gallery 应用层集成，不是 `CatTableView` 内置状态；应用使用排序代理时应把 `table.model` 视为排序代理，并通过 `sourceColumn`/映射方法访问源列。
- 当前只演示单列排序；多列、筛选和服务端排序不在本变更范围。
