# Design

## Context

`CatTableView` 是单 `TableView` 加 `HorizontalHeaderView` 的薄封装，当前只暴露 `delegate`、`columnWidthProvider`、`rowHeightProvider` 和 `view`。这里的 `TableView` 是 Qt Quick 的 QML 类型（`import QtQuick`，C++ 实现为 `QQuickTableView`），不是 Qt Widgets 的 `QTableView`；库的依赖只有 Core/Gui/Qml/Quick/QuickControls2/ShaderTools，没有 Widgets。

该类型自 Qt 6.5 起自带编辑管线：`edit(index)`、`closeEditor()`、`editTriggers`、附加属性 `TableView.editDelegate`，以及提供给 delegate 的必需属性 `editing`；按 Enter/Tab 时发出附加对象 `commit` 信号，Esc 只关闭不提交。项目最低 Qt 6.8，满足这些 API 的版本要求。需求见 [proposal](proposal.md) 和 [差异规约](specs/ui-components/spec.md)。

## Goals / Non-Goals

**Goals:**

- 用 Qt 原生编辑管线实现 `editDelegate`，不自建编辑器生命周期。
- 用公开基类承载 delegate 契约，默认 delegate 与用户 delegate 走同一套属性。
- 状态插槽和 toolbar 在冻结四区下保持全宽、不遮挡表头。
- 未设置新属性时行为与现状完全一致。

**Non-Goals:**

- 内置排序、筛选、分页或右键菜单。
- 行级 `rowDelegate`、多级表头、服务端模型。
- 为 delegate 增加隐式主题遍历。

## Decisions

### 复用 Qt 原生编辑，而不是自建编辑状态机

`edit(row, column)` 先设置 `selectionModel` 的当前索引，再调用 `view.edit(model.index(row, column))`；默认 `editTriggers` 设为 `DoubleTapped | EditKeyPressed`。Qt 会检查 `Qt::ItemIsEditable`、创建 edit item、把 `editing` 必需属性设为 true，并在 Enter/Tab 时发出 `commit` 信号，Esc 时仅调用 `closeEditor()`。

`commitEdit(value)` 以 `selectionModel.currentIndex()` 为准，调用 `model.setData(index, value, EditRole)`；成功则对所有区调用 `closeEditor()` 并返回 `true`，失败保留编辑态并返回 `false`。这条不变式由库的 `edit()`、双击和键盘入口共同保证：Qt 的原生触发都先更新当前索引；应用直接调用 `view.edit()` 属于逃逸口行为，不在契约内。

- 备选：自建 `editingIndex` 和 `Loader` 编辑器。需要自己处理焦点、滚动、模型重置和四区同步，等于重写 Qt 已有管线。

### `editDelegate` 由默认 delegate 转发，而不是库注入单元格

Qt 的编辑委托是每个 cell delegate 上的附加属性，不是视图级属性。`CatTableView` 暴露顶层 `editDelegate: Component`，默认 cell delegate 设置 `TableView.editDelegate: control.editDelegate`。用户替换 `delegate` 时需要在自定义 delegate 中写同一行；契约文档明确这一点。

- 备选：库在内部包装 cell delegate 并注入编辑委托。会改变 delegate 的上下文和必需属性来源，且用户 delegate 仍是外层，得不偿失。

### 用公开基类表达契约

`CatTableCell` 和 `CatTableHeaderCell` 是 `Item` 派生的公开 QML 类型，声明 `required property` 和主题、背景、内边距：

```qml
CatTableCell {
    required property int row
    required property int column
    required property var display
    required property bool selected
    required property bool editing
    property ThemePalette theme: Theme
}
```

`row`、`column`、`selected`、`editing` 由 `TableView` 直接提供，`theme` 由默认 delegate 绑定。内置默认 delegate 用 `CatTableCell` 实现，用户继承后天然满足契约；不继承的自定义 delegate 只要声明同名属性也能工作，缺属性只降级、不告警。

- 备选：只写文档不提供基类。约定容易漂移，升级时用户无法自查。

### 状态插槽用 `Loader` 覆盖内容区，不遮挡表头和 toolbar

`emptyState`/`loadingState` 各自放在内容区上方的 `Loader`，`active` 由库计算：空态为 `model.rowCount===0 && !loading`，加载态为 `loading`。未提供 Component 时 `active=false`。插槽根项仍可绑定 `visible=false` 进一步隐藏。冻结生效时覆盖层跨越四区，`clip` 到内容区，不含表头和 toolbar。

### toolbar 固定在全宽顶部

`toolbar` 实例化在表头之上的独立容器，高度取内容隐式高度，不参与任何滚动；未提供时容器高度为 0，不预留空间。

### 表头委托同时用于两段表头

`headerDelegate` 同时赋给冻结段和主体段的 `HorizontalHeaderView`。契约提供 `column`、`display`、`theme`（宽度取 item 自身 `width`）。库不监听点击、不维护排序状态；需要排序图标时由 delegate 自己画，并由应用把排序结果写回模型或代理模型。

### 与冻结变更的兼容

冻结把表格拆成四个 `TableView`，本变更的 `delegate`、`editDelegate`、`headerDelegate` 需在四个视图和两段表头分别赋值；编辑索引共享 `selectionModel`，提交或取消时对四个视图都调用 `closeEditor()`，避免同一单元格出现两个编辑器。状态覆盖层和 toolbar 在四区外层，天然全宽。

### 无障碍与默认行为

基类保留文本内容和 `Accessible` 角色设置入口；状态覆盖层提供可访问名称，不拦截键盘。未设置新属性时不创建 toolbar/覆盖层，不改变现有 delegate 和滚动行为。

## Risks / Trade-offs

- 自定义 delegate 未转发 `TableView.editDelegate` → `edit()` 无效。契约文档和基类示例给出正确写法，测试覆盖默认路径。
- `commitEdit` 依赖当前索引等于编辑索引 → 库的三个入口都先同步当前索引；直接绕过库使用 `view.edit()` 时可能写错单元格，文档标注为逃逸口限制。
- 状态覆盖层拦截指针 → 覆盖层根项不安装输入处理器，只有用户内容自己声明的控件可交互；空/加载态默认不阻挡滚动，滚动仍由下层视图处理。
- 四区各自维护编辑 item → 提交/取消时统一关闭，测试覆盖跨区编辑。
- 新增两个公开 QML 类型 → 需要加入组件目录、导入测试和中文/英文文档，避免只存在于源码。

## Migration Plan

全部为新增属性和类型，默认值保持现有行为；现有 `delegate` 继续可用，缺新属性只降级。QML-only 变更，无 C++ ABI、安装规则或构建选项影响。回退只需还原 `CatTableView.qml` 并删除两个基类文件及其登记。
