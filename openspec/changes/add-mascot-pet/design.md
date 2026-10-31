# Design

## Context

动机与范围见 [proposal.md](proposal.md)。已检查 `src/uikit/qml/CatWindow.qml`、`CatLottieAnimation.qml`、模块 CMake、vector 与 ui-components 主规格，以及 QML/vector 测试入口。

- CatWindow 是带标题栏、最小尺寸和边缘缩放交互的 ApplicationWindow，不直接作为桌宠外壳。
- CatLottieAnimation 已包装 VectorItem，提供播放、进度、主题 reducedMotion 和错误提示；当前 ThorVG QRhi 为实验后端，无 CPU 回退。
- 现有 QML 行为测试默认使用 software/offscreen，不能证明 Lottie GPU 或桌面输入穿透可用；vector 已有可选 GPU 检查入口。
- Dock 存在系统拖动和整窗穿透预览，但不等同于桌宠的区域输入、多屏定位或原生 Wayland 能力。
- 最初仅创建规划；实施范围与当前交付情况以任务清单和 validation.md 为准。未交付接口仍为设计建议。

## Goals / Non-Goals

**Goals:** 将角色表现、窗口策略、平台协议隔离，使多个独立桌宠共享公共契约；先建立平台能力证据，再完成组件。全部已确认范围由两个 delta specs 约束。

**Non-Goals:** 不公开页面内嵌 View，不建立统一多宠管理器，不改变 vector 的现有行为，不内置第三方服务或新渲染引擎。内部视觉 Item 仅作实现组合。

## Decisions

### 1. 已确认的产品决策

| 项目 | 决策 |
| --- | --- |
| 交付形态 | 仅桌面悬浮，完整窗口组件 |
| 平台 | Windows、macOS、Linux X11、原生 Wayland |
| Wayland 范围 | 按当前 Qt／合成器能力提供；不依赖扩展或辅助服务 |
| 平台目标 | 可直接嵌入宿主应用；能力受限明确反馈，不静默假报成功 |
| 输入 | 默认整窗接收鼠标；整窗穿透和区域外穿透均须宿主显式开启 |
| 区域 | 包声明矩形、椭圆、多边形命名区域，支持按动作覆盖，无逐帧透明度检测 |
| 动作 | 替换、排队、忙时忽略；可选拖拽抢占，结束回默认动作 |
| 包来源 | 本地目录与 qrc，无 ZIP、下载和脚本 |
| 渲染组织 | 每包一种后端，同时支持精灵图包与 Lottie 包 |
| 气泡 | 纯文本、增量更新、可选按钮、自动／手动关闭 |
| 托盘 | 完全由宿主应用实现，组件不提供托盘与退出菜单 |
| 移动 | 拖拽、外部定位、平滑移动，默认约束屏幕可用区域 |
| 实例 | 多个独立实例，统一管理及宿主托盘由应用自行实现 |
| 持久化 | 组件导出／恢复，宿主保存，恢复校正失效屏幕 |
| 外部表现 | 命名动作与气泡，无连续参数，不接入 Agent/TTS/音频 |
| 朝向缩放 | 左右朝向、统一缩放，包决定是否允许镜像 |
| 窗口策略 | 默认置顶、跳过任务栏、无原生 transient parent；宿主最小化不影响桌宠，不主动抢焦点 |
| 顺序 | 平台窗口能力原型先行 |
| 验收 | 允许分阶段；未实机通过标待验收，全部通过才宣布完整支持 |
| 实施推进 | 2026-10-28 用户授权开始实现，并明确先继续角色包解析与播放，保留平台验收待办 |

### 2. 公共窗口与内部组合

建议公开 QML `CatMascotPet`，作为 Window 外壳；公开 C++/QML `MascotPetController` 提供状态与请求入口。命名沿用库的 Cat 视觉类型与无 Cat 控制器惯例。

内部组合为：窗口 → 私有角色渲染 Item + 主题气泡；控制器 → 角色包解析、动作队列；私有桌面适配 → 输入区域、移动、层级、焦点和屏幕。平台类型与原生句柄不进入公共头。宿主创建、显示、隐藏和销毁实例，负责开机自启、应用启停、系统集成及存储；库不管理这些策略。

不继承现有 CatWindow 的标题栏和边缘缩放行为，复用 ThemePalette、CatLabel、CatButton、现有弹出与菜单能力。气泡默认与角色共用宿主窗口，布局改变窗口范围时补偿窗口原点以保持角色脚底锚点不动；原型若证明需要独立气泡窗口，可替换内部组合而保持同一契约。

