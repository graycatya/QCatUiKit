# Spec Delta

## ADDED Requirements

### Requirement: R12 TableCustomization

`CatTableView` SHALL 以主题、结构、行为三层提供定制入口：delegate 与表头契约、编辑委托、状态插槽和工具条。未提供定制内容时 SHALL 保持现有行为。

#### Scenario: delegate 契约

- WHEN 应用为 `delegate` 声明 `row`、`column`、`display`、`selected`、`editing`、`theme`
- THEN 单元格 SHALL 获得对应行列数据、选中态和编辑态，并可使用传入主题。
- WHEN 自定义 delegate 缺少上述任一属性
- THEN 缺失属性对应的能力 SHALL 降级，SHALL NOT 输出告警。
- WHEN 后续版本为 `delegate` 增加新的必需属性
- THEN 该变更 SHALL 按破坏性变更处理。

#### Scenario: 基类组件

- WHEN 应用继承 `CatTableCell`
- THEN 实例 SHALL 自带必需属性、主题背景、选中背景和内边距，默认外观 SHALL 与内置 delegate 一致。
- WHEN 应用继承 `CatTableHeaderCell`
- THEN 实例 SHALL 自带 `column`、`display`、`theme` 和表头背景。

#### Scenario: 进入编辑

- WHEN 已提供 `editDelegate` 且模型 `flags` 包含 `Qt::ItemIsEditable`
- THEN 双击单元格、按 F2 或 Enter、或调用 `edit(row, column)` SHALL 进入编辑态。
- WHEN 模型 `flags` 不包含可编辑标记
- THEN 上述入口 SHALL NOT 进入编辑态。

#### Scenario: 提交与取消

- WHEN 编辑中调用 `commitEdit(value)`
- THEN 值 SHALL 以 `EditRole` 写入当前编辑索引并退出编辑。
- WHEN 模型拒绝写入
- THEN `commitEdit` SHALL 返回 `false`，编辑态 SHALL 保持。
- WHEN 编辑中调用 `cancelEdit()` 或按 Esc
- THEN 模型 SHALL NOT 被修改，编辑态 SHALL 退出。

#### Scenario: 空状态

- WHEN 提供 `emptyState`、模型行数为 0 且 `loading=false`
- THEN 空状态 SHALL 显示并覆盖整个内容区；冻结生效时 SHALL 覆盖四区。
- THEN 空状态 SHALL NOT 遮挡表头和 `toolbar`。
- WHEN 未提供 `emptyState`
- THEN 空状态区域 SHALL 不显示任何默认内容。

#### Scenario: 加载状态

- WHEN 提供 `loadingState` 且 `loading=true`
- THEN 加载状态 SHALL 显示并覆盖整个内容区；冻结生效时 SHALL 覆盖四区。
- WHEN `loading=false`
- THEN 加载状态 SHALL 隐藏。
- WHEN 未提供 `loadingState`
- THEN `loading=true` SHALL NOT 产生任何遮挡。

#### Scenario: 工具条

- WHEN 提供 `toolbar`
- THEN 工具条 SHALL 显示在表头之上并跨越整个表格宽度，横向和纵向滚动 SHALL NOT 移动工具条。
- WHEN 未提供 `toolbar`
- THEN 表格 SHALL NOT 为其预留空间。

#### Scenario: 表头定制

- WHEN 提供 `headerDelegate`
- THEN 冻结列表头段和主体表头段 SHALL 都使用该 delegate，并获得 `column`、`display` 和主题。
- THEN 表头点击、排序图标和筛选交互 SHALL 由 delegate 自行处理，库 SHALL NOT 内置排序状态。

#### Scenario: 冻结兼容

- WHEN 冻结生效
- THEN 四区 SHALL 使用同一 `delegate` 和 `editDelegate`，两段表头 SHALL 使用同一 `headerDelegate`。
- THEN `toolbar`、`emptyState` 和 `loadingState` SHALL 跨越全宽。

#### Scenario: 默认行为不变

- WHEN 未设置 `editDelegate`、`emptyState`、`loadingState`、`toolbar`、`headerDelegate` 和 `loading`
- THEN 布局、渲染、滚动、现有 `delegate` 和宽度/高度 provider 行为 SHALL 与变更前一致。
