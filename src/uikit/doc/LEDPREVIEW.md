# LED 预览 / LedPreview

[文档索引](README.md) · [Gallery](../../../examples/uikit/gallery/README.md)

`LedPreview` 使用 QQuickRhiItem、QRhi 和 GPU instancing。每个灯点是一个朝向相机的四边形，片元着色器在圆外 discard，启用深度测试和深度写入。每个控件每次渲染只提交一次实例化 draw。Deck A、Output、Deck B 使用相同控件和独立数据源。需要硬件 Qt Quick 后端，不支持 `QT_QUICK_BACKEND=software`。

## 构建 / Build

仓库要求 CMake 3.24+、C++17、Qt 6.8+（QQuickRhiItem 从 6.7 提供）。需要 Core、Gui、Qml、Quick、QuickControls2、ShaderTools，以及和 Qt 版本匹配的 GuiPrivate 开发头。Gallery 需要 LinguistTools，测试需要 Test、QuickTest。着色器通过 `qt_add_shaders` 构建为 `.qsb` 并打入库资源，不需要运行时 glsl 文件。

```sh
cmake -S . -B build/led -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/path/to/Qt
cmake --build build/led --parallel
cmake --build build/led --target QCatUiKit_qmllint qcat-gallery_qmllint
ctest --test-dir build/led --output-on-failure
```

Windows 请从 x64 MSVC 开发终端执行，并使用相同 ABI 的 Qt 包。程序位于 `build/led/bin`，将 Qt 的 `bin` 放入 PATH。Linux 程序位于 `build/led/examples/uikit/gallery`，运行时库路径见 [BUILDING.md](BUILDING.md)。安装消费者链接 `QCatUiKit::QCatUiKit`，导入 `QCatUiKit`。GuiPrivate 只在库实现中使用。QRhi 的兼容范围有限，应使用构建时的同版 Qt 部署。

在 Windows 离屏测试环境中，如 Qt 报缺少 `lib/fonts`，可设置 `QT_QPA_FONTDIR=C:/Windows/Fonts` 后运行 CTest。生产运行使用系统字体，通常无需该设置。

