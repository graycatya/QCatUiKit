# 角色包与动画播放

[文档索引](README.md) · [组件目录](COMPONENTS.md)

`MascotPetController` 已提供本地与 qrc 角色包加载、精灵与 Lottie 播放、命名动作请求和角色区域交互。开发预览工具可播放自带的 CatGray Voxel 十个动作，切换动作、暂停、缩放并验证减少动效。`--lottie` 选择从 CatGray 桌宠 Blender 模型直接生成的十动作矢量包，窗口按钮可切换两种角色包。`CatMascotPet` 提供宿主直接持有的透明窗口、角色播放、气泡与窗口请求接口。窗口能力按运行会话反馈，不能据此宣布平台完整支持；组件不提供托盘；边缘气泡已提供接口；完整多屏与桌面交互仍待实机验收。

Windows 桌面原型及跨进程输入检查见 [平台原型与验收边界](MASCOTPLATFORMS.md)。它是独立手动验证程序，不是已交付的公共桌宠窗口。

## 运行预览

启用 `BUILD_TESTING=ON` 和 `QCATUIKIT_BUILD_MANUAL_TESTS=ON` 构建 `qcat-mascot-player`。在当前构建目录查找该可执行文件；Linux 通常位于 `tests/manual/mascot-player/`。不传参数时读取内置 qrc 包。可传 `--package /absolute/path/to/package` 或具体清单路径，`--action working` 选择起始动作。`--screenshot /tmp/catgray.png` 在启动后保存窗口截图并退出，适用于渲染检查。`--dark` 与 `--width 420 --height 520` 可检查深色和窄屏布局；独立安装时通过 `--import-path` 指定安装包的 QML 根目录。

该工具是开发用普通窗口，不提供桌面悬浮功能。其 Loader 访问库内私有视觉实现，应用不能把这个内部 URL 当作稳定 API。角色素材许可位于工具的 `catgray/LICENSE.txt` 与仓库 `LICENSES/CatGray-Voxel.txt`，与库代码许可分别适用。

## 大角色包加载

`packageSource` 设置后进入 Loading，完整清单、资源、数值和边界校验在后台完成，不跳过未选动作的检查。校验任务最多同时执行两个；VectorItem 的读取与 Lottie CPU 准备使用另一个最多两线程的队列。校验通过后加载默认动作；Lottie 首帧实际绘制后才进入 Ready。动作请求仍应在包可用后提交，Loading 时尚无动作清单的请求会被拒绝。

手动预览的私有视觉在换包校验时保留先前的画面及逻辑尺寸；换动作时冻结旧 Lottie 场景，新场景准备完成后切换。保留画面不提供旧包的可交互状态，失败或清空来源仍显示错误占位或清空。快速换源和对象销毁会使旧任务失效；已经开始的单次 JSON/ThorVG 解析会完成后再丢弃，不阻塞销毁方。

这项优化减少界面等待，不能消除复杂素材的 CPU 解码、首帧路径三角化和 GPU 上传成本。私有角色视觉还提供容量可配的动作缓存和后台预热，详见下文；不改变素材质量。耗时诊断方式见 [验证说明](VALIDATION.md)。

## 控制器接入

以下为完整的非可视 QML 示例，可在已有 QML 应用中实例化；需要窗口时使用下文的 CatMascotPet。

```qml
import QtQuick
import QCatUiKit

MascotPetController {
    id: pet
    property bool initialActionRequested: false
    packageSource: "file:///absolute/path/to/character/manifest.json"
    onPackageSourceChanged: initialActionRequested = false
    onStatusChanged: {
        if (status === MascotPetController.Ready && !initialActionRequested) {
            initialActionRequested = true
            requestAction("working")
        }
    }
    onActionEnded: (requestId, reason) => console.log(requestId, reason)
    onRequestRejected: (requestId, reason) => console.log(requestId, reason)
}
```

