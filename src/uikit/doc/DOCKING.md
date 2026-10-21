# Dock 工作区

[文档索引](README.md) · [维护入口](MAINTENANCE.md)

`CatDockWorkspace` 提供标签组合、四方向分割、分隔条调整、单窗口拖拽、浮动／回停和版本化布局保存／恢复。
已实现 P0–P2 和 P3 的浮动宿主、回停、持久化及同工作区跨窗口标签拖动。浮动宿主保持单个标签组，尚不支持内部四方向分割；边缘自动隐藏尚未实现。源码独立实现，沿用 Apache-2.0。

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

工作区采用独立标题栏与扁平标签栏，标题栏提供浮动／关闭操作，标签下划线表示当前活动面板。
两行总高度为主题 `controlHeight × 2`，计入最低停靠高度。标题文字和标签都可拖动当前面板。
聚焦内容时显示面板边框反馈；分隔条支持悬停高亮、拖动和键盘调整。
分隔条的鼠标高亮只在悬停／按压期间显示，点击后移出恢复普通颜色；键盘聚焦或方向键调整时保留焦点提示。
拖动进入主工作区目标组时显示五方向指示器，命中方向格或组边缘选择分割，中央合并标签；
半透明区域预览提交后的占用范围。浮动目标只显示中央合并指示器，避免暗示尚不支持的内部拆分。
工作区四周另有外边缘指示器：拖到最外侧边缘会相对于整棵主布局停靠，左／右形成贯穿全高的列，上／下形成贯穿全宽的行。
组内十字指示器仍只拆分当前标签组。例如属性面板要位于编辑器与日志整体的最右侧，应使用工作区右侧外边缘，而非编辑器组内的右方向格。
拖动标签或面板标题时显示独立的随动缩略预览，离开主工作区、经过窗口间空白区域后仍可见；源标签淡化，目标浮动窗口标签栏高亮。
预览不接受鼠标输入、不主动申请焦点，面板内容只在释放时迁移。可见面板在开始拖动时捕获一次等比缩略图（最大 480×320）；未显示的面板使用标题卡片，避免临时改变活动标签。
Esc、释放或工作区销毁会隐藏／清理预览并忽略迟到的捕获结果。系统原生窗口最外层标题栏仍用于移动原生窗口；跨窗口合并应拖动应用内标签或面板标题。

- `CatDockPanel.dockId` 是非空且唯一的应用稳定标识，注册期间不能修改。直接作为工作区子面板声明；不要绑定其 parent/x/y/width/height/visible/anchors，几何和可见性由工作区管理。
- `title`、`minimumDockWidth`（默认 120）、`minimumDockHeight`（默认 80）、`closable`、`movable` 和 `floatable`（默认 true） 控制标题、内容区最小尺寸和用户操作。最小尺寸使用非负有限数值。
- 面板由原工作区持有；浮动只切换视觉父项，不重新创建面板或业务内容。工作区销毁时清理浮动窗口，隐藏工作区时一并隐藏窗口。业务计时器是否暂停由应用处理。
- 点击关闭仅发出 `closeRequested()`，由应用接受后调用 `closePanel()`；因此应用可以先询问未保存内容。`closable`／`movable` 和 `layoutLocked` 限制界面操作，不禁止应用通过 controller 修改布局。
- `CatDockWorkspace.controller` 是工作区独占的只读控制器引用。面板创建／销毁自动注册／注销；不要对该控制器手工注册没有对应 QML 面板的 ID。
- 工作区和面板均支持 `theme`。使用局部主题时，显式传入每个面板和应用内容。
- `minimumDockWidth`／`minimumDockHeight` 是整棵布局的最低尺寸。空间不足时保持面板最小尺寸并裁剪，`sizeConstrained` 为 true；宿主可以设置窗口最小尺寸或在外层提供滚动。不会自动隐藏面板。
- 拖拽标签进入目标组边缘区域执行分割，中央区域合并为标签组。主工作区标签拖到工作区外释放时浮动单个面板，窗口初始位置跟随释放点并受屏幕可用区域约束；尊重 `floatable`、`movable` 和 `layoutLocked`。释放才提交；Esc 或鼠标抓取取消不会修改布局。浮动标签在工作区外释放仍保持原布局。标签获取焦点后，左右键调整组内顺序；分隔条获取焦点后，方向键调整比例。布局方向为物理左／右，不随 RTL 翻转。

## DockController（C++／QML）

C++ 头：`<QCatUiKit/DockController>`，命名空间 `QCatUiKit`；采用 Pimpl。所有调用必须在 QObject 所属线程。

