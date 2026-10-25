# 矢量图、动画与画布

[文档索引](README.md) · [构建与接入](BUILDING.md) · [验证方法](VALIDATION.md)

`CatSvgImage` 显示 SVG；`CatLottieAnimation` 播放 Lottie JSON；`CatVectorCanvas` 绘制形状、路径和渐变。三者共享 `QCatUiKit::VectorItem`，使用 `import QCatUiKit`。C++ 公开头为 `QCatUiKit/VectorItem`、`QCatUiKit/VectorItem.h` 和 `QCatUiKit/qcatvectoritem.h`，不暴露 ThorVG 或 QRhi 私有类型。

## 构建与使用

默认启用 `QCATUIKIT_ENABLE_THORVG=ON`。CMake 自动下载官方 ThorVG `v1.1.3`，应用仓库内 QRhi 实现，并通过 Meson 构建；无需相邻 ThorVG 目录。QRhi 后端由本项目提供，在 v1.1.3 基础上编译，保留版本号 1.1.3。可用 `QCATUIKIT_THORVG_SOURCE_DIR` 覆盖为自维护源码。依赖目录、离线缓存和工具说明见 [构建与接入](BUILDING.md)。构建必须使用匹配的 Qt、编译器和运行库。

未启用支持时，组件仍可创建，backendAvailable 为 false；提供内容后报告 Error。Qt Quick software 后端不可运行 QRhi，也报告明确错误。空 SVG／Lottie 为 Null；没有可渲染窗口时内容可能保持 Loading。QRhi 后端为实验功能，无 CPU 回退。

下面的组件可直接放入 Qt Quick 页面：

```qml
import QtQuick
import QCatUiKit

CatSvgImage {
    width: 320; height: 200
    source: "illustration.svg"
    fillMode: VectorItem.PreserveAspectFit
}
```

```qml
import QtQuick
import QCatUiKit

CatLottieAnimation {
    id: animation
    width: 320; height: 200
    source: "animation.json"
    loops: 1
    onFinished: console.log("Finished")
}
```

页面按钮可调用 animation.play()、pause()、stop()、restart()，或写入 progress 定位。autoPlay 只决定后续加载是否自动开始；CatLottieAnimation 默认 true，其他组件为 false。animationEnabled 绑定主题 reducedMotion；隐藏或窗口未暴露时冻结时钟，不补放隐藏时间。

## 属性与方法

| 接口 | 行为 |
| --- | --- |
| `source: url` | 本地文件、qrc 或相对 QML 上下文 URL，不下载网络资源 |
| `sourceData: bytearray` | 内存 SVG／JSON；非空时优先于 source |
| `format` | Svg、Lottie、Canvas；各 Cat 组件设置默认格式 |
| `fillMode` | Stretch、PreserveAspectFit（默认）、PreserveAspectCrop，居中适配并裁剪 |
| `viewBox: rect` | Canvas 坐标范围，默认 (0,0,100,100)，宽高须正且有限 |
| `shapes: list` | Canvas 形状快照，按列表顺序绘制；替换列表触发重建 |
| `status`、`errorString` | Null／Loading／Ready／Error 及错误原因，渲染线程成功记录绘制命令后进入 Ready，不表示 GPU 已完成执行 |
| `backendAvailable` | 库是否编译 QRhi 后端，不保证当前设备一定可用 |
| `sourceSize`、`contentRect` | 文档尺寸与逻辑像素内容矩形；加载后更新隐式尺寸 |
| `duration`、`frameCount` | Lottie 时长（秒）及帧数；其他格式为 0 |
| `progress`、`currentFrame` | 可写 0..1 进度及只读帧位置，末帧为 frameCount−1 |
| `autoPlay`、`animationEnabled` | 加载后自动播放及是否允许时钟推进 |
| `playing`、`paused` | 播放请求状态；Loading 时 play 可预约，Ready 后开始；实际推进受可见性和 animationEnabled 控制 |
| `loops`、`playbackRate` | 默认 −1（无限），正整数为总循环次数；速率 (0,16]，默认 1 |
| `renderedFrames` | 成功记录的绘制次数，不是屏幕帧率、实际呈现次数或 GPU 完成时间 |
| `reload()` | 重新读取并解析内容，清零进度及循环状态 |
| `play()`、`pause()` | 播放／恢复及暂停 Lottie |
| `stop()`、`restart()` | 停止并归零；归零后重新播放 |
| `appendShape(map)`、`clearShapes()` | 校验并追加（失败返回 false，列表不变）；清空画布 |
| `finished()` | 有限循环完成时发出一次，进度留在 1 |