C++ 包含 `<QCatUiKit/MascotPetController>` 并链接 `QCatUiKit::QCatUiKit`。所有操作在对象所属线程执行。清单和完整资源校验、Lottie 读取及 CPU 解析在后台执行；切包使旧代次失效，QRhi 绘制在渲染线程执行。

| 接口 | 当前行为 |
| --- | --- |
| packageSource | file URL 指向目录或清单；qrc URL 指向清单或资源目录。目录优先 mascot.json，再 manifest.json。相同 URL 不重新加载；如需重载，先置空再设置 |
| status、errorCode、errorString | Null / Loading / Ready / Error；解析失败为 InvalidPackage，渲染失败为 RenderFailed；错误保留定位字段。精灵通过资源校验后 Ready，Lottie 在私有视觉收到有效 QRhi 绘制反馈后 Ready；切换动作时可重新进入 Loading；没有视觉窗口时可保持 Loading |
| availableActions、currentAction | 有效包动作名列表与当前动作；切包立即清空旧包，错误时不继续旧动画 |
| actionCacheLimitMiB | 每实例 Lottie 动作源文档总预算，默认 150 MiB，1 MiB = 1048576 字节；无固定动作数限制。0 关闭缓存和预热，负值拒绝并发出 configurationRejected("InvalidActionCacheLimit")；有效变更通知 actionCacheLimitMiBChanged。不是实际内存／显存上限 |
| requestAction(name, policy, options) | 返回实例内唯一字符串标识；下一事件循环处理。policy 为 Replace、Enqueue、IgnoreIfBusy；options 仅支持 loops，值为 -1 或正整数 |
| queueLength | 等待请求数，最多 64 个；默认底态不算忙碌，排队请求可立即替代默认动作 |
| pauseActions()、resumeActions()、paused | 冻结和继续当前帧时钟；保留有限循环的剩余时长与队列 |
| playbackEnabled | 视觉生命周期开关，false 时冻结时钟；私有视觉按有效可见性、窗口显示状态及主题 reducedMotion 更新 |
| resetToDefault() | 取消当前与排队请求，回到无限默认底态；保留手动暂停状态 |
| facing | left / right；缺少独立素材时仅在 allowMirror 为 true 时镜像。当前若任一动作不能提供该朝向则拒绝切换，以免后续动作失效 |
| petScale | 正有限缩放，默认 1；只影响显示大小，不改变帧时长 |
| logicalSize、footAnchor、bubbleAnchor | 角色局部逻辑尺寸与脚底／气泡逻辑锚点，实际使用镜像时 x 同步镜像；锚点不乘 petScale，通过 anchorsChanged 通知变更。控制器本身不移动窗口，CatMascotPet 在支持定位的会话中补偿脚底位置 |
| spriteFrame | 内部视觉只读快照，新增 renderer 区分 sprite/lottie；两者均有 source 和 mirrored。sprite 另含 atlasSize、frameSize、帧原点 x/y 与动作内 index。Lottie 不暴露连续口型参数 |
| fallbackSource | 已校验的静态占位 URL；Error 时私有视觉显示该图片，缺失或加载失败时显示默认占位，status 仍保持 Error |
| dragEnabled、dragAction、dragging | 默认允许拖拽，反馈动作默认空。有效 dragAction 在拖拽期间无限循环，不占用宿主请求标识 |
| beginDrag()、endDrag() | 供宿主／窗口接入实际拖拽生命周期；开始时可打断动作、取消队列，结束回默认底态。这两个方法本身不移动窗口 |
| interactionRegions | 当前动作区域的显示坐标快照，已应用实际镜像与 petScale；动作覆盖整体替换包级区域 |
| hitTest(position, forDrag) | position 为角色显示局部坐标；返回 id 与原始包逻辑坐标 logicalPosition，空 map 表示未命中；forDrag=false 匹配 clickable，true 匹配 draggable |
| activateRegion(position) | 使用同一命中规则，命中后发 regionActivated(name, logicalPosition)，否则返回 false |

