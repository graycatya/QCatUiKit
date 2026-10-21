# Dock 工作区

[文档索引](README.md) · [维护入口](MAINTENANCE.md)

`CatDockWorkspace` 用来搭建类似 IDE 的工作区：面板可以并排显示、合并成标签，也可以拖出成为独立窗口，再拖回主窗口。移动和关闭面板不会重新创建内容，输入框中的文字等状态会保留。

主工作区支持四方向分割和整列／整行停靠；同一工作区的主窗口与浮动窗口之间可以拖动标签。浮动窗口目前只能容纳一个标签组，不能在内部继续分割。其他限制见[当前限制](#当前限制)。

## 最小接入

```qml
import QtQuick
import QCatUiKit

CatDockWorkspace {
    id: workspace
    width: 720; height: 480
    Component.onCompleted: Qt.callLater(function() {
        workspace.controller.dock("log", "editor", DockController.Bottom)
    })
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

这个示例把日志放在编辑器下方。`Qt.callLater` 用来等待两个面板完成注册；恢复已保存的布局时也要等待注册完成。

## 鼠标和键盘怎么操作

拖动面板的标签或应用内标题文字，鼠标旁会出现缩略预览。到达目标后，半透明区域表示松开鼠标后的位置；按 Esc 可以取消。

| 想做什么 | 拖到哪里 |
| --- | --- |
| 合并成标签 | 目标面板中央 |
| 在某个面板旁分割 | 目标组的四方向格或组边缘 |
| 放到整个工作区最右侧，形成完整侧列 | 工作区最右边的外边缘指示器；左侧同理 |
| 放到整个工作区最上方／最下方，形成完整横行 | 工作区顶部／底部的外边缘指示器 |
| 拖出成为独立窗口 | 将主工作区标签拖到工作区外并松开 |
| 放入已有浮动窗口 | 拖到该窗口的面板区域，合并成标签 |

例如编辑器在上、日志在下时，拖到**编辑器组内的右方向格**，只会在编辑器旁分割；要让属性面板占据编辑器和日志整体右侧，请拖到**工作区右侧外边缘**。

系统原生窗口最外层标题栏用于移动窗口。要重新停靠，拖动窗口内部的面板标题或标签。浮动标签在所有目标区域外松开，会保持原布局。

标签右键菜单或键盘 Menu 键可浮动单个面板／整个标签组；浮动窗口内的回停按钮可让整组返回原位置。标题栏还提供浮动和关闭按钮。

标签获取焦点后，左右键可调整组内顺序；分隔条获取焦点后，方向键可调整尺寸。分隔条在鼠标移出后恢复普通颜色，键盘操作时保留焦点提示。左／右始终指屏幕上的物理方向，不随 RTL 翻转。

## 工作区和面板接口

| 工作区接口 | 用途 |
| --- | --- |
| `panels`（默认属性） | 直接在工作区内声明 `CatDockPanel` |
| `controller`（只读） | 查询布局和执行停靠、关闭、保存等操作 |
| `theme` | 设置工作区主题；局部主题还需显式传入面板及其内容 |
| `layoutLocked`（默认 false） | 禁止用户拖动、浮动和调整布局；应用仍可调用控制器 |
| `dragging`（只读） | 当前是否正在拖动面板 |
| `minimumDockWidth` / `minimumDockHeight`（只读） | 当前主布局需要的最低尺寸 |
| `sizeConstrained`（只读） | 工作区是否小于上述最低尺寸 |
| `floatPanel(id, wholeGroup = false)` | 按界面权限浮动面板，并以面板当前位置设置初始窗口几何；返回是否成功 |

`CatDockPanel` 的接口如下：

| 面板接口 | 用途 |
| --- | --- |
| `dockId`（必填） | 非空、唯一的稳定 ID；注册期间不能修改 |
| `title` | 显示名称，默认等于 `dockId` |
| `content`（默认属性） | 面板内的业务内容 |
| `theme` | 面板主题，默认使用全局 `Theme` |
| `minimumDockWidth = 120` / `minimumDockHeight = 80` | 内容区域最小尺寸，使用非负有限数值 |
| `closable = true` | 是否允许用户请求关闭 |
| `movable = true` / `floatable = true` | 是否允许用户移动／浮动；整组浮动要求组内所有面板均允许 |
| `closeRequested()` | 用户请求关闭；应用调用 `controller.closePanel(dockId)` 后才真正关闭 |

### 尺寸、内容和关闭行为

- `CatDockPanel.dockId` 是非空且唯一的应用稳定标识，注册期间不能修改。直接作为工作区子面板声明；不要绑定其 parent/x/y/width/height/visible/anchors，几何和可见性由工作区管理。
- 每组标题栏和标签栏合计占主题 `controlHeight × 2`，额外计入布局最低高度。
- 面板由原工作区持有；浮动只切换视觉父项，不重新创建面板或业务内容。工作区销毁时清理浮动窗口，隐藏工作区时一并隐藏窗口。业务计时器是否暂停由应用处理。
- 点击关闭仅发出 `closeRequested()`，由应用接受后调用 `closePanel()`；因此应用可以先询问未保存内容。`closable`／`movable` 和 `layoutLocked` 限制界面操作，不禁止应用通过 controller 修改布局。
- `CatDockWorkspace.controller` 是工作区独占的只读控制器引用。面板创建／销毁自动注册／注销；不要对该控制器手工注册没有对应 QML 面板的 ID。
- `minimumDockWidth`／`minimumDockHeight` 是整棵布局的最低尺寸。空间不足时保持面板最小尺寸并裁剪，`sizeConstrained` 为 true；宿主可以设置窗口最小尺寸或在外层提供滚动。不会自动隐藏面板。
- 只有松开鼠标才会改变布局；Esc 或鼠标抓取取消不会修改布局。拖动预览不会迁移业务内容，也不接收鼠标输入或主动申请焦点。未显示的标签使用标题卡片，避免为了截图切换活动面板。

工作区中的 `registry`、`geometry`、`floatingHosts` 以及拖动辅助函数用于内部实现，应用不要依赖它们或控件子对象树。`DockDragPreview` 等不带 Cat 前缀的 Dock QML 类型也不是公共组件。

## DockController（C++／QML）

C++ 头：`<QCatUiKit/DockController>`，命名空间 `QCatUiKit`；采用 Pimpl。所有调用必须在 QObject 所属线程。

下表中返回 `bool` 的方法以 true 表示请求成功（包括无需改动的成功请求），false 表示被拒绝。`operationRejected(reason)` 提供开发诊断；应用应自行提供适合用户的提示。直接操作控制器不会检查面板的 `movable`、`floatable`、`closable` 或工作区的 `layoutLocked`。

| 接口 | 行为 |
| --- | --- |
| `registerPanel(id)` / `unregisterPanel(id)` | 注册并显示；注销并从布局移除。重复／未知 ID 返回 false |
| `dock(id, target, position = Center)` | 移入目标组，Center 合并，Left/Right/Top/Bottom 分割；源必须注册（可以已关闭），目标必须打开且不是自身。浮动目标仅接受 Center。主工作区为空时，空 target 配合 Center 将单个打开面板移回主工作区 |
| `dockToWorkspace(id, position, ratio = 0.5)` | 把已注册的面板放到整个主布局外侧，只接受 Left/Right/Top/Bottom；空工作区建立根组，关闭的面板可重新打开。比例用法见下方示例 |
| `closePanel(id)` / `openPanel(id)` | 从布局移除；重新打开到主工作区第一个组并激活，不记忆关闭前位置。对已打开面板调用 openPanel 只激活它 |
| `activatePanel(id)` / `moveTab(id, index)` | 激活打开的面板；组内调整到零起始位置 |
| `setSplitRatio(nodeId, ratio)` | 设置内部节点比例，必须为有限值且 0 < ratio < 1；实际呈现受最小尺寸约束 |
| `saveLayout()` / `restoreLayout(json)` | 输出版本 2 JSON，兼容读取版本 1；恢复失败时保持原布局 |
| `layout` / `panelIds` / `floatingWindows` | 主工作区树／注册 ID 列表／浮动标签组快照 |
| `layoutChanged()` / `panelsChanged()` | 布局（含浮动几何）变化／注册面板列表变化的通知；QML 属性绑定会自动更新 |
| `operationRejected(reason)` | 操作失败的开发诊断；错误原因不用于直接显示给最终用户 |
| `floatPanel(id, wholeGroup = false)` | 将主工作区面板或其整个标签组浮动；已浮动面板返回 false |
| `dockBack(windowId)` | 整个浮动组回停；恢复原标签位置／分割方向和比例，原目标不存在时并入首个主工作区组 |
| `floatingWindowForPanel(id)` | 返回浮动宿主 ID，未浮动返回空字符串 |
| `setFloatingGeometry(windowId, x, y, width, height)` | 更新逻辑像素几何；由窗口宿主自动调用，也可由应用设置 |
| `constrainFloatingWindows()` | 将浮动窗口移回现有屏幕的可用区域；创建／恢复、屏幕移除及可用区域变化时自动处理 |

布局树的每个节点有库生成的 `id`，`kind` 为 `split` 或 `tabs`。分割节点包含 `orientation`（`horizontal`／`vertical`）、`ratio`、`first`、`second`；标签节点包含 `tabs` 和 `active`。
应用使用 controller 操作，不直接修改快照。关闭后清理空节点、合并只有一个子节点的分割。

### 停靠到组旁，还是整个工作区旁

以下调用写在 QML 的信号处理器或 JavaScript 函数中，假定 `editor`、`log` 和 `properties` 已注册：

```javascript
// 只拆分编辑器所在的组：日志可能仍位于这两个面板下方。
workspace.controller.dock("properties", "editor", DockController.Right)

// 在整个主布局右侧放置属性列，左侧原有编辑器／日志组合保留。
// ratio 是第一个子节点（这里是左侧原布局）的比例，属性列占约 25%。
workspace.controller.dockToWorkspace("properties", DockController.Right, 0.75)
```

`ratio` 必须是有限数值且大于 0、小于 1，始终表示左侧或上侧的比例。因此 Left／Top 的新面板占 ratio，Right／Bottom 的新面板占 `1 - ratio`。最低尺寸可能使实际显示比例发生变化。唯一主面板停靠到自身工作区边缘时，布局不变。

## 保存和恢复布局

以下同样是信号处理器或函数中的调用片段：

```javascript
// 应用负责把字符串写入 Settings 或文件。
const snapshot = workspace.controller.saveLayout()
// 所有面板注册后，再读取并恢复。
if (!workspace.controller.restoreLayout(snapshot))
    console.warn("无法恢复工作区布局")
```

保存的是布局和浮动窗口几何，不是编辑器文字等业务数据；这些数据由应用另行保存。面板 ID 应跨启动保持稳定。恢复失败时原布局不变。

### 布局格式与兼容

恢复前完成文档和树校验：版本、节点 ID、面板 ID 唯一性、活动标签、比例、深度（64）和节点数（4096）；文档限制 1 MiB 字符。
未知面板 ID 被略过并清理空组；已注册但快照没有的面板保持关闭。面板注册完成后再恢复布局。
版本 2 在 `root` 外加入 `floating` 数组。每项包含 tabs 节点字段、`geometry`（x/y/width/height）和回停提示
`returnTarget`、`returnPosition`、`returnIndex`、`returnRatio`。节点与面板 ID 在主树和全部浮动组之间共同校验唯一性。
可选字符串 `returnNode` 记录原相邻子树的节点 ID，使分割组旁的面板可以恢复为完整侧列；旧版 v2 没有此字段时按面板提示回退。字段不是字符串时整份恢复请求被拒绝，当前布局保持不变。
窗口坐标必须有限，绝对值小于 1e7；宽高范围为 1–32768 逻辑像素。恢复后根据当前屏幕修正位置和大小。
版本 1 被当作没有浮动窗口的布局。读取旧版本不会保留当前浮动窗口；未来未知版本会被拒绝。
当前不记录窗口最大化／最小化状态。

## 浮动与回停

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
也可通过回停按钮或 `dockBack()` 返回。

## 当前限制

- 浮动窗口只支持标签合并，不支持内部四方向分割。
- 不能在两个独立的 `CatDockWorkspace` 之间拖动面板。
- 不支持边缘自动隐藏。
- Wayland、多显示器不同 DPI、实际屏幕热插拔仍需在目标设备验证；不能把 Windows 合成鼠标测试当作这些环境的验收。

## 实现与验证入口

- 布局／持久化：`src/uikit/qcatdockcontroller.h/.cpp`。
- 呈现与交互：`qml/CatDockWorkspace.qml`、`qml/CatDockPanel.qml`；内部 `DockTabButton.qml`、`DockDragArea.qml`、`DockDropOverlay.qml` 统一标题／标签、拖动和指示器，`DockDragPreview.qml` 提供穿透输入的随动缩略窗，`DockGeometry.js` 计算几何及方向命中，`DockFloatingWindow.qml` 管理原生宿主。
- 演示：`examples/uikit/gallery/GalleryDockPreview.qml`，Gallery 页面 ID `dock-workspace`。
- Gallery 窄屏／关闭／恢复回归：`tests/auto/gallery/data/tst_dock.qml`；示例在最小尺寸不足时提供横向滚动。
- C++ 行为：`tests/auto/dock/tst_dockcontroller.cpp`；QML 生命周期／交互：`tests/auto/qml/data/tst_dock.qml`。
- 浮动验证：`tests/auto/qml/data/tst_dockfloating.qml` 检查内容／焦点／弹层、关闭拒绝、图表像素、隐藏／销毁和屏幕外恢复；C++ 测试覆盖组回停、缺失目标、v1 兼容及无效 v2 数据。
- 浮动标签拖回测试：`tst_dockfloating.qml` 覆盖全局坐标命中、五方向提交、取消、宿主遮挡和鼠标／Esc 路径。
- 跨窗口合并：控制器测试检查失败时布局不变、宿主清理、保存恢复和回停；QML 测试检查两种来源的鼠标／Esc、预览、权限及空主工作区；Gallery 测试覆盖完整 Main 页面嵌套滚动。
- 运行方法见[验证说明](VALIDATION.md)，各次平台及结果见[历史记录](VALIDATION_HISTORY.md)。
