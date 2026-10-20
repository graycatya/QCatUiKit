# Dock 工作区

[文档索引](README.md) · [维护入口](MAINTENANCE.md)

`CatDockWorkspace` 提供标签组合、四方向分割、分隔条调整、单窗口拖拽、浮动／回停和版本化布局保存／恢复。
已实现 P0–P2 和 P3 的浮动宿主、回停、持久化；跨窗口拖拽和边缘自动隐藏尚未实现。源码独立实现，沿用 Apache-2.0。

## 最小接入

```qml
import QtQuick
import QCatUiKit

CatDockWorkspace {
    id: workspace
    width: 720; height: 480
    CatDockPanel {
        dockId: "editor"; title: qsTr("Editor")
        closable: false
        CatTextArea { anchors.fill: parent; text: "Persistent content" }
    }
    CatDockPanel {
        dockId: "log"; title: qsTr("Log")
        onCloseRequested: workspace.controller.closePanel(dockId)
        CatLabel { text: qsTr("Ready") }
    }
}
```

工作区完成创建后，可调用 `controller.dock("log", "editor", DockController.Bottom)`。
若在 `Component.onCompleted` 中操作，用 `Qt.callLater` 等待面板注册完成。

## 公共契约

- `CatDockPanel.dockId` 是非空且唯一的应用稳定标识，注册期间不能修改。直接作为工作区子面板声明；不要绑定其 parent/x/y/width/height/visible/anchors，几何和可见性由工作区管理。
- `title`、`minimumDockWidth`（默认 120）、`minimumDockHeight`（默认 80）、`closable`、`movable` 和 `floatable`（默认 true） 控制标题、内容区最小尺寸和用户操作。最小尺寸使用非负有限数值。
- 面板由原工作区持有；浮动只切换视觉父项，不重新创建面板或业务内容。工作区销毁时清理浮动窗口，隐藏工作区时一并隐藏窗口。业务计时器是否暂停由应用处理。
- 点击关闭仅发出 `closeRequested()`，由应用接受后调用 `closePanel()`；因此应用可以先询问未保存内容。`closable`／`movable` 和 `layoutLocked` 限制界面操作，不禁止应用通过 controller 修改布局。
- `CatDockWorkspace.controller` 是工作区独占的只读控制器引用。面板创建／销毁自动注册／注销；不要对该控制器手工注册没有对应 QML 面板的 ID。
- 工作区和面板均支持 `theme`。使用局部主题时，显式传入每个面板和应用内容。
- `minimumDockWidth`／`minimumDockHeight` 是整棵布局的最低尺寸。空间不足时保持面板最小尺寸并裁剪，`sizeConstrained` 为 true；宿主可以设置窗口最小尺寸或在外层提供滚动。不会自动隐藏面板。
- 拖拽标签进入目标组边缘区域执行分割，中央区域合并为标签组。释放才提交；Esc、取消或移到工作区外不会修改布局。标签获取焦点后，左右键调整组内顺序；分隔条获取焦点后，方向键调整比例。布局方向为物理左／右，不随 RTL 翻转。

## DockController（C++／QML）

C++ 头：`<QCatUiKit/DockController>`，命名空间 `QCatUiKit`；采用 Pimpl。所有调用必须在 QObject 所属线程。

| 接口 | 行为 |
| --- | --- |
| `registerPanel(id)` / `unregisterPanel(id)` | 注册并显示；注销并从布局移除。重复／未知 ID 返回 false |
| `dock(id, target, position)` | 移入目标组，Center 合并，Left/Right/Top/Bottom 分割；目标必须在主工作区打开且不是自身；源面板可来自浮动窗口 |
| `closePanel(id)` / `openPanel(id)` | 从布局移除；打开到第一个组并激活（首版不记忆关闭前位置） |
| `activatePanel(id)` / `moveTab(id, index)` | 激活打开的面板；组内调整到零起始位置 |
| `setSplitRatio(nodeId, ratio)` | 设置内部节点比例，必须为有限值且 0 < ratio < 1；实际呈现受最小尺寸约束 |
| `saveLayout()` / `restoreLayout(json)` | 输出版本 2 JSON，兼容读取版本 1；恢复失败时保持原布局 |
| `layout` / `panelIds` / `floatingWindows` | 主工作区树／注册 ID 列表／浮动标签组快照 |
| `operationRejected(reason)` | 操作失败的开发诊断；错误原因不用于直接显示给最终用户 |
| `floatPanel(id, wholeGroup = false)` | 将主工作区面板或其整个标签组浮动；已浮动面板返回 false |
| `dockBack(windowId)` | 整个浮动组回停；恢复原标签位置／分割方向和比例，原目标不存在时并入首个主工作区组 |
| `floatingWindowForPanel(id)` | 返回浮动宿主 ID，未浮动返回空字符串 |
| `setFloatingGeometry(windowId, x, y, width, height)` | 更新逻辑像素几何；由窗口宿主自动调用，也可由应用设置 |
| `constrainFloatingWindows()` | 将浮动窗口移回现有屏幕的可用区域；创建／恢复、屏幕移除及可用区域变化时自动处理 |