`actionStarted(id, name)` 只对宿主动作发出；默认底态不产生请求完成事件。`actionEnded(id, reason)` 的当前原因包括 Completed、Interrupted、Cancelled、Ignored、Failed。自然结束后先执行 FIFO，再回默认动作。Replace 打断当前并取消旧队列；IgnoreIfBusy 忙时直接报告 Ignored。外部无限动作会占用执行权，直到替换、重置或切包。

`requestRejected(id, reason)` 报告 UnknownAction、NotReady、InvalidPolicy、InvalidOptions、InvalidLoops、QueueFull 或 InteractionBusy。无效请求不改变当前动作或队列。切包取消已提交但尚未分派的旧代次请求。`configurationRejected(reason)` 报告 FacingUnavailable、InvalidScale、UnknownDragAction 或 InteractionBusy，并保留原设置。错误请求的拒绝与动作终态均不会伪装成自然完成。

配置反馈时，beginDrag 发 dragStarted，打断当前请求并取消旧队列；拖拽期间新动作请求返回 InteractionBusy，即使在请求实际分派前已结束拖拽，也不会补执行。endDrag 发 dragFinished 并回默认动作，不恢复被打断动作。没有反馈动作时拖拽不改变调度。禁用拖拽会结束当前拖拽；切包也结束拖拽并丢弃旧动作。未知反馈动作被拒绝；切入不包含已配置反馈的角色包时会清除该配置并报告原因。反馈期间 resetToDefault 被拒绝，保留交互优先级。

区域命中按清单逆序选择第一个符合用途的区域，支持矩形、椭圆与多边形；拒绝非有限坐标及角色外框之外的点。实际镜像时命中先还原到包逻辑坐标，再检测原始区域。私有视觉的 TapHandler 调用 activateRegion；窗口穿透区域和移动仍由后续桌面适配负责。

桌宠 Lottie 由私有 VectorItem 关闭 autoPlay 后受调度器控制，通过 finished 通知自然完成；没有第二套时长推算。暂停、隐藏及 reducedMotion 控制 animationEnabled。渲染错误使当前请求 Failed、队列 Cancelled，停止播放并显示占位，不发 Completed。精灵图片运行时加载失败也使用同一错误路径。

## mascot.json 格式

每包固定一个 renderer。以下完整清单示例使用共享图集：每格 64×64，图集至少有三格，idle 使用首格，jump 使用后两格。帧从左到右、从上到下排列，允许最后一行存在未用格。

```json
{
  "schemaVersion": 1,
  "id": "sample-cat",
  "renderer": "sprite",
  "logicalSize": [64, 64],
  "footAnchor": [32, 60],
  "bubbleAnchor": [32, 0],
  "defaultAction": "idle",
  "defaultFacing": "right",
  "allowMirror": true,
  "actions": {
    "idle": {
      "loops": -1,
      "right": {"source": "atlas.png", "frameSize": [64, 64], "startFrame": 0, "frameCount": 1, "fps": 8}
    },
    "jump": {
      "loops": 1,
      "right": {"source": "atlas.png", "frameSize": [64, 64], "startFrame": 1, "frameCount": 2, "fps": 8}
    }
  }
}
```

`startFrame` 可省略，默认为 0；`loops` 必填，-1 为无限循环，正整数为有限循环。默认底态始终无限；宿主请求按动作 loops 或请求覆盖值执行。left/right 可分别提供独立素材；缺省 defaultFacing 为 right，缺省 allowMirror 为 false。默认朝向必须可用于所有动作。

可选 `fallbackImage` 为包内图片路径。`regions` 为包级区域列表，动作内同名字段整体覆盖包级列表。区域含非空唯一 id、shape（rect / ellipse / polygon），以及可选 clickable / draggable 布尔值。rect/ellipse 使用四元素 rect，多边形使用至少三个两元素 points。区域命中已接入；原生桌面输入区域尚未接入。

