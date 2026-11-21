# CatTableView RTL 设计 Spike

状态：草案，决策已确认。本文件只做设计分析，不包含实现，也不改变冻结变更的验收状态。

相关工件：[proposal](proposal.md)、[差异规约](specs/ui-components/spec.md)、[design](design.md)、[tasks](tasks.md)、[RTL 变更](../add-tableview-rtl/tasks.md)。

## 决策记录（2026-11-20）

| 决策 | 选择 |
| --- | --- |
| 1 | **A**：公开 `ColumnReverseProxyModel`；`CatTableView` 同时暴露 `mapToSource` / `mapFromSource` |
| 2 | **A**：索引映射使用方法 |
| 3 | **B**：RTL 下垂直滚动条位于左侧 |
| 4 | **A**：RTL 是本版本归档门槛，实现拆到 `add-tableview-rtl` |

## 结论

- Qt Quick 的 `TableView` 和 `HorizontalHeaderView` 不读取布局方向，列始终按模型索引从左到右布局。
- 只镜像四个视图的 `x` 坐标，只能把冻结**块**放到右侧，块内仍然是列 0 在左、列 `F-1` 在右，不满足“从起始侧连续冻结”的 RTL 语义。
- 真正的 RTL 需要**列反转代理模型**，并同步处理选择索引、编辑写回、键盘方向、初始滚动位置、表头分段和列宽手柄。
- 建议不把 RTL 塞进 `add-tableview-freeze-panes`，而是在 spike 评审通过后单独建 `add-tableview-rtl` 变更；冻结变更的 3.3 保持阻塞。

## 证据

本机 Qt 6.12 源码检索结果：

| 文件 | `layoutDirection` / `RightToLeft` / `mirror` |
| --- | --- |
| `qquicktableview.cpp` / `qquicktableview_p.h` | 无相关处理 |
| `qquickheaderview.cpp` / `qquickheaderview_p.h` | 无相关处理 |

现有组件的 RTL 做法是 `LayoutMirroring.enabled` 配合 `Row`/`RowLayout` 的 `layoutDirection`，例如 `CatButton`、`CatPagination`。这套机制只影响布局容器，不会改变 `TableView` 的列顺序。

## 目标语义

- `frozenColumnCount` 仍然表示**源模型起始侧**的连续列数，即源列 `0..F-1`；RTL 下这些列位于最右侧。
- 无冻结时，RTL 表格也应从源列 0 开始显示，即源列 0 在最右侧。
- 冻结时四区水平镜像：
  - 右侧：冻结列表头 + 冻结列区（顶部/底部由冻结行数决定）。
  - 左侧：主体表头 + 主体列区。
- 垂直滚动条仍跨四区；水平滚动条只在主体区，并按 RTL 方向镜像。
- 选择、编辑、列宽、行高、键盘导航在四区和主体之间保持一致。
- `frozenColumnCount` / `frozenRowCount` 的越界钳制、`freezeTo` / `clearFreeze` 语义不变。

## 方案对比

| 方案 | 做法 | 结论 |
| --- | --- | --- |
| A. 仅几何镜像 | 冻结块放右侧，表头/滚动条镜像 | 列顺序错误，选择与视觉不对应，拒绝 |
| B. 列反转代理模型 | 代理把源列反序映射，四区使用代理 | 推荐；需要索引映射和新的选择/编辑契约 |
| C. 复制一份反向模型 | 维护独立数据副本 | 数据同步、编辑和性能都不可接受 |
| D. 修改 Qt TableView | 给 Qt 打补丁或子类化 | 超出库的维护边界，拒绝 |

## 推荐架构

### 1. 列反转代理

新增内部或公开的 `QCatColumnReverseProxyModel`（`QAbstractProxyModel` 子类）：

```
proxyColumn = sourceColumnCount - 1 - sourceColumn
sourceColumn = sourceColumnCount - 1 - proxyColumn
```

- 行不变，列反转；`data`、`flags`、`headerData`、`setData` 全部映射到源索引。
- `columnsInserted/Removed/Moved` 和 `modelReset` 需要重新计算代理列范围。
- 第一版可以对列移动/重置使用 `beginResetModel/endResetModel`，代价是选择丢失；后续再细化增量信号。
- 代理本身不复制数据，索引映射 O(1)，不会按行/列复制模型。

### 2. 四区与表头

RTL 下四区的横向角色互换：

```
左侧主体区           右侧冻结列区
┌──────────────┬──────┐
│ 主体列 × 冻结行 │ 冻结列 × 冻结行 │
├──────────────┼──────┤
│ 主体列 × 主体行 │ 冻结列 × 主体行 │
└──────────────┴──────┘
```

- 代理列范围：
  - 主体：`0 .. N-F-1`
  - 冻结：`N-F .. N-1`
- provider 不能再按 `column < frozenColumnCount` 判断冻结侧，必须按代理列范围判断。
- 表头同样分两段：主体段在左、冻结段在右；两段都通过代理读取 `headerData`。
- 冻结段宽度 = 冻结源列的宽度和 + 内部间距；主体从左侧剩余宽度开始。

### 3. 初始滚动与水平滚动条

代理反转后，源列 0 位于代理内容的右端。RTL 表格初始应显示源列 0 附近，因此：

- 无冻结和冻结模式下，主体/单视图初始 `contentX` 应位于最大可滚位置，而不是 0。
- 水平滚动条位置镜像：初始滑块在右端，向左拖动减小 `contentX`。
- 需要在模型重置、列数变化、尺寸变化时决定是否保持右端锚定；第一版建议只在初始化和模型重置时锚定，用户滚动后保持当前位置。
- 垂直滚动条位置与 LTR 相同，仍跨四区；是否移到左侧作为独立决策。

