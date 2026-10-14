# LED 布局、Deck、Storage 与设备输出

[文档索引](README.md) · [渲染及旧 RGBA8 API](LEDPREVIEW.md)

这些类型均来自 `import QCatUiKit`，C++ 链接 `QCatUiKit::QCatUiKit`。
`LedDeck`、`LedMixer`、`LedStorageBrowser`、`LedLayoutSettings`、`LedPreviewSettings`、
`LedOutputSettings` 是公共 QML 控件，不依赖 Gallery。

## 最小可运行装配

```qml
import QtQuick
import QtQuick.Layouts
import QCatUiKit

Item {
    width: 1280; height: 900
    LedProject {
        id: project
        Component.onCompleted: {
            repository.populateExamples()
            const s = repository.storages[0].id
            deckA.browseStorage(s); deckB.browseStorage(s)
            deckA.activatePlace(s, repository.places(s)[0].id)
            deckB.activatePlace(s, repository.places(s)[2].id)
            mixer.conversion = Lighting.ApproximateWhite
        }
    }
    CatScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        GridLayout {
            width: parent.width
            columns: width < 920 ? 1 : 3
            LedDeck { title: "A"; runtime: project.deckA; Layout.fillWidth: true; Layout.alignment: Qt.AlignTop }
            LedMixer { runtime: project.mixer; Layout.fillWidth: true; Layout.alignment: Qt.AlignTop }
            LedDeck { title: "B"; runtime: project.deckB; Layout.fillWidth: true; Layout.alignment: Qt.AlignTop }
        }
    }
}
```

`LedProject` 只是可选装配：拥有 layout、registry、repository、deckA、deckB、mixer、output、
memoryBackend。应用可以独立创建并注入这些对象，不必采用该装配。默认项目不包含预设或 Patch；
`populateExamples()` 显式填充两个集合、各四个可播放预设及空槽位。
Gallery 配置三个设备分组，分别观察前三个逻辑灯点。

## 公共接口与线程边界

| 头文件 / QML 类型 | 作用 |
| --- | --- |
| `LedLayoutModel` / `LedLayoutModel` | 布局校验、异步应用、不可变布局快照 |
| `LightingFrame` / `Lighting`, `PreviewColorAdapter` | RGB/RGBW/RGB_CW_WW、不含糊的 RGBA8 适配 |
| `LedEffectRegistry` / `LedEffectRegistry` | 注册 Provider、效果参数描述及独立实例 |
| `LedStorage` / `LedStorageRepository`, `LedPlaceModel` | 内存仓库、版本化 JSON、列表模型、缩略图 |
| `LedDeck` / `LedDeckRuntime` | 独立时钟、预设工作副本、连续通道帧 |
| `LedMixer` / `LedMixerRuntime` | 混合、冻结、Master、Blackout、公共最终帧 |
| `LedOutput` / `LedOutput`, `MemoryOutputBackend` | Profile、Patch、异步最新帧发送接口 |
| `LedProject` / `LedProject` | 可选装配和项目 JSON |

真实头命名为 `qcat*.h`，同时安装带 `.h` 和无扩展名的转发头，例如
`<QCatUiKit/qcatlightingframe.h>`、`<QCatUiKit/LightingFrame.h>`、`<QCatUiKit/LightingFrame>`。

配置方法、模型、Registry 和 Adapter 在对象所属线程调用。`QCatLightingFrame` 是不可变值，
构造时深拷贝外部字节；`latestFrame()` 可跨线程读取。灯点位置和通道均为连续缓冲区。
旧 `LedSource::setLayout/submitFrame` 仍可由生产线程提交。

布局 `apply()` 合并为一个正在运行和一个最新待处理请求。每个 Deck 最多一个 render 作业和一个
最新选择请求；旧实例晚到的帧不能覆盖新选择。作业只持有独立状态，无须持有 Deck QObject。
缩略图最多两个在途作业、128 张缓存，使用独立实例在 32×16 布局、t=0 生成静态 PNG；
可见 delegate 按需请求。输出发送最多一个在途事务，下一周期读取最新完整帧，慢后端不会阻塞效果线程。

新管线限制为 **100 万点**，RGB/RGBW/五通道颜色分别为每帧约 3/4/5 MB，另有布局、预览 RGBA、
混合和上传缓冲。旧 Source 的 1000 万点输入限制保持不变。容量限制不是帧率保证。

## 布局

`configuration` 是带 `version:1` 的 QVariantMap。`validateConfiguration()` 返回字段错误，
`apply()` 在后台生成并保留最后请求；失败不替换当前布局。`applyNow()` 是同步 C++/测试/配置恢复入口，
交互式大布局使用 `apply()`。`configuration` 和 `snapshot()` 只描述已经应用的状态。

