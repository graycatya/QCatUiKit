# Tasks

## 1. 排序代理

- [x] 1.1 新增 Gallery 内部 `GalleryTableSortModel`（`QSortFilterProxyModel`）：`sourceTableModel`、`toggleSort(sourceColumn)`、`clearSort()`、`sortColumn`/`sortOrder`/`sorted`；只在 Gallery CMake 登记。
- [x] 1.2 增加排序代理单元测试：升序、降序、同列切换、切列、清除、空模型和源模型行数。

## 2. Gallery 接入

- [x] 2.1 Gallery 表格改用“源模型 → 排序代理 → CatTableView”分层；RTL 仍由 CatTableView 叠加列反转代理。
- [x] 2.2 自定义 `headerDelegate` 点击表头时经 `mapToSource` 映射视觉列，显示升/降序指示；补中英文文案。
- [x] 2.3 排序前关闭编辑器；点击行和恢复选择时在视图/排序代理/源模型之间映射，排序后尽量保持源行。

## 3. 测试与文档

- [x] 3.1 Gallery QML 测试：LTR/RTL、无冻结/1/1、升序/降序、表头指示、选择和编辑回归。
- [x] 3.2 更新 `COMPONENTS.md` 和 `VALIDATION.md`，说明排序代理分层和视觉列映射。
- [x] 3.3 运行 QML、models、Gallery、qmllint、`check_docs.py`、`git diff --check` 和 OpenSpec 严格校验，写入 `validation.md`。