每实例独立控制器与请求标识。Qt/QObject 所属线程执行状态变更；资源回调携带加载代次，切包和销毁使旧回调失效。

### 3. 公共接口草案

| 范围 | 建议接口 |
| --- | --- |
| 资源与状态 | packageSource、status、errorCode、errorString、availableActions、currentAction |
| 动作 | requestAction(name, policy, options) → requestId；pauseActions()、resumeActions()、resetToDefault() |
| 动作反馈 | actionStarted(id, name)、actionEnded(id, reason)、requestRejected(id, reason)、queueLength |
| 外观交互 | facing、petScale、dragEnabled、dragAction、regionActivated(name, logicalPosition)、dragStarted/dragFinished |
| 气泡 | showBubble(text, options) → bubbleId、updateBubble(id, text)、appendBubbleText(id, text)、closeBubble(id) |
| 气泡反馈 | bubbleActionTriggered(bubbleId, actionId)、bubbleClosed(id, reason) |
| 桌面 | alwaysOnTop、inputTransparent、constrainToAvailableGeometry、setPosition(screenId, point)、moveTo(screenId, point, durationMs)、cancelMove() |
| 平台反馈 | capabilities、platformStatus、moveEnded(id, reason)、platformError(code, detail) |
| 窗口生命周期 | closeHides；宿主持有对象，托盘及进程策略由宿主实现 |
| 保存恢复 | exportState() → map、restoreState(map) → result |

请求标识在实例生命周期内唯一。所有请求结果可区分 Completed、Interrupted、Cancelled、Ignored、Rejected、Failed；自然完成只对应 Completed。默认循环不产生宿主请求的重复完成事件。无效动作与不支持能力均保留可用状态并返回原因。

### 4. 角色包 v1 草案

入口建议为 `mascot.json`，`packageSource` 可指向目录或具体清单；qrc 可直接指向清单。所有素材路径相对包根解析，不允许越界，包括经本地符号链接解析后的越界。未知主版本拒绝，扩展元数据放入独立 metadata 对象。

```json
{
  "schemaVersion": 1,
  "id": "sample-cat",
  "name": "Sample Cat",
  "renderer": "sprite",
  "logicalSize": [160, 160],
  "footAnchor": [80, 150],
  "bubbleAnchor": [80, 12],
  "defaultAction": "idle",
  "defaultFacing": "right",
  "allowMirror": true,
  "fallbackImage": "preview.png",
  "regions": [
    {"id": "body", "shape": "ellipse", "rect": [24, 30, 112, 120], "clickable": true, "draggable": true}
  ],
  "actions": {
    "idle": {
      "loops": -1,
      "right": {"source": "idle.png", "frameSize": [160, 160], "frameCount": 8, "fps": 12}
    }
  }
}
```

精灵图首期采用等尺寸网格图集，按行从左到右取帧，关闭帧间混合，允许配置平滑缩放；每个方向可选 startFrame（默认 0），与 frameCount 共同界定动作在共享图集中的范围。帧尺寸须整除图集尺寸，允许尾部未用格，帧范围不得越界。

兼容用户已制作的 CatGray Sprite v1 manifest.json：type/asset/frameSize/anchor/animations/stateMap 在内存转换为统一包结构，归一化 anchor 转逻辑像素，loop 转 loops。默认朝向 right、禁止镜像，默认动作来自 stateMap.Idle 或 idle；不改写源素材。当前阶段固定图集边长最多 4096、不同图片总解码预算 128 MiB；运行时 GPU 纹理能力验证与桌面窗口仍待后续接入。Lottie 包 renderer 为 lottie，朝向条目使用 source 指向每动作独立 JSON，不提供图集字段或按进度驱动口型。共用动作 loops、区域及锚点。

逻辑尺寸、锚点和区域统一使用角色局部逻辑像素；按动作覆盖的 regions 整体替换包级列表，重叠区域后声明者优先。左右镜像同步变换区域与气泡锚点；脚底全局位置保持稳定。

建议清单上限 1 MiB、动作 256 项、每组区域 128 项、精灵帧 4096 帧；图集上限按当前纹理能力校验，Lottie 沿用现有 64 MiB 单文档上限。数值均要求有限，缩放为正，loops 为 -1 或正整数。具体限额属于设计默认值，可在评审时调整。

替代方案为允许任意 QML 包与混合后端；因首期协议、生命周期与渲染边界更复杂而不采用。

### 5. 动作状态与打断建议