清单最多 1 MiB、256 个动作，每动作每方向最多 4096 帧、FPS 为 0.01–240、每组最多 128 个区域、每多边形最多 256 个点。图集边长最多 4096，帧尺寸须整除图集尺寸，帧范围不能超出格数；包内不同图片总 RGBA 解码预算为 128 MiB。当前固定图集上限是加载预算，尚未证明所有目标 GPU 的纹理能力。逻辑尺寸为 1–4096 整数，锚点须在角色外框内。图片会在解析阶段校验解码，实际显示复用 Qt Quick AnimatedSprite 的单帧采样，不新增 GPU 渲染器；帧间不混合，公共桌宠默认启用平滑缩放，可通过 smooth 关闭。

所有资源必须为包内相对路径；拒绝 URL、绝对路径、父目录引用及符号链接越界。只接受本地与 qrc 来源，不下载、不解压、不执行包内脚本。Lottie 清单使用 renderer=lottie，每个 left/right 条目只提供 source，不含精灵字段。单文档最多 64 MiB；基本 JSON 和外部图片／字体路径在包边界内校验，当前拒绝嵌入图片。外部资源的格式支持仍取决于现有 VectorItem；CatGray Lottie 示例的十个动作使用纯矢量 JSON，不依赖字体、图片或网络。Lottie 需要启用的 ThorVG 与有效 QRhi 环境，Qt Quick software 不支持，会明确 Error。

## CatGray manifest.json 兼容

当清单没有 renderer 且 type 为 sprite 时，读取现有 CatGray Sprite v1 字段：asset 为共享图集，frameSize 为单格尺寸，anchor 为 0–1 归一化脚底坐标，animations 中含 startFrame、frameCount、fps、loop。转换仅在内存中进行，不改写文件。

逻辑尺寸取 frameSize，footAnchor 为 anchor 与尺寸逐轴相乘，bubbleAnchor 默认为上边中点；默认动作取 stateMap.Idle，缺省为 idle。loop=true 对应无限动作，false 对应单次动作。该格式只提供 right，默认禁止镜像；七种业务状态只是宿主映射信息，不引入 Agent、TTS、音频或自主行为。示例 walk/run 为原地动作，jump 已含角色竖直位移。

自动测试和重复命令见 [验证方法](VALIDATION.md)。平台支持和本次执行结果保留在对应 OpenSpec change 中。


## Lottie 示例与现有 CatGray 动作

`tests/manual/mascot-player/catgray-lottie` 是从 `CatGray_DesktopPet.blend` 求值网格直接生成的角色包，包含与精灵包对应的全部十动作。动画使用 768×832 坐标画布、24 FPS，直接投影和裁剪模型表面，缩放不会引入精灵像素格；体素造型本身仍保留棱角。材质光照使用矢量平面着色，与 Cycles 渲染存在差异。来源、导出步骤与角色专用许可见 [工具说明](../../../tests/manual/mascot-player/README.md)。Lottie 自动测试同样使用实际 CatGray 包。

使用已有 CatGray 矢量动作时，将有使用权的 Blink.json、Mouth_Talk.json 放入新的本地角色目录，并添加以下完整 mascot.json；不需要更改原模型或嵌入脚本。下面把原始 512×512 矢量画布显示为 256×256：

```json
{
  "schemaVersion": 1,
  "id": "catgray-lottie-local",
  "renderer": "lottie",
  "logicalSize": [256, 256],
  "footAnchor": [128, 240],
  "bubbleAnchor": [128, 8],
  "defaultAction": "idle",
  "allowMirror": false,
  "actions": {
    "idle": {"loops": -1, "right": {"source": "Blink.json"}},
    "talk": {"loops": 1, "right": {"source": "Mouth_Talk.json"}}
  }
}
```

通过预览工具 `--package /absolute/path/to/new-package --action talk` 载入，包内路径仍按根目录校验。窗口中的错误文本和控制器 errorString 是实际后端错误，不能以占位图可见判断动画播放成功。

## 动作缓存、预热与切换延迟