属性发送对应 Changed 信号。statusChanged 同时通知错误，metadataChanged 通知源尺寸、时长和帧数，playbackChanged 通知播放状态及设置。进度截断至 [0,1]，NaN 和无效 viewBox、速率、循环值被忽略。整表赋值 shapes 中的无效内容报告 Error，修正后可恢复。

## 声明式画布

```qml
import QtQuick
import QCatUiKit

CatVectorCanvas {
    width: 320; height: 200
    viewBox: Qt.rect(0, 0, 160, 100)
    shapes: [
        {type: "rectangle", x: 8, y: 8, width: 144, height: 84, radius: 18,
         fill: {type: "linear", x1: 0, y1: 0, x2: 160, y2: 100,
                stops: [{offset: 0, color: "#3b82f6"}, {offset: 1, color: "#a855f7"}]}},
        {type: "path", commands: [["M", 45, 50], ["L", 70, 75], ["C", 90, 50, 110, 40, 120, 25]],
         fill: "transparent", stroke: "white", strokeWidth: 7}
    ]
}
```

rectangle 使用 x/y/width/height 和圆角 radius；ellipse 使用同样矩形指定外接框。path 的 commands 支持 M(x,y)、L(x,y)、C(x1,y1,x2,y2,x,y)、Z，必须从 M 开始。坐标使用 viewBox 单位，形状和渐变数值须有限且绝对值不超过 10⁷。最多 4096 个形状，每条路径最多 65536 条命令。

通用样式：fill（默认黑色）、stroke（默认无）、strokeWidth（默认 1）、opacity（0..1，默认 1）、fillRule（nonzero 或 evenodd）。颜色可为 Qt color 或颜色字符串。fill、stroke 均支持上述 linear 渐变，或 `{type: "radial", cx: 80, cy: 50, radius: 40, stops: [...]}`。stops 需 2..256 项，offset／color 中的 offset 按非递减顺序处于 [0,1]。

## 线程与边界

Gallery 提供 svg-image、lottie-animation、vector-canvas 三个页面，包含缩放方式、播放／暂停、进度定位、重新加载及添加形状的操作。Gallery 的示例资源只用于演示，应用应提供自己的 SVG／JSON。

源码位于 src/uikit/vector；公开类使用 Pimpl。文件在 GUI 线程读取，单份数据上限 16 MiB；解析、ThorVG 对象和 QRhi 资源只在渲染线程使用与销毁。带版本号的线程安全快照回传状态，过期加载结果不覆盖新内容。

本地 SVG 的外部资源以源文件目录解析；qrc／sourceData 的图片请嵌入 SVG，Qt 资源路径不自动映射到普通文件。不提供脚本执行、网络下载、SVG 编辑器或全部 SVG／Lottie 特性的等价承诺。

渲染使用宿主的 QRhi、命令缓冲及目标，不自行提交帧，也不在 active render pass 内调用 ThorVG。设备迁移时重建 canvas，先释放 canvas 对 paint 的引用，再销毁 Animation，最后释放同线程初始化引用。GPU 和平台覆盖见验证记录，加载成功不等于特性、性能或稳定性已全面验收。

CPU 负责格式解析、动画求值及路径／描边三角化；当前适配使用 QPainterPath、QPainterPathStroker 和 qTriangulate。QRhi GPU 负责最终绘制、着色、混合及效果；选择 GPU 后端不会把前述 CPU 工作自动迁移到 GPU。
