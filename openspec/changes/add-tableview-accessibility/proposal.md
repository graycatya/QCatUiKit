# Proposal: 表格可访问性

## Why

`CatTableView` 已有冻结、RTL、编辑、定制和排序示例，但表格语义只停留在视觉层：屏幕阅读器无法识别表头、单元格、选中状态和排序方向，键盘也缺少跨页导航。表格是本库最主要的复杂控件，缺少可访问性会直接影响键盘用户和辅助技术用户。

## What Changes

- `CatTableView` 暴露 Table 角色、可配置名称和行列摘要。
- `CatTableCell` 暴露 Cell 角色、显示文本和选中状态。
- `CatTableHeaderCell` 暴露 ColumnHeader 角色和名称，并允许派生表头提供排序描述。
- 普通、冻结和 RTL 视图统一支持 `PageUp` / `PageDown` 与 `Ctrl+Home` / `Ctrl+End`。
- Gallery 排序表头把当前排序方向写入 `Accessible.description`。
- 增加键盘、Accessible 属性和焦点回归测试。

## Non-Goals

- 不实现自定义屏幕阅读器适配层，使用 QML `Accessible` 附加属性。
- 不改变排序、选择、编辑或冻结的现有行为。
- 不引入新的公开 C++ ABI。