私有角色视觉为每实例启用 Lottie 场景缓存。已播放的动作保留动画与渲染画布，再次切换可以跳过 CPU 解析及画布重建；预热完成的动作复用 CPU 场景，首次使用仍需挂接及绘制。动作文件仍通过后台读取检查，缓存不等于所有切换零开销。普通 CatLottieAnimation / VectorItem 不自动开启此缓存。

先显示默认动作，再优先预热配置的 dragAction、可选 working、jump，随后逐个准备其余动作和方向，相同来源只计一次。包无需包含这些固定名称；播放暂停时预热仍可推进，可见性为 false 时不保证预热进度。

actionCacheLimitMiB 默认 150，没有固定动作数量限制；预算按当前活动动作和非活动缓存的不同来源文档字节合计。预算足够时可保留全部动作；超出时按最近使用淘汰，预热本身不淘汰现有场景。当前动作即使单独超限仍可播放，此时不保留其他场景，切走后也不缓存它。动态减小预算在下一次渲染同步淘汰超额缓存；增大预算会重新尝试预热。设为 0 关闭缓存和预热，不停止当前动作。源文档成本不能代表解码内存或显存上限；复杂矢量素材仍会增加实际资源占用，切换期间旧画面、解码任务可暂时并存。

例如在 QML 对象声明中设置 `MascotPetController { actionCacheLimitMiB: 300 }`，或在 C++ 中调用 `controller.setActionCacheLimitMiB(300)`；不设置时使用 150。容量足够仍需等待首次预热完成，提前请求未就绪的动作仍会加载；GPU 首帧绘制不因 CPU 预热而消失。

切包、显式 VectorItem.reload()、渲染器销毁或 QRhi 更换会使缓存失效；不同实例不共享可变动画。切包期间保留的旧画面不会写入新包缓存。隐藏或暂停仍遵守原有播放冻结规则。

启用 `qcatuikit.vector.loading.debug` 可查看 `mascot cache hit resident`、`mascot cache hit prewarm`、`mascot cache miss`、`mascot cache evict` 与 `mascot prewarm finished`。启用 `qcatuikit.mascot.loading.debug` 可查看 `action first rendered feedback ms`，包含动作名和请求标识。该时间从动作开始执行算起，包含读取和渲染反馈经过 GUI 事件循环的时间，不含 Enqueue 等待，不是显示器扫描输出时刻。缓存命中和预热仍需实际绘制才能报告 Ready；测试方法见 [验证说明](VALIDATION.md)。


## 宿主直接集成桌宠窗口

`CatMascotPet` 是公共 QML Window，不依赖 Gallery、GNOME/KWin 扩展或配套服务。宿主决定创建、显示、隐藏和销毁，以及开机自启、应用启停、托盘策略与存储。组件不修改系统设置、不管理扩展、不自动持久化，也不会自行退出宿主进程。已有扩展探针仅为独立研究工具。

以下是完整的窗口 QML 示例，可由宿主加载或作为已有窗口的对象属性持有；将 packageSource 替换为自己的已部署角色包：

```qml
import QtQuick
import QCatUiKit

CatMascotPet {
    id: pet
    packageSource: "qrc:/characters/cat/mascot.json"
    visible: true
    onRegionActivated: (name, position) => console.log(name, position)
    onBubbleActionTriggered: (id, actionId) => console.log(actionId)
    onPlatformError: (code, detail) => console.log(code, detail)
}
```

宿主可调用 `show()`、`hide()`、`close()` 或销毁实例。默认 `closeHides=true`，close 仅隐藏，保留角色和队列；宿主仍能 show 恢复。Qt 应用的 quitOnLastWindowClosed 是宿主自己的进程策略，应由宿主配置。组件不调用 quit。`theme` 默认 Theme，`smooth` 默认 true，控制精灵平滑缩放；像素画可显式设 false。窗口使用独立工具窗，transientParent 固定为空；宿主最小化不会连带最小化桌宠。支持的平台请求跳过任务栏，默认 alwaysOnTop=true。

