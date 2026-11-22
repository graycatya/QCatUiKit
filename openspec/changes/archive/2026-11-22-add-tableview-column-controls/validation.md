# Table column controls validation

日期：2026-11-22。环境：Windows 11、Qt 6.12.0 MSVC Debug/Release、offscreen/software。

## 分层

- `TableModel → GalleryTableSortModel → GalleryTableColumnModel → CatTableView → ColumnReverseProxyModel`。
- 列代理基于 `QIdentityProxyModel`，复用 Qt 的行/重置转发，只覆盖列映射。
- `mapToSource` / `mapFromSource`、`index`、`parent`、`sibling` 和 `columnCount` 使用列配置；避免 `parent()` 递归和隐藏首列时索引失效。

## 功能

- 隐藏/恢复源列，至少保留一列可见。
- 前后移动源列，恢复重置。
- 排序表头视觉列经 RTL、列配置代理和排序代理映射。
- 选择恢复使用列配置代理的可见首列。
- Gallery 数据页提供列复选框、左右移动和重置控件，补齐中英文文案。

## 测试

- `gallery_data_demo` C++ 测试：
  - 列映射、数据/表头转发
  - 隐藏/恢复、最后一列保护
  - 移动列、重置
  - 真实 `GalleryDataDemo` 清空/恢复行后列代理行数同步
- Gallery QML 测试：
  - 隐藏/恢复列后 `table.view.columns` 同步
  - 移动/重置列
  - 重排列后点击表头，排序列映射正确
- Debug / Release `qml_contracts`：485 passed、0 failed、4 skipped。
- Debug / Release 完整 Gallery：866 passed、0 failed、3 skipped。
- 列代理单元测试 `gallery_data_demo`：通过。
- Gallery qmllint、`check_docs.py`、`git diff --check`、OpenSpec 严格校验：通过。

## 已知边界

- 列配置是 Gallery 应用层能力，未进入公共 `CatTableView` API 或 C++ ABI。
- 列宽仍按视图列位置保存，重排列后宽度跟随位置而不是源列；如需按源列持久化，应另加列宽键。
- 当前通过左右按钮调整顺序，未实现拖拽表头排序。
- RTL 下顺序在配置顺序基础上再反转；未做真实屏幕阅读器人工验收。