- 默认动作是可被排队请求立即替代的底态，不算 Busy。活动请求或拖拽反馈算 Busy。
- Replace 打断当前请求并取消旧队列；Enqueue 按 FIFO；IgnoreIfBusy 立即报告 Ignored，不入队。队列建议上限 64，超限拒绝新请求。
- 有限请求结束后先执行队列，再回默认动作。无限外部动作可占用队列执行权，直至替换或 resetToDefault；此行为须在文档明确。
- 配置拖拽反馈时，拖拽取消当前请求与队列，反馈动作优先；期间新动作请求拒绝为 InteractionBusy，结束回默认动作。不配置时，拖拽仅控制窗口并发事件。
- pauseActions 冻结当前动作，恢复继续；隐藏和 reducedMotion 同样冻结时钟但保留请求。停止推进不会产生伪完成，宿主可显式替换或重置。
- 切包取消当前与排队请求并关闭旧气泡，旧动作完成回调按代次丢弃；无效新包进入错误与占位状态，不播放旧包动作。

这些规则使请求有确定终态，避免为首期增加优先级栈和动作恢复机制。

### 6. 渲染与气泡

精灵播放组合 Qt Quick AnimatedSprite，暂停其自身时钟，通过控制器时钟选择网格帧，复用整张图集纹理，不新建 GPU 渲染器；Lottie 私有视觉直接使用 VectorItem，不叠加通用 Loading 圈，关闭其独立自动播放，由调度器启动。两后端统一开始、结束、暂停与失败回调。占位使用现有 Image／CatSvgImage 等组件，不掩盖错误状态。

气泡使用 Qt 纯文本显示，不执行 HTML、Markdown 或链接。默认一次显示一个气泡，show 替换旧气泡并通知 Replaced；增量更新携带 bubbleId，拒绝过期写入。自动关闭从 show 起计时、更新不重置；durationMs=0 表示手动关闭。隐藏期间冻结剩余时限。操作按钮仅发事件，不默认关闭；宿主可关闭或替换。尺寸超限使用受限滚动区域，文字字号跟随主题、不随角色缩放。文本首选视口最多 240 逻辑像素，并受上下实际剩余空间限制；横向按气泡超出可用区的实际距离计算角色局部偏移，不用左右两端二选一，气泡显示／增量更新／关闭时通过窗口补偿保持角色原点。拖动期间以鼠标目标的角色原点驱动气泡方位，偏移改变同步拖动起点，不以已被原生整窗边界限制的位置反推目标，也不由延迟布局定时器回写旧原点。

### 7. 桌面能力适配与原型门槛

每平台实现统一私有接口，按功能报告 Supported、Unavailable、Unknown 与 reason；另行记录平台验收 Pending、Passed、Failed，不能混为一个布尔值。平台入口选择来自实际会话，原生 Wayland 与 XWayland 分列。

前置原型必须验证透明窗口、非矩形输入区域、整窗穿透、拖拽、精确定位／平滑移动、层级与焦点、多屏 DPI、气泡边界和宿主生命周期。Windows/macOS/X11 先验证 Qt 能力，不足处封装最小平台实现；原生 Wayland 不假设可以精确定位或置顶；Qt 无法提供时返回 Unavailable，不尝试连接桌面配套。

原型交付必须明确支持的系统／桌面／Qt 版本、使用的协议、所需权限与受限能力。平台受限时保留角色展示和可用交互，通过能力与请求失败说明限制，不宣称完整支持。具体系统调用和桌面协议在该阶段通过源码与官方资料验证，本文不声称已有可用实现。

公共组件不安装、启用、禁用、探测或连接桌面扩展。已有 GNOME/KWin 配套研究不属于公共组件依赖和验收前置条件。扩展启停与系统集成由宿主应用自行实现。

Windows 前置原型位于 `tests/manual/mascot-probe`，使用独立 Win32 背后进程记录实际鼠标、键盘及焦点消息。Qt Quick 浮层通过 QWindow.setMask 和 WindowTransparentForInput 控制区域与整窗输入，通过被动窗口标志及 WM_MOUSEACTIVATE 保持普通显示／点击不抢焦点；主动气泡和托盘菜单可申请前台及键盘焦点。Qt Widgets 只用于探针的 QApplication、QSystemTrayIcon 和 QMenu，不加入库依赖。原型输出与完整平台验收分别记录，主规格仍不提前同步。具体运行入口及局限见 MASCOTPLATFORMS 与 VALIDATION。