布局树包含 split 节点（方向、ratio、first、second）和 tabs 节点（tabs、active）；节点 id 由库生成。
应用使用 controller 操作，不直接修改快照。关闭后清理空节点、合并只有一个子节点的分割。

恢复前完成文档和树校验：版本、节点 ID、面板 ID 唯一性、活动标签、比例、深度（64）和节点数（4096）；文档限制 1 MiB 字符。
未知面板 ID 被略过并清理空组；已注册但快照没有的面板保持关闭。面板注册完成后再恢复布局。
版本 2 在 `root` 外加入 `floating` 数组。每项包含 tabs 节点字段、`geometry`（x/y/width/height）和回停提示
`returnTarget`、`returnPosition`、`returnIndex`、`returnRatio`。节点与面板 ID 在主树和全部浮动组之间共同校验唯一性。
窗口坐标必须有限，绝对值小于 1e7；宽高范围为 1–32768 逻辑像素。恢复后根据当前屏幕修正位置和大小。
版本 1 被当作没有浮动窗口的布局。读取旧版本不会保留当前浮动窗口；未来未知版本会被拒绝。
布局不存业务数据；保存到文件／Settings 由应用负责。当前记录窗口几何，不记录最大化／最小化状态。

## 浮动与回停

标签右键菜单或键盘 Menu 键提供“浮动面板”“浮动标签组”。浮动窗口标题栏下提供“停靠回去”。
应用也可调用 `workspace.floatPanel("editor")` 或 `workspace.floatPanel("editor", true)`；该入口尊重
`layoutLocked`、`movable` 和 `floatable`，并以面板当前位置作为初始窗口位置。整个组中任一面板禁止移动／浮动时拒绝操作。
直接调用 controller 属于应用级操作，不受这些界面策略限制。

回停优先使用记录的相邻面板和原标签索引／分割比例。应用已修改目标布局时，使用当前目标组；目标消失时并入主工作区首组，主工作区为空则建立根组。不会恢复整份历史布局而覆盖其他面板的后续修改。

点击浮动窗口系统关闭按钮，不立即销毁窗口：

- 组内有 `closable: false` 时不执行关闭。
- 否则依次发出各面板的 `closeRequested()`；应用调用 `closePanel()` 接受。未接受的面板继续保留在窗口中。
- 所有面板关闭或回停后才销毁空宿主；关闭面板保留内容，销毁整个工作区则释放全部内容。

浮动宿主为原生窗口、与主窗口使用同一 QML 引擎。已有焦点子项在迁移后恢复焦点，面板内弹出层可在新窗口重新打开；
应用自行持有的全局弹出层、原生子窗口和外部渲染上下文不在该保证内。局部主题仍需显式传入面板内容。
跨窗口拖拽尚未实现；本阶段用回停按钮或 `dockBack()` 返回。

## 实现与验证入口

- 布局／持久化：`src/uikit/qcatdockcontroller.h/.cpp`。
- 呈现与交互：`qml/CatDockWorkspace.qml`、`qml/CatDockPanel.qml`；几何计算 `qml/DockGeometry.js` 和 `qml/DockFloatingWindow.qml` 为内部实现。
- 演示：`examples/uikit/gallery/GalleryDockPreview.qml`，Gallery 页面 ID `dock-workspace`。
- Gallery 窄屏／关闭／恢复回归：`tests/auto/gallery/data/tst_dock.qml`；示例在最小尺寸不足时提供横向滚动。
- C++ 行为：`tests/auto/dock/tst_dockcontroller.cpp`；QML 生命周期／交互：`tests/auto/qml/data/tst_dock.qml`。
- 浮动验证：`tests/auto/qml/data/tst_dockfloating.qml` 检查内容／焦点／弹层、关闭拒绝、图表像素、隐藏／销毁和屏幕外恢复；C++ 测试覆盖组回停、缺失目标、v1 兼容及无效 v2 数据。
- 下一阶段 P3：跨窗口拖拽、停靠目标识别、预览／取消；Windows／Wayland、多显示器实际热插拔分别验证。