布局、预览设置和灯具映射对话框先编辑草稿。数值输入支持 Enter、Tab 或点击其他控件结束编辑；
直接点击“应用”也会校验并提交当前输入，无须先按 Enter。非法文本会阻止应用并聚焦对应输入框。
“取消”丢弃草稿，重新打开读取已应用值；“默认值”同时重置数值和显示文本。
Deck／Place 速度及设备发送间隔在结束编辑后直接更新。

| type | 参数和数量 |
| --- | --- |
| Matrix2D | countX/countY；countZ 固定 1；spacingX/Y |
| Matrix3D | countX/countY/countZ、spacingX/Y/Z；数量为三者乘积 |
| Ring | count、radius、startAngle（度）、clockwise；无重复首尾 |
| Disk | radialLayers 表示中心以外等距同心环数量；angularPoints 为每环点数；总数 `1+radialLayers*angularPoints` |
| SphereSurface | count、radius；黄金角球面采样，不代表单点球体 |
| Custom | positions 为 XYZ 三元组数组；不推测 countX/Y/Z |

矩阵原点居中：`X=(x-(nx-1)/2)*sx`、`Y=((ny-1)/2-y)*sy`、
`Z=(z-(nz-1)/2)*sz`。X 向右、Y 向上，索引 `x+nx*(y+ny*z)`，颜色数组与这个点序一致。
64×32 与 16×16×8 都恰好 2048 点。空矩阵及单点合法；半径和间距必须有限且为正。

布局快照携带项目 UUID 和 revision。三个 Adapter 分别把这个身份映射到各自 Source 的本地 layoutId；
数字相同不代表 Source 间布局相同。布局替换先发布黑帧，旧版本帧不能进入新点位。

## 形状、相机和预览设置

| 新属性 | 语义 |
| --- | --- |
| pointShape | Square / Circle / Cube / Sphere；默认 Circle |
| sizeMode | ScreenPixels（默认，沿用 pointSize）或 SceneUnits（使用 sceneSize） |
| sceneSize | 场景单位直径；Cube 为边长。立体几何会随透视和朝向改变投影 |
| displaySpacing | X/Y/Z 显示中心间距因子，0.1–10；不改变源 XYZ、效果采样或设备值 |
| gapX/gapY | 规则矩阵二维视图的间隙比例，0–0.95 |
| sliceAxis | 0=原 XY 投影，1/2/3=展开 X/Y/Z 切片；只对规则矩阵二维视图生效 |
| slicesVertical | 横向或纵向连续排列切片；相邻切片留一格 |
| previewEnabled | 暂停本预览更新，不暂停效果、另一预览或设备输出 |
| maximumFps | 1–240，默认 60；按需刷新，合并过快通知。VSync 由窗口/后端统一管理 |
| showGrid/showAxes | 屏幕参考网格、随相机旋转的坐标轴指示 |
| backgroundImage | 图片 URL，置于灯点之后；空字符串关闭 |
| camera()/setCamera() | yaw/pitch/zoom/panX/panY 可序列化；presetView(0/1/2/3)=正面/等轴/右侧/俯视 |

显示中心按 `p'=(p-center)*displaySpacing` 计算，相机拟合采用变换前半径，避免间距因子被自动适配抵消。
例如固定 6 px 灯点、中心距离 10 px 时，X 因子 2 得到 20 px 中心距离，点宽仍为 6 px。
二维矩阵间隙以当前点尺寸为基准：方片/立方体 X/Y 填充为 `size*(1-gapX/Y)`；
Circle/Sphere 使用 `size*min(1-gapX,1-gapY)` 保持圆形。间隙不推测 Custom 网格。
场景单位尺寸随缩放变化；固定像素尺寸保持近似固定的立体中心截面大小，立体前后表面仍有真实透视。

Square/Circle 使用两个三角形的朝向相机方片；Cube 为 12 个三角形；Sphere 固定 12 纬×24 经网格，
每球 576 个三角形。四种形状均一次实例化 draw，开启实际深度测试/写入；球体并非渐变圆片。
高点数立体网格成本明显高于 Circle，不会自动把用户选择换回圆片。

二维左拖平移，三维左拖旋转；中键平移、滚轮缩放、双击适配。右键或“视图”按钮打开设置。
`LedPreviewSettings` 先编辑草稿，应用才改变预览；取消不写入，默认值只替换草稿。
`LedPreview.settings()/applySettings()` 保存/校验外观和相机。三个实例完全独立；Gallery 可显式从 A 同步外观。

预览限帧使用单调时钟的绝对节拍，保留 60 FPS 的约 16.67 ms 周期，不把每帧固定取整为 17 ms。
迟到的定时器不会持续累积漂移；空闲或阻塞后跳过过期时隙，不补画历史帧。
没有新数据或显示变更时不持续绘制；调整限帧和重新启用时重排等待，避免沿用旧的低帧率延迟。
实际帧率仍受效果生产速度、窗口刷新和 GPU 成本限制。