`controller` 是实例持有的 MascotPetController，供宿主访问完整动作信号、暂停状态和队列；不替换其所有权。窗口转发 packageSource、status、errorCode、errorString、availableActions、currentAction、petScale、facing、dragEnabled、dragAction、actionCacheLimitMiB，以及 requestAction、pauseActions、resumeActions、resetToDefault、regionActivated、dragStarted 和 dragFinished。

### 窗口能力和移动

`capabilities` 按功能提供 status 与 reason，`platformStatus` 保持 Pending，实际桌面验收单独记录。定位、平滑移动和置顶在原生 Wayland 明确 Unavailable；窗口不会连接已安装扩展以获得这些权限。skipTaskbar 同样报告当前会话的限制。Gallery 在 Linux Wayland 会话且有 DISPLAY、没有显式 QT_QPA_PLATFORM 时优先选 xcb，以获得置顶和任务栏控制；显式 -platform／QT_QPA_PLATFORM 选择会被保留。宿主应用可按自己的窗口需求选择 Qt QPA。offscreen/minimal 不能证明桌面能力。其他 QPA 的 Qt 请求路径报告 Unknown，表示可发起请求但仍需桌面验证，不能读作完整支持。

`alwaysOnTop` 默认请求置顶；不支持时不会施加标志，修改请求发 platformError。`inputTransparent` 默认 false，切换整窗穿透，宿主可随时设 false 恢复；`inputRegionsEnabled` 默认 false，默认整窗接收鼠标，不启用区域外穿透。只有宿主显式设 true 时，动态输入范围才来自当前动作、镜像、缩放后的声明区域和气泡矩形。Linux xcb 在 XShape 可用时仅更新 ShapeInput，不裁切角色画面；缺少 X11 开发依赖或 Xext 运行库的构建仍能显示角色，但 inputRegions 报告 Unavailable。Windows 使用不绘制内容的私有原生输入窗口承接声明区域，角色画面保持完整；原生鼠标消息转换为同实例 Qt 输入，支持默认全窗阻挡、区域外穿透与整窗穿透。输入窗口随角色窗口隐藏和销毁，不创建托盘或独立生命周期。创建／更新失败时发出 InputRegionUnavailable，能力报告不可用。macOS 采用完全透明的私有 AppKit 输入面板覆盖声明区域；视觉窗口整窗穿透，不应用形状掩码。该路径已在 Metal 下 DPR=1／1.25 以独立背后进程验证真实输入、气泡键盘及连续拖动，完整平台矩阵仍待验收。`dragEnabled` 控制角色拖动，原生 Wayland 使用 Qt 的系统拖动请求，其他桌面使用原生鼠标事件全局坐标；不通过 QML 局部坐标反复反推窗口位移。真实输入与透明合成须实机检查。

角色显示默认不申请键盘焦点。用户点击气泡操作按钮后，窗口按需申请焦点，按钮可用空格再次触发；Tab／Shift+Tab 在关闭按钮、文本滚动区和操作按钮之间循环，不要求更改 macOS 系统键盘导航设置。气泡关闭、窗口隐藏或失活、开启整窗穿透后恢复被动焦点策略；再次显示角色不会沿用上一轮气泡的键盘焦点权限。主动申请发生在点击完成后，避免在按下与释放之间修改原生窗口标志。平台可能拒绝激活请求，不能仅以调用成功作为跨应用焦点验收证据。