### 4. 选择与索引

- 四区共享的 `ItemSelectionModel` 必须绑定代理模型，因为 `TableView` 要求 `selectionModel.model === view.model`。
- `table.model` 仍暴露源模型；RTL 下 `table.view.model` 是代理，这一点必须写进文档。
- 新增 `mapToSource(index)` / `mapFromSource(index)` 方法（或等价属性），把视图/选择索引转回源索引；无 RTL 时是恒等映射。
- 应用点击、定位、滚动到某行时应继续使用源索引，经 `mapFromSource` 后传给视图。
- `currentIndex`、选中集合在 RTL 下都是代理索引；文档必须明确，避免应用拿源索引直接比较。

### 5. 编辑

- 编辑管线使用代理索引；代理的 `setData` 负责映射到源模型。
- 如果定制变更的 `commitEdit(value)` 以 `selectionModel.currentIndex` 为目标，必须经过代理写回；代理已映射时可直接调用代理 `setData`。
- 失败时保留编辑态；空值/只读判断仍按源模型 `flags`。
- 跨区编辑关闭其他区编辑器的逻辑不变。

### 6. 键盘

- 所有导航以**代理列**为准，视觉方向与代理列递增方向一致。
- RTL 下：
  - `Right` 增加代理列（视觉向右，靠近冻结区）。
  - `Left` 减少代理列（视觉向左，远离冻结区）。
  - 从主体进入冻结区的边界是代理列 `N-F`，用 `Right` 进入；从冻结区返回主体用 `Left`。
- 行方向不受 RTL 影响。
- 需要在 RTL 下验证 Home/End、Tab、Enter 的列方向，避免和 Qt 默认处理冲突。

### 7. 列宽手柄

- 列顺序反转后，表头单元格的视觉边界也随之反转。
- 建议 RTL 下把手柄放在表头单元格的**左边界**，拖动方向取反（向左拖增宽），调整的仍是该表头对应的源列。
- 主体段和冻结段都要使用同一套方向规则。
- `resizableColumns=false` 时禁用，和 LTR 一致。

## 公开 API 与 ABI

- 代理模型如果作为公开类型，需要 `models` 能力的新 Requirement 和 ABI 审查；如果仅内部使用，则不需要公开头文件，但必须给 `CatTableView` 增加索引映射方法。
- 新增方法建议：
  ```qml
  QModelIndex mapToSource(QModelIndex proxyIndex)
  QModelIndex mapFromSource(QModelIndex sourceIndex)
  ```
- 不改变 `frozenColumnCount`、`frozenRowCount`、`freezeTo`、`clearFreeze` 的语义。
- RTL 下 `view.model` 行为变化需要在 `COMPONENTS.md` 和差异规约中明确，避免应用误用。

## 测试矩阵

| 层 | 用例 |
| --- | --- |
| C++ 代理 | 列映射、`data`/`flags`/`headerData`、`setData`、列插入/删除/移动、模型重置 |
| QML 无冻结 RTL | 源列 0 在最右、初始滚动在右端、表头顺序、水平滚动条镜像 |
| QML 冻结 RTL | 冻结块在右、主体在左、四区对齐、行高一致、选择跨区 |
| 索引 | `mapToSource`/`mapFromSource` 往返、`currentIndex` 转换、源索引定位 |
| 编辑 | 代理 `setData` 写回源、提交/取消、跨区关闭编辑器 |
| 键盘 | 左右方向跨冻结边界、Home/End、Tab |
| 列宽 | 表头左边界手柄、主体/冻结列同步、`resizableColumns=false` |
| 模型变化 | 列插入/删除/移动、模型重置后的选择与滚动 |
| Gallery | RTL 语言切换、表格页冻结控件、视觉检查 |
| 性能 | 代理映射 O(1)、10 万行 delegate 数量仍有上界 |

## 风险

- `view.model` 在 RTL 下变成代理，可能破坏依赖源索引的应用代码。
- 列反转代理与 `ItemSelectionModel`、`commitEdit`、`positionViewAtCell` 的索引契约必须同时落地，否则会出现“选中了但写错列”。
- 初始右端锚定、列变化后的锚定策略容易产生滚动跳动。
- 列宽手柄在 RTL 下的左边界映射需要单独的拖拽测试。
- 现有 Gallery 没有 RTL 表格页，需要新增检查入口。

## 待确认决策

1. 代理模型是公开 `models` 类型，还是只作为内部实现？
2. 索引映射用 `mapToSource`/`mapFromSource` 方法，还是暴露 `sourceIndex`/`proxyIndex` 属性？
3. RTL 下垂直滚动条保持右侧，还是也镜像到左侧？
4. RTL 是否仍是冻结变更归档的硬性条件？如果是，3.3 必须等独立变更完成；如果不是，应从 R11 移除并记录为非目标。

## 建议的下一步

1. 评审本 spike，先确认上面 4 个决策。
2. 若接受，创建独立变更 `add-tableview-rtl`：
   - `models` 差异规约：列反转代理与索引映射。
   - `ui-components` 差异规约：RTL 四区、滚动条、键盘、列宽手柄。
3. 实现顺序：代理 + C++ 单测 → `CatTableView` 索引映射 → 四区几何/初始滚动 → 选择/编辑 → 键盘/列宽 → Gallery RTL。
4. 在独立变更落地前，`add-tableview-freeze-panes` 的 3.3 保持未完成；不要用几何镜像冒充 RTL。