## 效果、Storage 与 Deck

Provider 提供算法，Storage 是集合，Place 是预设配置，Deck 是运行实例。
`QCatLedEffectProvider` 提供 `id()/effects()/create()`；每个 `QCatLedEffectInstance::render()`
接收布局、参数、Schema 和效果时间，返回连续通道字节。render 在工作线程调用，算法应有界、按给定时间
确定性生成，不访问 GUI。用 `registry.registerProvider(shared_ptr)` 注册；重复 Provider ID 被拒绝。
内置 `qcat.waves`（rainbow/ripple）和 `qcat.channels`（solid/chase）演示两个独立提供者。
`solid` 可编辑所有当前发光通道；`chase` 的 channel 从 0 起，可选择 W/CW/WW 做真实通道动画。

描述包含 id/version/name/schemas/reverse 和参数列表 `{id,name,default,min,max}`。
当前控件提供无限时长的播放，不展示未实现的有限时长、定位或循环控件。
内置 Provider 支持反向时间；C++ 允许 -8…8，基础 UI 采用 0…8。
第三方 Provider 不声明 reverse 时拒绝负速度。旧 RGBA 算法用 `QCatLightingFrame::fromRgba8()`
显式拆出 RGB 和 opacity，绝不能把四字节帧按长度猜成 RGBW。

Place 保存稳定 id、name/description、providerId、effectId/effectVersion、parameters、schema、
conversion、brightness、speed、thumbnail、configVersion。独立 W/CW/WW 位于参数对象内。
数量最多 128 个 Storage、每集合 4096 槽位、仓库合计 65536 槽位。

| Deck 方法 | 行为 |
| --- | --- |
| browseStorage | 仅浏览，不改变活动预设 |
| loadPlace | 校验并准备，成功显示 t=0，保持暂停 |
| activatePlace | 校验并准备，成功从头播放；点击可用槽位默认此动作 |
| play / pause | 继续 / 冻结效果时间，保持当前帧 |
| stop | 时间归零、全部发光通道置零，保留已加载配置 |
| unload | 停止并清除已加载配置 |
| restart | 时间归零并播放 |
| previous / next | 活动预设所属集合的可用槽位，边界不循环，忽略正在浏览的其它集合 |
| editPlace / setParameter | 编辑本 Deck 的工作副本；不自动保存 |
| savePlace / saveTo | 显式写入当前或另一可写 Repository 的指定槽位 |

空槽位、缺失算法、错误版本或参数无效均保留原活动预设并报告错误。加载中的选择与已经活动的选择分开。
活动身份包含 `activeRepositoryId/activeStorage/activePlace`；切换数据源只是浏览，ID 相同的外部副本也不会被误标为当前活动槽位。
A/B 可载入同一 Place，仍有独立实例和时钟；保存或清空仓库不会修改另一 Deck 的已加载快照。

时间增量为 `真实经过时间 × Place.speed × Deck.speed`，暂停期间不计入，速度 0 不推进。
亮度按 `效果值 × Place.brightness × Deck.brightness` 一次量化（四舍五入）后送入 Mixer。
Master 在混合后再作用一次。相机、预览亮度或混合比例不推进效果时间。

仓库提供 `createStorage/savePlace/renamePlace/copyPlace/clearPlace`，以及 `document/replaceDocument`。
数据库/服务适配器可派生 Repository 并覆写 `persist(candidate,error)`，先保存成功才提交内存状态；
异步拉取后在对象线程调用 replaceDocument。只读仓库禁止修改，但可以通过 saveTo 另存到其它仓库。

`exportJson(storageId?,placeId?)` 区分 repository/storage/place，外层 `{version:1,kind,data}`。
`importJson()` 原子校验后追加集合；集合 ID 冲突则生成新的集合和槽位 ID；Place 导入明确替换目标槽位，
保留目标槽位稳定 ID。未知 Provider 配置可保存供修复，不能播放。
`saveFile/loadFile` 只接受本地文件 URL，写入使用 QSaveFile，仓库 JSON 上限 16 MiB。

## 颜色、Mixer 和设备分流

```text
Storage → Provider 独立实例 → A/B LightingFrame
              → 通道混合 → 可冻结混合帧 → Master / Blackout → 最终 LightingFrame
                    ├─ PreviewColorAdapter → RGBA8 → LedSource → LedPreview
                    └─ PatchMapper → DeviceChannelFrame → OutputBackend
```

帧 Description 含 projectId、layoutRevision、pointCount、frameId、instanceId、timestampUsec、
schema、encoding、stride。当前只支持 ChannelValues8，stride 必须等于 3/4/5，无逐点填充；
opacity 是单独的可选 N 字节缓冲。构造器验证身份、长度、stride 和容量。

