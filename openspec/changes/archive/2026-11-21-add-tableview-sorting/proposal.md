# Table sorting integration

## Why

`CatTableView` 已支持冻结、RTL 和表头委托，但还没有完整的排序接入示例。R12 明确规定库不内置排序状态，排序应由应用模型和 `headerDelegate` 负责；当前缺少这条链路的实现与回归，RTL 下视觉列映射也容易被误用。

本变更只增加 Gallery 集成示例和排序代理模型，不改变 `CatTableView` 的公开契约。

## What Changes

- 新增 Gallery 内部的 `GalleryTableSortModel`（`QSortFilterProxyModel`）：包装 `TableModel`，提供 `toggleSort(sourceColumn)`、`clearSort()` 和 `sortColumn`/`sortOrder` 只读状态。
- Gallery 表格改用“源模型 → 排序代理 → CatTableView”的分层；RTL 时 CatTableView 再叠加列反转代理。
- 自定义 `headerDelegate` 显示升序/降序指示；点击表头时先通过 `mapToSource` 把视觉列映射回源列，再调用排序代理。
- 排序后保持源模型中的当前选中行（若仍存在），并关闭编辑器；冻结四区和 RTL 四区共享同一排序结果。
- 增加 Gallery 排序测试：LTR/RTL、无冻结/1/1、升序/降序、表头指示、选择与编辑回归。

## Impact

- 涉及 `examples/uikit/gallery/gallerytablesortmodel.{h,cpp}`、`GalleryDataPreview.qml`、Gallery CMake、中英文 TS 和 Gallery 测试。
- 不新增公开 C++ 头、导出符号或 QML 类型；`CatTableView` 不新增排序属性/方法。
- 排序代理位于 CatTableView 的 RTL 代理下层，视觉列映射仍由 `CatTableView.mapToSource` 完成。

## Non-Goals

- 不在 `CatTableView` 内实现排序状态、排序图标或点击排序。
- 不改变 `TableModel` 的排序行为，不把排序写回源数据顺序。
- 不实现多列排序、筛选或服务端排序。
