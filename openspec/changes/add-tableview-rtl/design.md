# Design

## Context

Qt Quick 的 `TableView` 与 `HorizontalHeaderView` 不读取布局方向，列始终按模型索引从左到右布局。冻结变更 R11 要求 RTL 下冻结列在起始侧（右侧），仅镜像四区几何无法反转列顺序。实现必须同时解决列顺序、代理索引、初始滚动、选择、编辑、键盘和列宽手柄。需求见 [proposal](proposal.md) 和 [ui-components 差异规约](specs/ui-components/spec.md)。

## Decisions

2026-11-20 确认：

1. **决策 1 = B**：`ColumnReverseProxyModel` 只作为 `CatTableView` 的内部 C++ 实现，不进入公开头、QML 类型或 ABI；`CatTableView` 公开 `mapToSource` / `mapFromSource` 作为表格层入口。应用不能直接实例化代理，避免新增模型层公开契约。
2. **决策 2 = A**：索引映射使用 `mapToSource(QModelIndex)` / `mapFromSource(QModelIndex)` 方法，不新增 `sourceIndex` / `proxyIndex` 属性。
3. **决策 3 = B**：RTL 下垂直滚动条移到左侧，避免覆盖右侧冻结块。
4. **决策 4 = A**：RTL 是本版本归档门槛；本变更先归档，冻结变更随后勾选 3.3 并归档。

## RTL activation

`CatTableView` 读取 `control.LayoutMirroring.enabled`，继承窗口/父项设置。没有新增公开 RTL 开关，避免与 Qt 的布局方向机制冲突。

## Proxy lifecycle

RTL 下：

- `ColumnReverseProxyModel.sourceModel = control.model`
- 四个 `TableView` 和共享 `ItemSelectionModel` 的 `model` 都指向代理
- `control.model` 仍是源模型；`view.model` 在 RTL 下是代理
- 切换布局方向、模型或销毁时释放旧代理，避免悬挂连接

LTR 下不创建代理，`view.model` 仍是源模型，`mapToSource` / `mapFromSource` 为恒等映射。

## Column mapping

```
proxyColumn = sourceColumnCount - 1 - sourceColumn
sourceColumn = sourceColumnCount - 1 - proxyColumn
```

`data`、`flags`、`headerData`、`setData`、`index`、`parent` 全部经 `mapToSource` / `mapFromSource`。列插入/删除发出对应的代理列变化；列移动和模型重置第一版可用 `beginResetModel/endResetModel`，并记录选择丢失风险。

## Geometry

定义：

```
frozenX = rtl ? width - frozenWidth : 0
mainX   = rtl ? 0 : frozenWidth
```

RTL 四区：

```
左侧主体区                 右侧冻结列区
┌──────────────────┬────────────┐
│ 主体列 × 冻结行     │ 冻结列 × 冻结行 │
├──────────────────┼────────────┤
│ 主体列 × 主体行     │ 冻结列 × 主体行 │
└──────────────────┴────────────┘
```

- 冻结行仍在顶部，主体行在下方。
- 冻结列区宽 `frozenWidth`，包含冻结列宽和内部间距。
- 主体区宽 `width - frozenWidth`。
- 表头：冻结段在右、主体段在左；两段分别同步对应视图的 `contentX`。
- provider 判定冻结侧按**代理列**范围：`proxyColumn >= columnCount - frozenColumnCount`。`frozenColumnCount` 仍表示源列 `0..F-1`。
- 列宽表继续按源列索引保存；provider 通过 `mapToSource(proxyColumn)` 取宽度。

## Initial scroll and scrollbars

代理反转后源列 0 位于内容右端：

- 首次布局、模型重置和列数变化时，主体/单视图 `contentX` 定位到最大可滚位置。
- 用户主动滚动后保持当前位置，不强制回右端。
- 水平滚动条绑定主体 `visibleArea.xPosition`，RTL 下镜像滑块方向。
- 垂直滚动条跨四区高度；RTL 下锚定左侧，LTR 下保持右侧。
- 冻结全部时隐藏对应滚动条，与现有行为一致。

