# Proposal: 表格列宽与列配置内存状态

## Why

列显示/顺序接入后，列宽仍按视图列位置保存，重排列后宽度会跟着位置而不是源列；`GalleryTableColumnModel` 在源模型重置时还会清空列顺序和隐藏状态，导致数据刷新后配置丢失。

本变更只维护运行期内存状态，不读写文件、不使用 `QSettings`，应用持久化仍由应用负责。

## What Changes

- `CatTableView` 增加 `columnWidthKeyProvider`，宽度内部映射可按稳定源列 key 保存。
- 普通视图 provider 保留显式/隐式宽度兜底，不影响自定义 delegate 尺寸。
- Gallery 用列代理的源列号作为宽度 key。
- `GalleryTableColumnModel` 按列 header key 保留顺序/隐藏状态：
  - 行数据重置时配置不变
  - 列新增/删除时保留仍存在的列
- 增加源列宽度跟随、数据重置保留配置的测试。

## Non-Goals

- 不读写文件、不引入 `QSettings`/JSON/INI。
- 不把列配置状态放进公共 C++ ABI。
- 不实现应用重启后的恢复。
- 不改变排序、冻结、RTL 和编辑行为。