Windows 公共适配使用视觉窗口与私有无绘制输入窗口组合：视觉窗口保持完整并整窗穿透，输入窗口以 WS_EX_NOREDIRECTIONBITMAP 创建，形状仅由声明区域及气泡形成；默认不开启区域模式时承接整个窗口。原生鼠标消息按视觉窗口 DPR 转为同实例 Qt 鼠标／滚轮事件，输入窗口持有拖动捕获，失去捕获或隐藏时释放交互。键盘焦点仍由气泡显式向公共窗口申请。输入窗口是视觉窗口的原生 owned window，不独立激活、不进入任务栏，隐藏及销毁与实例同步。失败向宿主报告 InputRegionUnavailable 并标记能力不可用。该组合不向背后应用转发消息、不安装钩子或轮询鼠标，不更改公共宿主生命周期契约。原生验证必须使用独立背后进程，而非仅验证同线程 HTTRANSPARENT 或仅抓取 Qt 窗口画面；macOS 公共区域适配见下述机制，单屏 Metal 真实输入证据见 validation.md。

macOS 公共适配将 QRegion 的非重叠矩形映射为透明 nonactivating NSPanel。视觉及输入面板按 alwaysOnTop 显式设置 AppKit 普通／置顶层级，避免 Qt Tool 在关闭置顶后仍处浮动层。面板独立于原生 child-window 分组，窗口／工作区通知驱动同步，并以 CoreGraphics 窗口 ID 顺序保持紧邻视觉窗口上方。显式 ignoresMouseEvents=false 使完全透明的矩形接收鼠标，视觉窗口保持整窗穿透，区域外直接到背后应用。输入面板不接受键盘焦点，原生事件经视图比例换算交给同实例 Qt 窗口，键盘仍由公共气泡主动申请并显式激活本应用；视觉窗口通过 AppKit ignoresMouseEvents 实现穿透，避免 Qt WindowTransparentForInput 同时拒绝键盘焦点。保留按下视图至释放，隐藏／销毁清理；不使用全局钩子、光标轮询或系统输入权限。面板数量随区域分解复杂度增长，缓存不变区域并释放多余面板；不以裁切视觉或包围矩形放宽形状。此实现与既有矩形探针不同，验收须分别记录系统命中、进程内事件与独立背后进程真实输入。

### 8. 移动、生命周期与状态

定位统一定义为目标屏幕可用区域左上角起算的逻辑坐标，位置指角色逻辑外框左上角；适配转换为原生坐标。屏幕标识使用可用稳定属性组合并带回退，不能假设跨机器唯一。屏幕不存在优先当前屏幕、再主屏；角色大于可用区域时先降低有效缩放以完整显示，并通知校正。

平滑移动按时间插值，不以帧数推算时长；拖拽、重新定位、新移动、切包、窗口隐藏或屏幕移除取消旧移动并通知。移动仅改变位置，不自动切换走路动作或朝向。用户拖动时边界限制不能阻止跨屏，目标屏幕根据拖拽位置更新。

托盘完全由宿主实现；组件不提供托盘图标、菜单或退出事件。关闭默认仅隐藏，宿主保留 show/hide 和穿透恢复入口。窗口不与宿主建立 native transient parent，独立于宿主最小化；可用平台显式请求跳过任务栏。原生 GNOME Wayland 无通用置顶／跳过任务栏协议，能力明确受限；Gallery 在未指定 QPA 且 XWayland 可用时使用 xcb，宿主可自行选择平台路径。

exportState 仅输出版本化 UI 配置和屏幕相对位置，不包含角色包文件内容、队列、气泡文本、服务数据或应用退出策略；角色源由宿主另存并先行加载。restoreState 先完整校验再应用，校正信息单独返回，不自动持久化。

### 9. 文件与集成方向

新增 C++ 控制器及私有实现置于 `src/uikit/mascot/`，公开头按现有三组安装／转发列表维护；QML 窗口及私有视觉辅助置于 `src/uikit/qml/` 并显式注册内部类型。平台原型留在独立手动测试目录，不进入公共库依赖。

Gallery 增加 mascot-pet 页面，使用真实公共 API 驱动独立悬浮窗口，提供精灵／Lottie 切换、动作队列、气泡、移动、穿透、实例和能力诊断。不得以 Gallery 私有辅助类形成应用依赖。

后续更新 COMPONENTS、README 索引、MAINTENANCE、专门 MASCOTPET 文档、BUILDING 和技能指引；重复验证步骤进入 VALIDATION，执行证据进入本 change。示例角色资源需可分发许可及来源记录。

## Risks / Trade-offs