## Selection

- `ItemSelectionModel` 绑定代理；四区共享。
- `view.selectionModel.currentIndex` 是代理索引。
- `CatTableView.mapToSource` / `mapFromSource` 转发代理映射；LTR 恒等。
- 应用用源索引定位时先 `mapFromSource`，再调用 `positionViewAtCell` 或设置选择。

## Editing

- Qt 编辑管线使用代理索引。
- `ColumnReverseProxyModel.setData` 映射到源模型；失败返回 false。
- 定制变更的 `commitEdit(value)` 对 `selectionModel.currentIndex` 调用代理 `setData`，成功才关闭编辑器。
- 进入编辑时关闭其他区编辑器、只读判断和 LTR 一致。

## Keyboard

- 所有方向键以代理列工作。
- RTL：`Right` 增加代理列（视觉向右，靠近冻结块），`Left` 减少（远离冻结块）。
- 主体进入冻结的边界是代理列 `N-F`：`Right` 进入，`Left` 返回。
- `Home`/`End` 按视觉首尾映射；`Tab`/`Enter` 不改变方向语义。
- 行方向不受 RTL 影响。

## Column resize

- RTL 下把表头列宽手柄放在单元格视觉左边界，拖动方向取反（向左拖增宽）。
- 手柄仍调整该表头对应的源列；共享宽度表按源列更新，四区同步。
- `resizableColumns=false` 时禁用，与 LTR 一致。

## API and ABI

- `ColumnReverseProxyModel` 作为内部 C++ 类：不进入 `qcat_public_headers`、兼容头或导出符号，不新增公开 ABI；只在 `src/uikit/CMakeLists.txt` 的 SOURCES 登记，并通过模块内部注册供 `CatTableView.qml` 创建。
- `CatTableView` 新增两个 QML 方法，不改变冻结属性和方法；这是本变更唯一的公开 API。
- RTL 下 `view.model` 变为内部代理，必须写入 `COMPONENTS.md` 和 R13；应用不能假设 `view.model === table.model`。
- 内部代理的注册方式要在实现任务 1.2 中验证：优先模块内匿名/私有注册，禁止把代理暴露为可导入的 QML 类型；若 Qt 的注册机制无法满足，再回到评审而不是默默改成公开类型。

## Risks

- 内部代理的 QML 注册机制受 Qt 限制；如果必须借助公开单例工厂，需重新评审公开面，不能把它当作“内部实现”。
- 应用依赖源索引比较 `currentIndex` 会失效；映射方法必须覆盖选择、定位和编辑。
- 列反转代理与列插入/移动/重置的通知容易漏映射；第一版重置会丢失选择，需要文档和测试。
- 初始右端锚定和列宽变化后的滚动位置可能跳动。
- 表头左边界手柄与 RTL 文本方向、列宽热区需要单独验证。
- 现有 Gallery 没有 RTL 表格页，需要新增可重复检查入口。

## Test plan

- C++ 代理单测：映射、数据/角色/表头、`setData`、列插入/删除/移动/重置、空模型、非法索引。
- QML LTR 回归：现有套件保持通过，映射方法恒等。
- QML RTL：无冻结列顺序、四区镜像、表头、初始右端、滚动条镜像、选择/映射、编辑写回、键盘、列宽、模型列变化。
- Gallery RTL：语言切换后表格页冻结控件和视觉检查。
- 性能：10 万行 RTL 下代理 O(1)、delegate 数量仍有上界。

## Rollout

1. 代理 + C++ 单测。
2. `CatTableView` 代理接入和映射方法。
3. 四区/表头几何镜像与初始滚动。
4. 选择、编辑、键盘、列宽。
5. Gallery RTL、文档、完整回归。
6. 本变更归档后，冻结变更勾选 3.3 并归档。