发光通道是设备级 0…255 值，**按通道值插值，不做隐藏的 sRGB 线性化**。
Mix 0 等于 A，1 等于 B，0.5 的 255/0 混合用 qRound 得到 128。
混合前按明确 workingSchema 和 conversion 统一，失败保持上一有效输出。
透明度在混合/设备映射前按黑色背景合成：`round(value*alpha/255)`，A 永不作为 W 发送。

Freeze 保持全部混合通道，Master/Blackout 继续有效；布局或工作 Schema 改变会解除 Freeze。
Blackout、Master=0、stop 清零当前全部通道。预览亮度完全不进入最终帧。

| Conversion | 行为 |
| --- | --- |
| Strict | 只接受同模型；通道重排由 Profile 处理 |
| AddWhiteZero | 同模型，或 RGB 扩为 RGBW/五通道且白通道置零 |
| ApproximateWhite | 显式允许下述白通道近似转换，不提取 RGB 中的白光 |

参考白颜色 `ref(K)` 在 2700K 的 `(1,.55,.2)` 与 6500K 的 `(.75,.85,1)` 间线性插值，
范围外钳位。预览将 `RGB + W*ref(singleK)` 或 `RGB + CW*ref(coldK) + WW*ref(warmK)`
逐分量加和并截断到 255，再保留独立 alpha。默认单白 4000K、冷白 6500K、暖白 2700K；
这是通道域技术预览，**不是色度校准或物理光混合**。

输出 ApproximateWhite：五通道→RGB 使用同样加和；五通道→RGBW 使用 `W=min(255,CW+WW)`；
RGBW→五通道按 `t=clamp((singleK-warmK)/(coldK-warmK),0,1)`，
`CW=round(W*t)`、`WW=W-CW`。转换不可逆，不宣称恢复原始白通道。全管线保持同 Schema 时原始白通道不经 RGB 合成。

Profile 独立定义 id/deviceId/portId/schema/channelOrder、三种参考色温、conversion，以及
firstPoint/pointCount/startChannel。逻辑索引从 0 起，地址从 1 起。最多 256 Profile、64 端口、
每端口 65536 字节；相同设备/端口的地址重叠被拒绝。颜色语义与物理排列分开，例如 RGB 的 GRB 排列。
同项目可把不同点组映射到三种模型。端口帧从地址 1 开始，未映射地址补零，包含项目、布局、映射和帧版本。

`QCatOutputBackend` 是纯 C++ 接口。`submit(frame,cancelled,error)` 在工作线程接收完整不可变帧，
实现必须在提交前检查 cancellation，并让 cancel 非阻塞。`LedOutput` 的 interval、repeatFrames、
enabled 控制发送周期、重复保持和启用；布局/映射修改取消旧事务。
`MemoryOutputBackend` 线程安全地保存最新完整帧，250ms 更新诊断，UI 只显示每端口前 32 字节。
后台接口和内存验证不需要窗口、LedPreview 或 GPU 读回。

## 保存与恢复

各模型有版本化配置入口；`LedProject.exportJson/importJson/saveFile/loadFile` 保存布局、仓库、Deck
工作副本、Mixer、Profile/Patch 和预览偏好（上限 32 MiB）。Gallery 把三路 settings() 写入 previewPreferences。
项目导入先校验布局、提供者/参数、仓库、映射及预览范围；无效数据不覆盖当前状态。
Project 保存其自有仓库和当前工作快照；Gallery 的额外 JSON 工作仓库可独立导出。恢复外部来源的工作快照时，
只有重新注入相同 repositoryId 的数据源才重新连接导航，不把其它仓库的同名 ID 当成原来源。
恢复时 Deck 暂停于 t=0，Freeze/Blackout 关闭；不保存高频帧、瞬时时间或自动写盘相机拖动。

旧 Source API、默认 Circle、屏幕 pointSize 和 previewBrightness 语义保留。旧接口没有项目文件格式；
不猜测来源不明的 JSON 或把旧 RGBA alpha 迁移为白光。

## 与 MADRIX 的边界

参考 [MADRIX Storage Places](https://help.madrix.com/m5/html/madrix/hidd_handling_selection_storage_array.html)
理解浏览/活动区分，本组件允许 A/B 独立载入同一 Place；点击直接切换，不自动移动 Crossfader。
布局计数表示灯点，不是完整灯具。自有 JSON 不兼容 `.mstx/.mstz/.mspx/.mspz`，也不包含 MADRIX 的商业算法。
不实现完整灯具库、图层编辑器、Cue、DMX/Art-Net/sACN/USB/串口；内存接收成功不代表硬件收到数据。
