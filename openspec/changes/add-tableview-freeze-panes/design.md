# Design

## Context

`CatTableView` 当前是单 `TableView` 加一个 `HorizontalHeaderView`：表头纵向固定，主体整体横向和纵向滚动。Qt Quick `TableView` 没有冻结列/行 API，`resizableColumns` 的显式列宽保存在各视图内部，且没有 resize 完成信号。因此冻结不能靠新增一个属性转发，必须组合多个视图并统一宽度状态。需求见 [proposal](proposal.md) 和 [差异规约](specs/ui-components/spec.md)。

## Goals / Non-Goals

**Goals:**

- 四区共享同一模型、选择、编辑和列宽/行高结果。
- 冻结数量变化不重置滚动和选中。
- 无冻结时保持现有布局和 `resizableColumns` 行为。
- 公开 API 只增加两个属性和两个方法，不新增 C++ 导出。

**Non-Goals:**

- 非连续冻结、冻结区内部滚动、表头拖拽抓手、行号列、多级表头、库内建右键菜单。
- 重写模型层排序、过滤或分页。
- 为冻结单独引入新的渲染后端。

## Decisions

### 四区用四个 `TableView` 加零尺寸 provider，而不是列子集代理模型

四个视图绑定同一模型实例，每个视图包一层 `columnWidthProvider` 和 `rowHeightProvider`，把不属于本区的列/行返回 0：

| 区域 | 列 | 行 |
| --- | --- | --- |
| 左上 | `< frozenColumnCount` 有宽 | `< frozenRowCount` 有高 |
| 顶部 | 其余列有宽 | `< frozenRowCount` 有高 |
| 左侧 | `< frozenColumnCount` 有宽 | 其余行有高 |
| 主体 | 其余列有宽 | 其余行有高 |

这样 `row`、`column` 始终是模型原始索引，四区可以直接共享一个 `ItemSelectionModel`，编辑提交和键盘导航不需要索引映射。

- 备选：列子集代理模型。视图索引干净，但需要双向映射 source/proxy，选择、编辑、`positionViewAtCell` 和排序都要额外桥接，任一环节漏掉都会出现选中错位。
- 风险：零宽列在部分 Qt 版本仍可能创建 delegate。实现时对越界列返回空 `Item`，并开启 `reuseItems`，用压力页验证实例数量。

### 冻结模式自管列宽，停用内置列宽拖动

`TableView` 的显式列宽只作用于自身视图，四个视图各拖各的会立刻不一致。冻结时关闭四个视图的 `resizableColumns`，由表头共享的 resize 手柄写入组件内部宽度表，四区 provider 读同一份数据；`resizableColumns` 的公开语义保持不变，只是实现路径换到共享手柄。

- 备选：拖动后轮询 `explicitColumnWidth()` 再分发给其他视图。拖动过程不同步，且无法处理同一列在两区同时调整。
- 行高没有内置拖动，继续由 `rowHeightProvider` 统一计算，四区读取同一结果。

### 滚动同步单向驱动，避免双向绑定

主体区是唯一驱动源：左侧区跟随 `contentY`，顶部区跟随 `contentX`，左上区不滚动。同步用属性赋值加防回环标志，不用双向绑定。可滚范围变化后按新范围钳制，不主动 `positionViewAtCell`。

### 表头分两段，列宽手柄独立

冻结段表头 `syncView` 到左上区（其 `contentX` 恒为 0），主体段表头 `syncView` 到主体区。列宽手柄只在表头单元格右边界 6 px 内生效，`resizableColumns=false` 时禁用；冻结模式下列宽写入共享宽度表，四区一起重排。

### 不提供冻结数量拖拽抓手

冻结数量只由 `frozenColumnCount`/`frozenRowCount` 和 `freezeTo()`/`clearFreeze()` 控制。评估过表头抓手，但发现性差、与列宽手柄抢空间，而且冻结通常是应用配置或持久化状态；应用需要交互时在自己的菜单或工具条调用 API。该决定取代最初的抓手方案。

### 编辑与键盘跨区

四区共用模型和选择，但编辑状态属于各视图：开始编辑时先关闭其他视图的编辑器，提交仍写回同一索引。方向键在区边界处由 `Keys` 处理器转移 `activeFocus` 并设置目标区的 `currentIndex`，再用 `positionViewAtCell` 保证可见。

### RTL 与无障碍

容器启用 `LayoutMirroring`，冻结列换到右侧，滚动条镜像；内部仍以“起始侧列数”解释 `frozenColumnCount`。四区分别设置 `Accessible.name`（顶部、左侧、主体），只有获得焦点的区暴露表格语义，避免同一行被重复朗读。

### 性能取舍

四视图会放大 delegate 实例数量。列表型大数据只虚拟化可见行，冻结开启后可见行数不变，理论上不改变复杂度；验收用现有 10 万行压力页做冻结开启冒烟，不新增硬性 FPS 指标。

## Risks / Trade-offs

- 零宽列仍创建 delegate → 越界 delegate 返回空 Item，`reuseItems`，用压力页核对实例数和滚动流畅度。
- 四视图滚动同步出现回环抖动 → 单向驱动 + 防回环标志，边界钳制只在驱动侧计算。
- 冻结区编辑与主体编辑同时存在 → 进入编辑时关闭其他视图编辑器，测试覆盖跨区提交。
- 键盘焦点跨四视图不连续 → 边界键处理显式转移焦点，测试覆盖四个方向。
- 无障碍重复朗读 → 只让焦点区暴露表格语义；真实读屏验证不在自动测试范围，记录为未覆盖。
- RTL 下 `contentX` 与列顺序可能由 Qt 自行镜像 → 以 Gallery RTL 检查和 QML 测试确认，必要时在 provider 坐标中显式换算。

## Migration Plan

默认 `frozenColumnCount=0`、`frozenRowCount=0` 走现有单视图路径，已有调用方无需修改。公开 API 为新增，不删除或改签名；QML-only 变更，无 ABI、安装或构建选项影响。回退只需还原 `CatTableView.qml` 和新增内部 QML 文件。