`screens` 提供 id、name、availableGeometry、devicePixelRatio。`setPosition(screenId, point)` 和 `moveTo(screenId, point, durationMs)` 的 point 均为目标屏幕可用区域左上角起算的逻辑坐标；空或失效屏幕使用当前屏幕，再回退主屏。setPosition 返回是否接受位置；moveTo 返回实例内唯一标识，终态经 moveEnded(id, reason) 报告 Completed、Cancelled 或 Failed。受限定位明确失败，不把 Wayland 内部坐标当作成功。`cancelMove()`、新移动、定位、拖动、切包和隐藏会取消旧轨迹；移动不切换动作或朝向。实际拖动期间，setPosition 返回 false、moveTo 报告 Failed，并发出 InteractionBusy，防止宿主轨迹与鼠标争用窗口；拒绝的请求不会在松开后重放。`constrainToAvailableGeometry` 默认 true，约束角色逻辑外框，气泡偏移不改变可定位范围；屏幕可用区、DPI 或边界约束开关变化后重新校正。超大角色会缩小 petScale 并发出 ScaleCorrected，effectivePetScale 可读回当前缩放。跨混合 DPI 和真实屏幕移除仍待完整验收。

### 纯文本气泡

在宿主事件处理器中调用：

```qml
const id = pet.showBubble("收到消息 <b>按纯文本显示</b>", {
    durationMs: 10000,
    actions: [{id: "reply", text: "回复"}]
})
pet.appendBubbleText(id, "，继续追加")
```

showBubble 返回 bubbleId，无效参数返回空字符串且保留旧气泡。options 支持 durationMs（默认 0，手动关闭）和最多八项 actions（非空唯一 id 与纯文本 text）。`bubbleId`、`bubbleText` 可观察当前内容。updateBubble(id, text)、appendBubbleText(id, text)、closeBubble(id) 拒绝旧标识。按钮只发 bubbleActionTriggered，不自动关闭或执行宿主业务。bubbleClosed 的原因为 Replaced、Closed、TimedOut 或 PackageChanged。隐藏冻结剩余时限，恢复继续；更新不重置时限，旧定时器不关闭新气泡。文字使用 PlainText 与受限滚动区域。

气泡按角色上方／下方的实际剩余空间选择方位、限制高度和水平对齐，长内容在内部滚动，在可定位会话中补偿窗口原点以保持角色锚点。原生 Wayland 无法精确补偿全局位置，组件报告定位不可用；气泡尺寸与布局仍由合成器约束，不能宣称完整边缘定位支持。宿主应保留恢复显示和关闭穿透的操作入口。

托盘完全由宿主应用实现。组件没有 trayEnabled、trayAvailable、trayIcon、trayActions 或 exitRequested 接口，不创建托盘图标、菜单或进程退出入口。宿主自己的托盘可调用 show/hide，并设置 inputTransparent 来恢复交互。

### 状态转移

`exportState()` 输出 version=1、screenId、position、petScale、facing、alwaysOnTop、inputTransparent、inputRegionsEnabled；不含包源、动作队列和气泡。宿主负责存储并在恢复前加载包。`restoreState(map)` 返回 ok、reason 或 corrected，拒绝无效版本／数值／朝向与不可定位会话；失效屏幕回退并报告 corrected。旧 v1 快照缺少 inputRegionsEnabled 时按 false 恢复。position 是 QPointF 值，直接传递 QVariant 快照；当前不提供 JSON 序列化转换。原生 Wayland 不恢复精确位置；宿主可分别恢复支持的外观属性。


## 精灵与 Lottie 的切换表现

PNG 图集使用 Qt AnimatedSprite 的单帧采样，固定 paused 并由控制器唯一时钟设置 currentFrame；关闭帧间混合。不会把整张大图集平移到裁切窗口，避免裁切和分数缩放引入的帧边界抖动。默认平滑缩放保留缩小时的细线，smooth=false 适用于明确需要最近邻采样的像素画。动作速度仍遵循包内 fps，CatGray PNG 样例为 8 fps，不通过加速播放掩盖低帧率。

桌宠 Lottie 私有视觉直接使用已有 VectorItem 和动作场景缓存，不叠加通用加载圈。缓存命中、后台准备与首帧接管仍可能经过 Loading，保留旧画面直至新场景可用；取消、错误和性能诊断不变。普通 CatLottieAnimation 的 Loading 提示不受桌宠策略影响。
