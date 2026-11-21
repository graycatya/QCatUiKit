# Proposal

## Why

`CatTableView` 目前只暴露 `delegate`、列宽/行高 provider 和 `view` 逃逸口：没有编辑态组件、空/加载状态、工具条和表头定制，应用只能在外层重复拼装，或整表复制后自行维护。表格的定制能力需要按主题、结构、行为分层，而不是继续堆叠布尔属性。

## What Changes

- 公开 `delegate` 契约：单元格 delegate 使用 `row`、`column`、`display`、`selected`、`editing`、`theme`；缺属性只降级对应能力，不告警。以后新增必需属性视为破坏性变更。
- 新增公开基类 `CatTableCell` 与 `CatTableHeaderCell`；继承后自动获得上述契约、主题背景和选中/编辑态呈现。
- 新增 `editDelegate`，转发到 Qt 6 原生 `TableView.editDelegate`；新增 `edit(row, column)`、`commitEdit(value)`、`cancelEdit()`。默认 `DoubleTapped | EditKeyPressed` 触发，是否可编辑按模型 `Qt::ItemIsEditable` 判断。
- 新增 `emptyState`、`loadingState` 两个 Component 插槽和 `loading` 属性；行数为 0 且非加载时显示空态，`loading=true` 时显示加载态。未提供插槽时不显示任何默认内容；覆盖内容区（含冻结四区，不含表头和 toolbar）。
- 新增 `toolbar` Component 插槽，位于表头之上、跨全宽、不随表格滚动。
- 新增 `headerDelegate`，在冻结列表头段和主体表头段都生效；只负责外观，库不内置点击或排序状态。
- 默认 `0/0` 冻结和未设置任何新属性时，现有布局、滚动和 `delegate` 行为保持不变。

## Capabilities

### New Capabilities

### Modified Capabilities

- `ui-components`: 新增 `CatTableView` 分层定制契约（R12）。

## Impact

涉及 `src/uikit/qml/CatTableView.qml`、新增公开 QML 类型 `CatTableCell.qml` 与 `CatTableHeaderCell.qml`、`src/uikit/CMakeLists.txt` 的 QML 模块登记、Gallery 数据页与中英文 TS、QML 行为测试，以及 `COMPONENTS.md`、`VALIDATION.md`。公开接口新增 6 个属性、3 个方法和 2 个 QML 类型；不改变 C++ 头、导出符号或 ABI。

依赖顺序：本变更的插槽和 delegate 契约需在 `add-tableview-freeze-panes` 的四区结构上验收；冻结未合并时先按单视图实现，冻结合并后补四区回归。

非目标：内置排序/筛选算法、库内建右键菜单、`rowDelegate`、表单容器、多级表头。