| 接口 | 行为 |
| --- | --- |
| `registerPanel(id)` / `unregisterPanel(id)` | 注册并显示；注销并从布局移除。重复／未知 ID 返回 false |
| `dock(id, target, position)` | 移入目标组，Center 合并，Left/Right/Top/Bottom 分割；源与目标均可来自浮动窗口，但浮动目标仅接受 Center；目标必须打开且不是自身。主工作区为空时，空 target 配合 Center 将单个打开面板移回主工作区 |
| `dockToWorkspace(id, position, ratio = 0.5)` | 相对于整棵主布局停靠，position 仅接受 Left/Right/Top/Bottom；ratio 为新分割第一子节点比例，有限且 0 < ratio < 1。源 ID 必须注册；空工作区建立根组，关闭的面板可重新打开；唯一主面板原地停靠不改变布局。交互入口参考源面板尺寸选择比例，呈现仍受最低尺寸约束 |
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
可选字符串 `returnNode` 记录原相邻子树的节点 ID，使分割组旁的面板可以恢复为完整侧列；旧版 v2 没有此字段时按面板提示回退，字段不是字符串则原子拒绝。
窗口坐标必须有限，绝对值小于 1e7；宽高范围为 1–32768 逻辑像素。恢复后根据当前屏幕修正位置和大小。
版本 1 被当作没有浮动窗口的布局。读取旧版本不会保留当前浮动窗口；未来未知版本会被拒绝。
布局不存业务数据；保存到文件／Settings 由应用负责。当前记录窗口几何，不记录最大化／最小化状态。

## 浮动与回停

标签右键菜单或键盘 Menu 键提供“浮动面板”“浮动标签组”。浮动窗口标题栏下提供“停靠回去”。
应用也可调用 `workspace.floatPanel("editor")` 或 `workspace.floatPanel("editor", true)`；该入口尊重
`layoutLocked`、`movable` 和 `floatable`，并以面板当前位置作为初始窗口位置。整个组中任一面板禁止移动／浮动时拒绝操作。
直接调用 controller 属于应用级操作，不受这些界面策略限制。

回停优先使用记录的相邻子树（分割停靠）或面板（标签合并），保留原标签索引／分割比例。相邻子树消失时回退到相邻面板；目标均消失时并入主工作区首组，主工作区为空则建立根组。不会恢复整份历史布局而覆盖其他面板的后续修改。

点击浮动窗口系统关闭按钮，不立即销毁窗口：

- 组内有 `closable: false` 时不执行关闭。
- 否则依次发出各面板的 `closeRequested()`；应用调用 `closePanel()` 接受。未接受的面板继续保留在窗口中。
- 所有面板关闭或回停后才销毁空宿主；关闭面板保留内容，销毁整个工作区则释放全部内容。

浮动宿主为原生窗口、与主窗口使用同一 QML 引擎。已有焦点子项在迁移后恢复焦点，面板内弹出层可在新窗口重新打开；
应用自行持有的全局弹出层、原生子窗口和外部渲染上下文不在该保证内。局部主题仍需显式传入面板内容。
浮动窗口的标签可拖回原主工作区：中央合并标签，四边分割，预览在主工作区显示，释放时提交。
Esc、鼠标抓取取消、浮动标签在所有目标区域外释放均保留原布局。主工作区为空时，拖入其区域可将单个标签移回。
拖动单个标签不会移动同组其他标签；`layoutLocked`／`movable` 限制此操作。
主工作区标签可拖入已有浮动宿主，浮动标签也可拖入其他浮动宿主，均作为标签合并；目标窗口显示整窗预览。
进入浮动宿主还要求源面板 `floatable: true`；原宿主为空后销毁。目标窗口保留几何与回停提示，新增标签随目标组回停；只在释放时提交。
重叠的浮动窗口按最近激活顺序命中，覆盖主工作区的浮动窗口优先。源宿主自身不接受此拖动。
当前不支持浮动宿主内部四方向分割，以及跨独立工作区拖动。
也可通过回停按钮或 `dockBack()` 返回。

## 实现与验证入口

- 布局／持久化：`src/uikit/qcatdockcontroller.h/.cpp`。
- 呈现与交互：`qml/CatDockWorkspace.qml`、`qml/CatDockPanel.qml`；内部 `DockTabButton.qml`、`DockDragArea.qml`、`DockDropOverlay.qml` 统一标题／标签、拖动和指示器，`DockDragPreview.qml` 提供穿透输入的随动缩略窗，`DockGeometry.js` 计算几何及方向命中，`DockFloatingWindow.qml` 管理原生宿主。
- 演示：`examples/uikit/gallery/GalleryDockPreview.qml`，Gallery 页面 ID `dock-workspace`。
- Gallery 窄屏／关闭／恢复回归：`tests/auto/gallery/data/tst_dock.qml`；示例在最小尺寸不足时提供横向滚动。
- C++ 行为：`tests/auto/dock/tst_dockcontroller.cpp`；QML 生命周期／交互：`tests/auto/qml/data/tst_dock.qml`。
- 浮动验证：`tests/auto/qml/data/tst_dockfloating.qml` 检查内容／焦点／弹层、关闭拒绝、图表像素、隐藏／销毁和屏幕外恢复；C++ 测试覆盖组回停、缺失目标、v1 兼容及无效 v2 数据。
- 浮动标签拖回测试：`tst_dockfloating.qml` 覆盖全局坐标命中、五方向提交、取消、宿主遮挡和鼠标／Esc 路径。
- P3 跨窗口合并：控制器测试检查原子拒绝、宿主清理、保存恢复和回停；QML 测试检查两种来源的鼠标／Esc、预览、权限及空主工作区；Gallery 测试覆盖完整 Main 页面嵌套滚动。
- 后续：浮动宿主内部四方向分割、边缘自动隐藏；Wayland、多显示器不同 DPI 与实际热插拔分别验证。
