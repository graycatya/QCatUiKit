# 角色包与动画播放

[文档索引](README.md) · [组件目录](COMPONENTS.md)

`MascotPetController` 已提供本地与 qrc 角色包加载、精灵与 Lottie 播放、命名动作请求和角色区域交互。开发预览工具可播放自带的 CatGray Voxel 十个动作，切换动作、暂停、缩放并验证减少动效。`--lottie` 选择从 CatGray 桌宠 Blender 模型直接生成的十动作矢量包，窗口按钮可切换两种角色包。当前尚无公共 `CatMascotPet` 桌面窗口；透明区域穿透、定位、气泡和托盘仍待实现，不能据此宣布平台支持。

## 运行预览

启用 `BUILD_TESTING=ON` 和 `QCATUIKIT_BUILD_MANUAL_TESTS=ON` 构建 `qcat-mascot-player`。在当前构建目录查找该可执行文件；Linux 通常位于 `tests/manual/mascot-player/`。不传参数时读取内置 qrc 包。可传 `--package /absolute/path/to/package` 或具体清单路径，`--action working` 选择起始动作。`--screenshot /tmp/catgray.png` 在启动后保存窗口截图并退出，适用于渲染检查。`--dark` 与 `--width 420 --height 520` 可检查深色和窄屏布局；独立安装时通过 `--import-path` 指定安装包的 QML 根目录。

该工具是开发用普通窗口，不提供桌面悬浮功能。其 Loader 访问库内私有视觉实现，应用不能把这个内部 URL 当作稳定 API。角色素材许可位于工具的 `catgray/LICENSE.txt` 与仓库 `LICENSES/CatGray-Voxel.txt`，与库代码许可分别适用。

## 大角色包加载

`packageSource` 设置后进入 Loading，完整清单、资源、数值和边界校验在后台完成，不跳过未选动作的检查。校验任务最多同时执行两个；VectorItem 的读取与 Lottie CPU 准备使用另一个最多两线程的队列。校验通过后加载默认动作；Lottie 首帧实际绘制后才进入 Ready。动作请求仍应在包可用后提交，Loading 时尚无动作清单的请求会被拒绝。

手动预览的私有视觉在换包校验时保留先前的画面及逻辑尺寸；换动作时冻结旧 Lottie 场景，新场景准备完成后切换。保留画面不提供旧包的可交互状态，失败或清空来源仍显示错误占位或清空。快速换源和对象销毁会使旧任务失效；已经开始的单次 JSON/ThorVG 解析会完成后再丢弃，不阻塞销毁方。

这项优化减少界面等待，不能消除复杂素材的 CPU 解码、首帧路径三角化和 GPU 上传成本。当前没有全动作预加载或解码缓存，也不改变素材质量。耗时诊断方式见 [验证说明](VALIDATION.md)。

## 控制器接入

以下为完整的非可视 QML 示例，可在已有 QML 应用中实例化；视觉窗口后续另行提供。

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

C++ 包含 `<QCatUiKit/MascotPetController>` 并链接 `QCatUiKit::QCatUiKit`。所有操作在对象所属线程执行。加载在下一次事件循环处理，切包使旧加载代次失效；解析和图片校验在该线程同步完成，不宣称后台解码。Lottie 的 ThorVG 解析和 QRhi 绘制沿用 VectorItem 的渲染线程实现。

| 接口 | 当前行为 |
| --- | --- |
| packageSource | file URL 指向目录或清单；qrc URL 指向清单或资源目录。目录优先 mascot.json，再 manifest.json。相同 URL 不重新加载；如需重载，先置空再设置 |
| status、errorCode、errorString | Null / Loading / Ready / Error；解析失败为 InvalidPackage，渲染失败为 RenderFailed；错误保留定位字段。精灵通过资源校验后 Ready，Lottie 在私有视觉收到有效 QRhi 绘制反馈后 Ready；切换动作时可重新进入 Loading；没有视觉窗口时可保持 Loading |
| availableActions、currentAction | 有效包动作名列表与当前动作；切包立即清空旧包，错误时不继续旧动画 |
| requestAction(name, policy, options) | 返回实例内唯一字符串标识；下一事件循环处理。policy 为 Replace、Enqueue、IgnoreIfBusy；options 仅支持 loops，值为 -1 或正整数 |
| queueLength | 等待请求数，最多 64 个；默认底态不算忙碌，排队请求可立即替代默认动作 |
| pauseActions()、resumeActions()、paused | 冻结和继续当前帧时钟；保留有限循环的剩余时长与队列 |
| playbackEnabled | 视觉生命周期开关，false 时冻结时钟；私有视觉按有效可见性、窗口显示状态及主题 reducedMotion 更新 |
| resetToDefault() | 取消当前与排队请求，回到无限默认底态；保留手动暂停状态 |
| facing | left / right；缺少独立素材时仅在 allowMirror 为 true 时镜像。当前若任一动作不能提供该朝向则拒绝切换，以免后续动作失效 |
| petScale | 正有限缩放，默认 1；只影响显示大小，不改变帧时长 |
| logicalSize、footAnchor、bubbleAnchor | 角色局部逻辑尺寸与脚底／气泡逻辑锚点，实际使用镜像时 x 同步镜像；锚点不乘 petScale，通过 anchorsChanged 通知变更。当前未执行桌面坐标补偿 |
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

Lottie 由 CatLottieAnimation 关闭 autoPlay 后受调度器控制，通过 finished 通知自然完成；没有第二套时长推算。暂停、隐藏及 reducedMotion 控制 animationEnabled。渲染错误使当前请求 Failed、队列 Cancelled，停止播放并显示占位，不发 Completed。精灵图片运行时加载失败也使用同一错误路径。

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

清单最多 1 MiB、256 个动作，每动作每方向最多 4096 帧、FPS 为 0.01–240、每组最多 128 个区域、每多边形最多 256 个点。图集边长最多 4096，帧尺寸须整除图集尺寸，帧范围不能超出格数；包内不同图片总 RGBA 解码预算为 128 MiB。当前固定图集上限是加载预算，尚未证明所有目标 GPU 的纹理能力。逻辑尺寸为 1–4096 整数，锚点须在角色外框内。图片会在解析阶段校验解码，实际显示复用 Qt Quick Image 与裁剪，不新增 GPU 渲染器；帧间不混合，私有视觉默认关闭平滑缩放。

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