实际实现对照了 [Qt 6.12 QQuickRhiItemRenderer](https://doc.qt.io/qt-6.12/qquickrhiitemrenderer.html)、[QRhi](https://doc.qt.io/qt-6.12/qrhi.html)、[QRhiCommandBuffer](https://doc.qt.io/qt-6.12/qrhicommandbuffer.html)，以及本机 Qt 6.12.0 的 `qquickrhiitem.h` 和 `rhi/qrhi.h`。最低版本的生命周期规则参考 [Qt 6.8 文档](https://doc.qt.io/qt-6.8/qquickrhiitemrenderer.html)。最终运行验证版本与结果见本文末尾。

## QML 接口 / QML API

```qml
import QtQuick
import QCatUiKit

LedPreview {
    width: 640; height: 360
    source: LedSource { id: leds }
    viewMode: LedPreviewRenderItem.Spatial3D
    pointSize: 6
    previewBrightness: 1
    backgroundColor: "#090d15"
    showStatistics: true
}
```

| 接口 | 含义 |
| --- | --- |
| `source` | `QCatLedSource*`，QML 类型为 `LedSource`。可以由多个预览共享；控件不取得所有权。 |
| `viewMode` | `LedPreviewRenderItem.Plane2D` 为 XY 正交视图；`Spatial3D` 为透视轨道视图，默认 3D。切换时重置相机。 |
| `pointSize` | 灯点直径，范围 1–128 **QML 逻辑像素**，默认 6。普通窗口下物理直径为 `pointSize * effectiveDevicePixelRatio()`；固定 backing texture 时按 texture/控件尺寸比转换。 |
| `previewBrightness` | RGB 预览倍率，0–1，默认 1；不修改数据源或 alpha。 |
| `backgroundColor` | 清屏颜色，可含透明度；输出使用预乘 alpha。默认深色。 |
| `showStatistics` | 显示灯点数、颜色提交 Hz、控件渲染 FPS、后端、GPU 名称、backing 分辨率和 DPR。默认 true。 |
| `resetView()` | 居中、清除旋转和平移、恢复缩放，按当前宽高比和灯点直径自动容纳整个布局。 |
| `orbit(dx,dy)` / `pan(dx,dy)` / `zoom(steps)` | 程序化相机操作；位移以逻辑像素计，zoom 每步乘 1.15。2D 忽略 orbit。 |
| `renderedFrames` / `renderedFrameId` | 已提交 LED 渲染 pass 数和最后绘制的源帧 ID；空布局或失败为 ID 0。 |
| `renderFps` / `colorUpdateHz` | 最近约 500ms 的控件 pass 频率和源接受帧频率。没有新帧或交互时无需持续重绘。 |
| `renderingInfo` / `renderError` | 实际 QRhi 环境信息或资源/后端错误；未启动 RHI 时信息为空。 |

一个 MouseArea 处理左键轨道旋转、中键平移、滚轮缩放和双击重置。预览内开始的拖动由预览保持鼠标抓取，嵌入 Flickable 或 CatScrollView 时不会被页面滚动抢走；预览外仍可拖动页面滚动。所有 LED 不创建独立 QML 对象或事件对象。布局替换自动重新适配相机；空、单点、重合坐标使用有效的默认半径。2D 保留实际 Z 深度，实现叠放遮挡。

`LedPreview` 是公共 QML 控件，`QCatLedPreview` 是公共 C++ QQuickRhiItem 类（`<QCatUiKit/LedPreview>`）；其 QML 基类名 `LedPreviewRenderItem` 也提供枚举。直接创建 C++ 类时，相机方法和统计属性可用，鼠标区域和统计文字由 `LedPreview.qml` 提供。

## C++ 数据格式 / Source API

```cpp
#include <QCatUiKit/LedSource>

QCatLedSource source; // 在 GUI 线程创建，生命周期须长于生产线程
QVector<QCatLedSource::Position> xyz = {{0, 0, 0}, {1, 0, 0}};
const quint64 layoutId = source.setLayout(xyz);
QByteArray rgba = QByteArray::fromHex("ff0000ff00ff00ff"); // 红、绿
if (!source.submitFrame(rgba, 1, layoutId))
    qWarning() << source.lastError();
```

`Position` 为三个连续 float（XYZ），12 字节。坐标限制为有限值、绝对值不超过 1e12，每源最多 1,000 万点。`setLayout` 深拷贝坐标，成功返回新的非零布局 ID；无效布局返回 0，保留之前的布局。空布局合法。新的布局先产生透明黑色帧，frameId 重置为 0。

`submitFrame` 输入连续 RGBA8 字节，每点严格 4 字节，和机器整数端序无关。方法深拷贝颜色，因此提交返回后调用方可重用自己的缓冲。frameId 必须非零，在同一布局内严格递增。颜色长度不符、旧 frameId、旧 layoutId 或提交期间布局被替换都会返回 false，并在 `lastError` 中给出原因。第三个参数为 0 表示提交给当前布局；多线程或异步管线应始终传入 setLayout 返回的 ID，避免同数量新布局接收到旧颜色。

`pointCount`、`layoutId`、`frameId`、`lastError` 为低频 QML 可观察属性，使用合并后的 `dataChanged` 信号。`submittedFrames()` 累计成功接受的帧数。XYZ 和实时颜色只通过 C++ 接口提交，不提供逐点 QObject、QML 属性或 QVariantList 通道。

## 线程与 GPU 生命周期 / Threading

- **生产线程**：调用线程安全的 `setLayout` / `submitFrame`。Gallery 在独立 QThread 的事件循环中使用 `Qt::PreciseTimer` 调度彩虹、波纹，以及 `round(A * (1-mix) + B * mix)` 的 RGBA8 逐通道线性混合；这不是 sRGB 色彩空间线性化。默认每 16ms 请求生成一帧，实际频率由生成耗时和系统调度决定。暂停且配置未变化时只检查状态，不重新生成颜色；定时器超时不会累积生成作业。
- **GUI 线程**：配置控件和相机。源只保留最新完整不可变快照，GUI 通知最多一个待处理事件。源析构前必须停止并 join 生产者；Gallery 析构设置停止状态、退出工作事件循环并 wait。不得和源析构并发调用任何成员。
- **渲染线程**：`synchronize()` 在 GUI 阻塞期间复制配置和 shared_ptr 快照，短暂取源锁。`render()` 只读不可变快照，无源锁；持有的旧帧不会被生产者修改。落后帧直接被覆盖，不积累队列。

静态 XYZ buffer 仅在布局变化或 GPU 重建时上传；动态 RGBA8 buffer 仅在完整帧变化时更新。四顶点 buffer、96 字节 uniform（含布局局部原点）、SRB、pipeline 和实例 buffers 都复用，容量不足时只在布局变更时扩容。着色器先减去布局中心，再应用相机矩阵，避免大坐标重合布局损失相机距离精度。固定 backing texture 时分别转换 X/Y 直径，屏幕上仍为逻辑像素圆形。没有每帧 GPU 对象创建、每点变换矩阵或 GPU 图像读回；测试截图是验证手段，正常显示不读回。

使用 `clipSpaceCorrMatrix()` 修正后端投影，使用 QQuickRhiItem 自动管理的颜色与深度目标。尺寸变化更新视口；render pass 格式或采样数变化重新创建兼容 pipeline。QRhi 指针变化释放所有本地 GPU 资源，重新上传布局和颜色。renderer 析构在渲染线程释放资源；场景图失效、跨窗口 reparent、关闭与重新创建都由节点生命周期驱动，不跨 QRhi 共享 GPU 对象。

相机和已同步的布局版本保留在 GUI 控件中；场景图释放并重建时保持相机，只有更换布局/数据源、切换视图或显式 reset 才恢复默认视角。

半透明圆点采用预乘 alpha 混合，同时写深度；这符合灯具遮挡要求，但不提供半透明物体的排序合成。alpha 为 0 的圆点 discard，不遮挡其它灯点。默认全不透明演示帧使用直接无光照颜色。

## Gallery 和压测 / Demo and benchmark

```sh
qcat-gallery --page led-preview --dark --language zh_CN
qcat-gallery --page led-preview --dark --led-count 10000 --led-layout matrix --led-benchmark 10 --led-report matrix-10k.json
qcat-gallery --page led-preview --dark --led-count 100000 --led-layout sphere --led-benchmark 10 --led-report sphere-100k.json
```

三路同时生成、渲染，默认每路 4,096 点。页面含暂停、2D/3D 视图、矩阵/球形布局、数量选择、重置和销毁重建入口；窄屏纵向排列三路。基准先暖机 2 秒，再测量指定秒数，报告每路颜色更新 Hz、LED pass FPS、窗口 frameSwapped 频率、实际 GPU/后端/像素尺寸。frameSwapped 不是显示器实际刷新完成或 GPU 耗时。`--led-interval 4` 可提高生产频率观察丢旧帧行为。目标为目标机器争取 60 FPS，任何容量预设均不保证该速率。

`tst_ledsource` 验证长度、帧版本、布局版本、非法坐标、空布局，以及 GUI 停滞时 10,000 帧的合并通知。`tst_ledrender` 是真实窗口 GPU 测试，默认构建但不加入离屏 CTest；以 `-DQCATUIKIT_TEST_LED_GPU=ON` 注册，或直接运行。它验证三路图像、遮挡、颜色、亮度、相机、空/单点布局、尺寸/MSAA 变化、控件重建、跨窗口和场景图重建。软件后端测试不能证明 GPU 渲染成功。

安装包的独立消费者位于 `tests/auto/cmake/led-consumer`，验证实际头文件、转发头、导出类与 QML 模块：

```sh
cmake --install build/led --prefix stage/led
cmake -S tests/auto/cmake/led-consumer -B build/led-consumer -G Ninja -DCMAKE_PREFIX_PATH="/absolute/stage/led;/path/to/Qt"
cmake --build build/led-consumer
```

运行消费者时设置安装包 `bin` / `lib` 的动态库路径与 `QML_IMPORT_PATH=/absolute/stage/led/lib/qml`。无需将 GuiPrivate 作为消费者依赖。

## 实测记录 / Verification record

2026-10-12，Windows 11 x64，MSVC 19.39 Release，Qt 6.12.0，NVIDIA GeForce RTX 5060 Ti，D3D11。Gallery 窗口 1200×840，DPR 1，Deck A/B backing 为 281×320，Output 为 278×320 物理像素。三路同时运行，灯点直径 6 逻辑像素，透视视图，混合 50%，Output 亮度 100%，sampleCount 1。每组暖机 2 秒、测量约 10 秒：

| 布局 | 每路点数（总点数） | 颜色提交 Hz | LED pass FPS（A / Output / B） | window frameSwapped Hz |
| --- | --- | --- | --- | --- |
| 二维矩阵 | 10,000（30,000） | 62.5 | 59.2 / 59.2 / 59.2 | 59.2 |
| 二维矩阵 | 100,000（300,000） | 62.5 | 59.8 / 59.9 / 59.9 | 59.9 |
| 三维球形 | 10,000（30,000） | 62.4 | 59.8 / 59.8 / 59.8 | 59.9 |
| 三维球形 | 100,000（300,000） | 62.5 | 59.6 / 59.7 / 59.6 | 59.7 |

另以三路各 100,000 球形点、`--led-interval 4`、暖机 2 秒、测量约 5 秒，测得颜色提交 213.4 Hz、三路 LED pass 与 frameSwapped 均约 59.8 Hz。数据生产和渲染频率独立，GUI 停滞的合并通知测试还验证了不会积累颜色帧队列。测量未包括 GPU 时间戳或显示器实际扫描完成；这些短时结果不保证更大窗口、不同显卡或长期负载稳定达到 60 FPS。

实际完成：干净编译库/Gallery/测试、两组 QML lint、10 组 CTest（Windows 离屏测试指定系统字体目录）、安装并构建运行独立 LED 消费者。真实窗口测试分别在 D3D11、OpenGL 和 D3D11 DPR 2 上通过，覆盖三路颜色、圆形裁剪及像素大小、深度遮挡、帧内存隔离、混合 0/50/100%、Output 亮度、鼠标左/中键/滚轮/双击、2D 忽略旋转但保留平移缩放、空/单点/重合布局、1 万/10 万动态布局、resize/MSAA、控件销毁重建、跨窗口以及场景图释放重建。Gallery 的英/中文和窄屏页面创建由回归测试覆盖，宽屏/窄屏还检查实际截图。

原始 JSON、日志、截图为本地构建产物，未加入版本控制。源码保持 Qt 6.8 最低要求，但本次没有实编/运行 Qt 6.8，也未验证 Linux/macOS、Vulkan、Metal、D3D12。上述结果只代表列出的实际工具链与环境。