- [Wayland 环境能力与扩展版本差异] → 实际能力反馈、版本矩阵、受限请求失败测试；未验证不承诺支持。
- [跨平台实机暂不可得] → 分阶段记录待验收，提供探针与步骤；可进入后续实现，但完整支持声明必须等待证据。
- [Lottie GPU／格式限制] → 保留现有错误契约，按样例与后端实机验收，不以软件测试替代。
- [气泡改变窗口尺寸导致角色跳动或拦截输入] → 以脚底锚点补偿原点，同步更新原生输入范围，并做背后窗口点击验证。
- [宿主最小化与任务栏策略] → 解除原生 transient 关系，使用平台任务栏提示并独立验收，托盘和退出策略由宿主实现。
- [内存与加载尖峰] → 清单与资源校验、限制图集和队列规模，按动作预加载策略评估峰值；不作未测性能承诺。

## Migration Plan

这是新增能力，无旧数据迁移。按平台探针、核心角色、窗口集成、Gallery／文档、实机验收推进；每阶段保留验证记录。回退时由宿主停止使用新组件，既有控件和 vector API 不变。未验收的平台保持 Pending，不自动归档为完整交付。

## 大角色包异步加载补充

包完整校验在后台执行，不降低包边界、图像、外部资源及数值检查；GUI 线程仅接收当前加载代次的值结果。取消在读块和动作边界检查，单次 JSON / 解码器内部调用完成后再次检查。后台任务不捕获控制器或 VectorItem 裸指针，销毁不等待任务。

VectorItem 异步读取本地/qrc 字节；Lottie CPU 解码由有界后台线程池处理，每个场景独占，不并发访问同一 ThorVG 对象。每个线程上的 ThorVG init/term 成对，渲染器存活期间保持后端引用，取消与销毁先清理发布结果。QRhi 资源创建、场景挂接与绘制仍留在所属渲染线程。加载期间冻结并保留旧场景，新场景准备完成后切换；错误或清空来源仍遵循既有错误/Null 契约。记录读取、校验、解码及首次绘制耗时，不承诺所有 GPU 上传零开销。按本次用户授权增加下述有限动作缓存与预热。

## 动作缓存与预热补充

用户于 2026-10-28 授权优化动作切换，随后明确要求按可配置容量限制、默认 150 MiB，取消固定动作数。公开 MascotPetController.actionCacheLimitMiB，非负整数，默认 150；0 关闭缓存与预热，负值保持原配置并通知 InvalidActionCacheLimit。预算按不同动作来源的源文档字节计量，包括当前活动动作和非活动缓存，按最近使用淘汰；活动动作即使单独超出预算也必须能播放，此时不保留其他场景，离开后不缓存它。准入成本不是解码内存或显存上限。缓存仅由私有 mascot 集成启用，不改变普通 VectorItem 默认行为。已播放动作保留 Animation 和所属 QRhi canvas；预热动作后台读取、解析 CPU 场景，首次使用仍需挂接并绘制。

预热优先包默认动作、配置的拖拽动作、可选 working / jump，其后覆盖其余所有动作和方向，并按来源去重，不要求包具备固定名称。一次只发起一个推测预热任务，不为预热淘汰已有缓存；预算不足时跳过，实际请求仍可按需加载并淘汰旧缓存。暂停时钟不阻止后台预热推进；不反复推测加载已经淘汰的动作。动态增大预算允许重新尝试，缩小预算在下一次渲染同步时淘汰超额缓存且不中断当前动作。切包、重载、渲染器或 QRhi 更换时失效，保留旧画面的对象不能加入新代缓存。缓存场景不在工作线程与渲染线程并发使用。

日志分别报告 resident / prewarm 命中、淘汰和动作启动到首帧渲染反馈的延迟。渲染反馈经 GUI 事件循环交付，不等于屏幕扫描输出时刻；测量不含 Enqueue 等待执行时间，不对未实测设备承诺 50 ms。预热占用额外 CPU、内存；低频动作与预算外动作继续按需加载。


### macOS 本轮验收范围

2026-10-30 用户明确取消本轮多屏、混合 DPI 与拔屏测试。这三项记为 Excluded，不阻塞任务7.3约定范围验收，也不记为 Passed。屏幕坐标与回退实现保持原契约；平台结论必须同时列出排除项。其余范围为单屏透明及层级、PNG/Lottie播放与动作、区域与整窗输入、焦点及键盘、带气泡拖动和移动、气泡布局、多实例／状态、Gallery最小化／隐藏／退出清理，以及Spaces／全屏的可见性和返回恢复。系统安全界面及独占全屏仍不承诺覆盖。
