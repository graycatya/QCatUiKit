# Design: 表格列显示与列顺序

## 分层

```
TableModel
  ↓
GalleryTableSortModel（排序，按源列）
  ↓
GalleryTableColumnModel（可见性/顺序）
  ↓
CatTableView
  ↓
ColumnReverseProxyModel（RTL 列反转）
```

列配置代理只做列映射，不保存排序状态；排序代理只做行排序，不知道列配置。

## 列代理

- `m_order` 保存所有源列的配置顺序。
- `m_hidden` 保存隐藏源列。
- `visibleColumns` 返回过滤后的源列顺序。
- `mapToSource` 将列配置代理列映射到排序代理列。
- `mapFromSource` 将排序代理列映射回列配置代理列。
- 隐藏最后一列时拒绝操作。
- 源模型列增加/删除/重置时重建配置，避免悬空列号。

## Gallery 选择与排序

- 表头视觉列 → `CatTableView.mapToSource`（RTL）→ 列配置代理 `mapToSource` → 排序代理列。
- 排序指示器使用该映射后的排序代理列。
- 行选择恢复先经排序代理映射源行，再使用列配置代理的可见首列创建视图索引。
- `GalleryTableSortModel.selectSourceRow` 接收列配置代理和视图模型，避免 QML 直接传 `QModelIndex` 触发 Qt 注解告警。

## UI

Gallery 表格下方增加列控制面板：

- 每个源列一个复选框
- 左右移动按钮
- 重置按钮
- 列名称来自排序代理的 headerData

## 测试

- C++：列映射、隐藏/恢复、顺序移动、最后一列保护、数据/表头转发。
- Gallery：隐藏/恢复列、移动列、排序后列映射、RTL 和冻结基本回归。
