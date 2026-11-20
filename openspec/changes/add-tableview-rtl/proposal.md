# Proposal

## Why

`add-tableview-freeze-panes` 的 R11 要求冻结列在 RTL 下位于起始侧（右侧）。但 Qt Quick 的 `TableView` 和 `HorizontalHeaderView` 不读取 `layoutDirection`，列始终按模型索引从左到右布局；只镜像四区几何只能把冻结块放到右侧，块内仍是列 0 在左，无法满足“起始侧连续冻结”的语义。

真正的 RTL 需要列反转代理模型，并同步处理选择索引、编辑写回、初始滚动位置、键盘方向、表头分段和列宽手柄。该工作属于模型层和表格布局的新能力，不应塞进冻结变更。

## What Changes

- 新增内部 `ColumnReverseProxyModel`（QAbstractProxyModel）：行不变、列反转，映射 `data`、`flags`、`headerData`、`setData`、`index` 和索引转换；列插入/删除/移动/重置后重新映射代理列。该类型只作为 `CatTableView` 的内部实现，不进入公开 C++ 头或 ABI。
- `CatTableView` 在 RTL（`LayoutMirroring.enabled`）下自动创建代理并让四区共享它；`table.model` 仍是源模型，`table.view.model` 在 RTL 下是代理。
- 新增 `mapToSource(index)` / `mapFromSource(index)`，无 RTL 时为恒等映射；无冻结点也支持。
- 四区、表头、水平/垂直滚动条在 RTL 下水平镜像：主体在左、冻结块在右，垂直滚动条移左侧。
- RTL 初始滚动定位到内容右端（源列 0 所在侧），水平滚动条滑块初始在右端并镜像拖动方向。
- 选择、编辑、键盘导航和列宽手柄按代理列工作：`Right` 进入冻结区、`Left` 返回主体；列宽手柄位于表头单元格左边界。
- Gallery 增加 RTL 表格检查入口；新增代理 C++ 测试和 QML RTL 测试。
- LTR 和冻结 `0/0` 的现有行为保持不变。

## Capabilities

### Modified Capabilities

- `ui-components`: 新增 R13 `TableRtlColumns`，实现 R11 的 RTL 场景。

## Impact

涉及内部 C++ 代理源文件、`src/uikit/CMakeLists.txt` 的 SOURCES 登记、`CatTableView.qml`、QML 行为测试、Gallery RTL 入口和中英文文案，以及 `COMPONENTS.md`、`VALIDATION.md`。不新增公开 C++ 头、导出符号或 ABI；只新增 `CatTableView` 的两个 QML 方法。RTL 下 `view.model` 变为内部代理，需要 API 文档明确；不改变 `frozenColumnCount`、`frozenRowCount`、`freezeTo()`、`clearFreeze()` 的语义。

非目标：任意列顺序、每列独立 RTL 开关、其他表格组件（如未来的表单）的 RTL、代理模型的排序/过滤组合。

依赖 `add-tableview-freeze-panes` 的 R11。建议归档顺序：本变更先归档（R13 进入基线），再归档冻结变更并勾选其 3.3。
