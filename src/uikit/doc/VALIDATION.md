# 验证范围与运行方法

[文档索引](README.md) · [历史执行记录](VALIDATION_HISTORY.md)

本文维护可重复执行的检查步骤。测试结果必须结合日期、平台和检查范围阅读；文档更新不表示产品测试已重新运行。逐次执行摘要保留在[历史记录](VALIDATION_HISTORY.md)。

## 最近结果摘要

| 范围 | 最近记录 | 结果与限制 |
| --- | --- | --- |
| macOS 日期时间轴与字体回归 | 2026-10-25，macOS arm64、Qt 6.10.2、Apple Clang 21、Debug／Release | 两配置全量构建、库／Gallery lint、CTest 各 19/19 组通过；原有 Wayland 专属跳过保留。Metal 矢量两配置各 7/7、Release 图表像素 192/192；未验证 Windows／Linux，详见历史记录 |
| 文档与旧接口清理 | 2026-10-25，Windows、Qt 6.12.0、MSVC Debug | 全量构建、lint；版本／QML／Gallery 启动／矢量契约 4/4，D3D11 7/7；旧 QML 与适配别名已移除 |
| 矢量 UI 与自有 QRhi | 2026-10-25，Windows、Qt 6.12.0、MSVC Debug | 官方 ThorVG v1.1.3，全量构建和 lint、矢量契约 1/1、D3D11 7/7；未覆盖其他平台与全部格式特性 |
| ThorVG v1.1.3 Meson 接入 | 2026-10-25，Windows、MSVC Release | 含 QRhi 的独立静态库运行时版本、画布创建、SVG CPU 像素检查 1/1；CPU-only 使用相同官方版本 |
| Qt 套件安装与双配置 | 2026-10-25，Windows、Qt 6.12.0、MSVC 2022 Debug／Release | Qt6 组件发现、真实套件默认 QML 路径、两配置各两个消费者及独立模式通过；构建、库／Gallery lint、CTest 18/18 通过，可选 GPU 未开启 |
| C/C++ 模块目录整理 | 2026-10-25，Windows、Qt 6.12.0、MSVC 2022 Release | 构建、库／Gallery lint、CTest 18/18、独立安装消费者及文档检查通过；本次未开启可选 GPU 测试，详见历史记录 |
| Windows 全库测试可靠性 | 2026-10-24，Windows 11、Qt 6.12.0、MSVC 2022 Release | 最终 CTest 22/22（含四组 GPU），并行度 2，耗时 122.41 秒；LED 连续三次各 10/10。构建、库／Gallery lint 通过。测试运行器的故意失败与启动失败反向验证均正确返回非零；未新增跳过，原有平台专属跳过仍保留。详见历史记录 |
| Wayland Dock 平台分支 | 2026-10-22，Ubuntu 26.04、Qt 6.12.0 Release | 构建、库／Gallery lint、CTest 18/18 通过；Wayland 适配器 5/5、QML 状态 6/6。XCB 原有浮动坐标失败在未修改基线也复现；真实鼠标拖放和 Windows/macOS 本次未验收，详见历史记录 |
| Dock、全库回归 | 2026-10-21，Windows 11、Qt 6.12.0、MSVC 2022 Release | Dock 控制器 16/16、工作区 12/12、浮动 25/25、Gallery Dock 15/15；构建及库／Gallery qmllint 通过。全库 CTest 为 15/17，仍有 LED 与下拉框宽度失败 |
| Windows 旧基线失败 | 同上；保留历史对照 | `led_pipeline` 的 `layoutMappingAndValidation` 在 `m.applyNow(c)` 失败；`qml_contracts` 的 `SelectionControls::test_compactComboDisplayFits` 三种密度失败。后续测试可靠性改进与复跑结果见历史记录，不将旧结果当作当前结果 |
| Dock 安装与兼容 | 同上 | 独立安装消费者可调用 C++／QML 的 `dockToWorkspace`；保留原 1384 个导出符号，新增 1 个；旧测试程序加载新库 10/10 通过 |
| Linux 全库回归 | 2026-10-20，Ubuntu 26.04、Qt 6.12.0 Release | 当时 CTest 17/17、浮动 software/offscreen 与 XCB/OpenGL 各 8/8；早于最新跨窗口与外边缘修改，不作为最新版本的 Linux 全量验收 |
| Linux 图表 GPU | 2026-10-19，同一 Linux／Qt 环境 | AMD Radeon 780M／RADV、Mesa 26.1.6；Vulkan DPR 1／2 二维／三维／PNG 192／22／7，OpenGL 二维／三维 192／22；未开启 Khronos validation layer |

计数包含测试初始化／清理。Dock 的 Windows 原生窗口测试使用 QtTest 合成鼠标与 software 渲染；多屏混合 DPI、实际热插拔和 Wayland 仍需单独验收。不同日期、平台的通过结果不能互相替代；最低 Qt 6.8、其他 GPU／驱动、真机触屏和设备丢失也没有由这些结果获得通用验证。

## macOS ThorVG 编译兼容检查

使用匹配的 macOS Qt 套件配置 Release 构建，保留默认开启的 ThorVG。修改 QRhi overlay 后直接执行 `cmake --build <build> --parallel`，确认 CMake 自动刷新依赖源码指纹，并编译 ThorVG、主库、Gallery 和测试程序。然后执行 `cmake --build <build> --target QCatUiKit_qmllint qcat-gallery_qmllint` 及 `ctest --test-dir <build> --output-on-failure -j 2`。重点确认 Apple arm64 的 Qt 头文件不再出现 `__yield` 未声明，以及嵌套 `Uniform` 的默认参数错误。运行时断言失败须单独记录；编译和离屏测试不能替代 Metal 像素验证。

日期时间轴定向回归可在对应构建目录执行 `QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software tests/auto/charts/tst_charts reversedAxisMapping dateTimeNavigationKeepsPlot`，检查反向映射、三种时间格式、左右轴及两种字号的缩放锚点和平移距离，不放宽原有坐标阈值。QML 测试初始化须选择字体数据库中真实存在的字体，保留 `failOnWarning` 检查。

真实 macOS 图形会话中，从临时输出目录运行对应构建产物，使用 `env -u QT_QUICK_BACKEND QT_QPA_PLATFORM=cocoa QSG_RHI_BACKEND=metal QSG_INFO=1 QCAT_VECTOR_GPU=1 <build>/tests/auto/vector/tst_vector` 验证矢量像素与播放；运行 `<build>/tests/auto/charts/tst_chartrender` 时使用相同图形环境但不需要 `QCAT_VECTOR_GPU`。检查日志实际设备与后端；`no screens available` 表示未完成图形测试，不能记为通过。Debug 使用独立构建目录重复构建、lint 和 CTest。

## 源码目录调整检查

Qt Creator 构建目录若出现 Ninja 未知规则或类型描述缺失，按 [构建排错](BUILDING.md#qt-creator-的-ninja-规则与类型信息) 重新生成同一目录，再执行全量构建和库／Gallery qmllint。等构建进程结束后，运行 `ninja -C <build> -t recompact` 检查生成图可解析；确认两个模块的 .qmltypes 文件存在且非空，库文件包含新公开类型。最后运行 `ctest --test-dir <build> -R '^(gallery_smoke|qml_contracts|vector_contracts)$' --output-on-failure` 检查启动和类型创建，不把该选择性检查写成全部 GPU 测试通过。

移动 C/C++ 实现后，重新配置 CMake，执行下节的全量构建、库／Gallery qmllint 和 CTest。再按 [安装包检查](BUILDING.md#检查安装包) 安装到独立目录并构建消费者，确认公开头和转发头仍可使用。运行 `python tools/check_docs.py`，确认模块内公开头仍纳入文档覆盖检查。目录职责及新增模块约定见 [维护入口](MAINTENANCE.md)。

## 自动测试入口

先按 [构建说明](BUILDING.md) 配置并构建：

```sh
cmake --build build/release --parallel
cmake --build build/release --target QCatUiKit_qmllint qcat-gallery_qmllint
ctest --test-dir build/release --output-on-failure
```

多配置生成器使用相同的 `--config Release` 与 `ctest -C Release`。关闭 Gallery 时不会注册 Gallery 相关测试。

所有测试通过 `qcat_add_test` 注册，默认超时为 300 秒；各组已有的更短超时继续生效。GPU 渲染组共享 `qcat_gpu_window` 资源锁，因此可以用 `ctest --test-dir build/release --output-on-failure -j 2` 并行运行整套测试，而不会同时启动多个 GPU 测试窗口。资源锁只协调同一次 CTest 调用；不要同时从多个终端运行 GPU 组。

Windows CTest 自动补充构建库与 Qt DLL 的运行路径，并默认把系统字体目录传给 offscreen 插件。可以通过 CMake 的 `QCATUIKIT_TEST_FONT_DIR` 指定其他字体目录；Linux 等已有字体数据库的平台无需设置。QML 与 Gallery 测试启动时检查字体数据库，缺少字体直接报错，不用缺字方框的宽度判断布局。直接运行离屏测试程序时，需自行设置 `QT_QPA_FONTDIR`（Windows）。

QtTest 各组使用独立文本报告，由 CMake 运行器把报告转发到 CTest；进程失败、崩溃或没有报告都会使 CTest 失败。日志在对应构建子目录的 `<CTest 名称>-<配置>.txt`，完整聚合输出见构建目录的 `Testing/Temporary/LastTest.log`。每次启动先清空该组旧报告，避免将旧结果当作本次执行证据。直接运行测试程序不经过运行器，可用 `-o <log>,txt` 保存日志。

| CTest 名称 | 主要覆盖 |
| --- | --- |
| models | 主题 Token、TableModel／ChatModel 数据与通知协议 |
| dock_controller | Dock 分割、标签、浮动／回停、工作区外边缘、布局格式和失败时保持布局不变 |
| dock_native | Wayland 平台隔离；原生平台直接运行时还验证缩略图像素与窗口局部拖放事件。CTest 离屏运行跳过这两项原生检查 |
| tilt_controller | 四元数、校准、输入映射、授权、超时与事件循环延迟 |
| charts_3d | 三维数据、相机、材质、拾取和切片 |
| charts_extended | 气泡、格网、等值线、OHLC、箱线和雷达契约 |
| chart_statistics | 堆叠面积、直方图、拟合和瀑布图 |
| chart_export | PNG 尺寸、透明度、错误与生命周期，默认软件后端 |
| chart_render / chart_3d_render / chart_export_render（可选） | 二维／三维／导出真实 RHI 回归；需 `QCATUIKIT_TEST_CHART_GPU=ON` |
| charts | 原始数据、容量、批次校验、坐标范围、采样、交互及对象生命周期 |
| led_source | RGBA8 长度、布局/帧版本、坐标校验、最新快照和跨线程通知合并 |
| led_pipeline | 布局点序/溢出、不可变多通道帧、独立 Deck 时钟和快照、Storage JSON、冻结/全黑、Patch/慢后端取消与无窗口输出 |
| led_render（可选） | 真实 GPU 三路像素、深度遮挡、颜色/混合、相机输入、DPR 和资源重建；需 `QCATUIKIT_TEST_LED_GPU=ON` |
| version | C++ 与 QML 的实际库版本、渠道和 ABI 信息 |
| qml_contracts | 类型创建、输入、状态、主题、布局、图表与密度 |
| qml_busy_timing | BusyIndicator 实际帧时序、可见性和减少动效 |
| gallery_smoke | Gallery 启动与加载 |
| gallery_contracts | 组件页面、示例创建、双语、导航、密度、数据操作与停止行为 |
| gallery_chart_data | 基础演示的百万源数据保留及概览 |
| gallery_data_demo | 十万行表格、一万条消息／提交历史、容量与持续操作 |

选择专项检查：

```sh
ctest --test-dir build/release -R '^gallery_data_demo$' --output-on-failure
ctest --test-dir build/release -R '^charts$' --output-on-failure
```

普通图形契约的 CTest 配置会设置离屏及软件后端；可选的 GPU 组需要真实图形窗口，不能沿用 software 环境。直接运行 Qt Quick 软件契约测试时，需要自行设置 `QT_QPA_PLATFORM=offscreen`、`QT_QUICK_BACKEND=software` 并传入构建目录的 QML 导入路径。

真实后端测试可在配置时开启 `QCATUIKIT_TEST_CHART_GPU`／`QCATUIKIT_TEST_LED_GPU`，然后通过 `ctest --test-dir build/release -R '^(chart_render|chart_3d_render|chart_export_render|led_render)$' --output-on-failure` 运行。用 `QSG_RHI_BACKEND` 选择后端并核对日志；Linux 应使用实际图形会话的 XCB／Wayland 平台，而非强制 offscreen。平台命令和实际执行环境见 [历史记录](VALIDATION_HISTORY.md)。

## 数据量与交互验证

图表数据测试实际为基础演示保留 1,000,000 条原始记录，检查概览数量、容量覆盖、删除、缩小容量和恢复；趋势演示另有拟合端点。还检查非整除分组、求和／均值、正负分离与曲线极值。页面测试验证完整整数、百万预设、反走样及持续运行后的停止。

表格测试实际保留 100,000 行，对话和输入提交历史分别保留 10,000 条，检查单元格／状态编辑、淘汰策略和统计。真实页面持续填充到约定容量后检查桌面及窄屏布局。消息输入测试经过实际 submit() 路径，覆盖空白拦截和草稿恢复。

其他相关交互包括图表在可滚动父容器中的拖动、密度切换时搜索文字／选区及图标位置、关闭对话尾部跟随后保持历史位置、离开页面和隐藏窗口停止测试。

滚动专项覆盖三种密度下的鼠标拖动与滚轮、横向滚动、滚动条拖动、模拟触摸、文字选取和多选下拉框。Gallery 还验证页面及两种导航布局的滚动、内层表格／对话独立滚动、长按后拖动、单击选行、列宽调整和图表平移。对应入口为 `tests/auto/qml/data/tst_scrolling.qml` 与 `tests/auto/gallery/data/tst_scrolling.qml`。

上述结果验证数据保留与交互行为。Gallery 的最慢批次和操作频率不包含后续完整绘制，不能当作 GPU 帧率或网络性能。

## Dock 专项检查

控制器可通过 CTest 单独运行：

```sh
ctest --test-dir build/release -R '^dock_controller$' --output-on-failure
```

工作区与浮动专项使用构建目录中的 `tst_qml`，分别传入 `-input tests/auto/qml/data/tst_dock.qml` 或 `-input tests/auto/qml/data/tst_dockfloating.qml`，并加上 `-import build/release/qml`。Gallery 专项用 `tst_gallery`，输入为 `tests/auto/gallery/data/tst_dock.qml`。程序的具体位置随构建布局而定。

离屏检查设置 `QT_QPA_PLATFORM=offscreen` 和 `QT_QUICK_BACKEND=software`；Windows 原生窗口复查改为 `QT_QPA_PLATFORM=windows`。多配置构建使用同一配置的测试程序与 QML 模块。

| 操作 | 应看到的结果 |
| --- | --- |
| 拖动主工作区标签到外部后松开 | 出现浮动窗口，保留原内容；外层 Gallery 页面不随标签拖动滚动 |
| 在主窗口与浮动窗口之间、两个浮动窗口之间拖动标签 | 预览跟随鼠标，松开后合并；源窗口没有剩余面板时销毁 |
| 拖动中按 Esc，或取消鼠标抓取 | 原布局和窗口几何保持不变，随动预览消失 |
| 浮动属性面板，再拖到工作区右侧外边缘 | 属性列贯穿整个主布局高度，日志只在左侧；其他三个外边缘同理 |
| 拖到编辑器组内的右方向格 | 仅拆分编辑器组，日志布局不被扩展成侧列 |
| 点击回停按钮，或保存后恢复布局 | 编辑器／日志组合与属性整列位置保持正确；旧版 v2 没有 returnNode 仍可恢复 |
| 鼠标点击分隔条后移出，再用键盘调整 | 鼠标高亮消失，键盘焦点提示与方向键调整仍有效 |
| 禁止移动／浮动，或锁定布局 | 对应用户操作不可执行，已有内容不变 |

外边缘专项为 Gallery `tst_dock.qml` 的 `test_floatPropertiesBackToWorkspaceEdge` 与 `test_dockBackPreservesPropertiesFullHeight`；分隔条专项为 QML `tst_dock.qml` 的 `test_splitterMouseExitAndKeyboardFocus`。浮动专项还检查内容／焦点／弹层、权限、关闭请求、输入穿透预览和生命周期。

原生平台手工验收时，在并排窗口之间拖动，检查缩略图经过窗口间空白区域仍可见、取消后布局不变。在不同 DPI 的显示器之间重复操作；Wayland 的全局坐标与窗口定位能力需单独确认。浮动窗口目前仅支持标签合并，不应显示内部拆分能力，控制器对其分割请求应拒绝且不改变布局。

在实际 Wayland 会话中直接运行适配器与 QML 状态专项，避免 CTest 的 offscreen 环境覆盖：

```sh
QT_QPA_PLATFORM=wayland QT_QUICK_BACKEND=software \
  ./build/release/tests/auto/docknative/tst_docknative
QT_QPA_PLATFORM=wayland QT_QUICK_BACKEND=software \
  ./build/release/tests/auto/qml/tst_qml \
  -input tests/auto/qml/data/tst_dockwayland.qml -import build/release/qml
```

适配器日志应显示实际 QPA 及顶层拖动协议是否可用；缩略图测试检查捕获到面板像素，事件测试注入窗口局部拖放事件。QML 测试检查主窗口坐标转换、浮窗合并、取消恢复和移动权限。它们没有提供真实输入序列，不能代替原生 `QDrag::exec()`、`xdg-toplevel-drag` 和 `startSystemMove()` 的完整手工拖动验收。

手工分别拖动浮窗内部标题和标签，确认鼠标旁确有内容缩略图；测试拖入主窗／另一个浮窗、移出所有窗口、释放与 Esc。支持顶层拖动协议时检查空白处释放保留浮窗、取消恢复原布局；降级路径的操作差异见 [Dock 原生 Wayland](DOCKING.md#原生-wayland)。XCB／XWayland 仍运行原有浮动专项，并检查 `dock_native` 日志为 `QPA: xcb`、协议支持为 false；有原生坐标失败时使用同机未修改基线作对照。Windows/macOS 继续各自的原生窗口回归。

## 视觉检查

检查与改动相关的亮暗模式、三种密度、宽窄布局、焦点、禁用、长文本及空数据。可用 Gallery 保存当前窗口：

```sh
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software \
  ./build/release/examples/uikit/gallery/qcat-gallery \
  --language zh_CN --dark --density mobile --width 420 --page table \
  --screenshot /tmp/qcat-table-mobile.png
```

截图参数保存当前可见区域，并不会自动运行压力测试或展开所有状态。需要检查大数据、滚动、弹出层或输入时，先执行相应交互；自动截图可关闭动画以获得稳定结果。

软件后端的二维图表采用图像回退；三维场景明确不可用。物理 GPU、Vulkan／Metal／OpenGL／Direct3D、触摸手势、平台窗口操作、输入法与屏幕阅读器应在目标设备另行验收。Qt 6.8 是声明的最低版本；最近完整回归记录使用 Qt 6.12.0，不等于完成了最低版本运行时验收。

## 安装与独立消费者

### 矢量 UI 与 QRhi

Gallery 的 `svg-image` 与 `lottie-animation` 页面分别使用 `Emotion_Joy.svg` 和 `CatGray_All_Actions.json`。构建 `qcat-gallery` 后，在真实图形会话运行 `<build>/examples/uikit/gallery/qcat-gallery --page svg-image` 与 `--page lottie-animation`，检查状态为 Ready；动画页检查播放、暂停及进度定位。完整动画约 57 MiB，需使用当前 64 MiB 加载上限的库。

按 [矢量专题](VECTOR.md) 开启本地实验后端，同时配置 `QCATUIKIT_TEST_VECTOR_GPU=ON`。运行 `ctest --test-dir <build> -C Release -R '^vector_' --output-on-failure`；Debug 使用对应配置或独立构建目录。`vector_contracts` 检查输入校验、QML 创建和 software 错误状态；`vector_render` 需要可暴露的真实窗口，检查 SVG 像素、适配／拉伸、尺寸变化、解码失败后恢复、Lottie 播放／暂停／定位／结束、路径和两种渐变、QML 嵌套数据及窗口迁移。

GPU 测试不使用 offscreen/software，且与其他 GPU 测试共用窗口锁。Windows 可用 `QSG_RHI_BACKEND=d3d11` 显式选择后端。测试中 QML 画布截图保存于测试工作目录的 `vector-qml-canvas.png`。窗口未暴露时会暂停播放时钟，尚未得到绘制机会时内容可能保持 Loading；不要以隐藏启动的 GUI 程序作为 GPU 运行验收。

安装启用后端的库到独立前缀，构建 `tests/auto/cmake/vector-consumer`，使用 `find_package(QCatUiKit)`，按同一配置编译。将安装库和 Qt 的 bin 加入 PATH 后直接运行 `vector-consumer`；程序验证三种公开头、C++ SVG 与 QML 画布的状态和像素，不依赖 ThorVG 头或导出目标。

Windows 增量构建还需检查 `ninja -C <build> -t deps src/uikit/CMakeFiles/QCatUiKit.dir/qcatuikit_qmltyperegistrations.cpp.obj` 的依赖数非零，且包含 `qcatvectoritem.h`。本项目会保留调用方的编译器 launcher，将实际 `/showIncludes` 输出统一为 UTF-8 和固定 ASCII 前缀；仅设置 VSLANG 在缺少英语资源时不足以保证依赖扫描正确。修复此前零依赖的构建目录后，应重编译旧对象并确认记录已恢复。

### ThorVG 构建验证示例

此工程单独检查 CMake 到 Meson 的接入，无需编译 Qt 库或 Gallery。准备 Ninja 和编译器环境后运行（Meson 自动准备）：

```sh
cmake -S tests/auto/cmake/thorvg-consumer -B build/thorvg-check -G "Ninja Multi-Config"
cmake --build build/thorvg-check --config Release
ctest --test-dir build/thorvg-check -C Release --output-on-failure
cmake --build build/thorvg-check --config Debug
ctest --test-dir build/thorvg-check -C Debug --output-on-failure
```

Meson 默认自动准备，无需加入 PATH；可用 `-DQCATUIKIT_MESON_EXECUTABLE=/path/to/meson` 覆盖。示例链接 `ThorVG::ThorVG`，检查运行库版本为 1.1.3，再解析 SVG、通过 CPU 光栅化并验证中心像素，覆盖下载校验、静态链接和渲染调用。下载缓存位置及默认选项见 [构建说明](BUILDING.md#qt-creator-自动配置与-thorvg)。该测试不属于常规 CTest，也不验证 GPU 或全部 SVG／Lottie 特性。

### Qt 套件安装

按 [构建说明](BUILDING.md#安装) 使用同一多配置构建目录，依次构建并安装 Release、Debug。在 Windows 检查 `bin/QCatUiKit.dll`、`bin/QCatUiKitd.dll`、两个链接库及 QML 目录中的两个插件都存在，CMake 包目录保留 `QCatUiKitTargets-release.cmake` 和 `QCatUiKitTargets-debug.cmake`。

安装到 Qt 套件后，仅指定该 Qt 前缀构建以下消费者：

```powershell
cmake -S tests/auto/cmake/qt-module-consumer -B build/qt-module-consumer `
  -G "Ninja Multi-Config" -DCMAKE_PREFIX_PATH="C:/Qt/6.x/msvc2022_64"
cmake --build build/qt-module-consumer --config Release
cmake --build build/qt-module-consumer --config Debug
```

该工程使用 `find_package(Qt6 COMPONENTS Quick QCatUiKit)`。`qt-module-consumer` 验证公开头、C++ 调用及普通 QML 模块的导入扫描；`qml-plugin-consumer` 仅链接 Qt Quick，验证从磁盘加载插件。清除 `QML_IMPORT_PATH`／`QML2_IMPORT_PATH`，仅将所用 Qt 的 `bin` 加入 PATH，两种配置分别运行两个程序，检查退出码均为 0。程序不调用 `addImportPath`。离屏运行可设置 `QT_QPA_PLATFORM=offscreen` 和 `QT_QUICK_BACKEND=software`；这不构成 GPU 验收。

安装到暂存前缀时，Qt6 组件搜索还需设置 `QT_ADDITIONAL_PACKAGES_PREFIX_PATH=<stage>`，运行时使用 `<stage>/bin` 和 `<stage>/<QML目录>`。暂存检查不能替代清除这些设置后的真实 Qt 套件检查。另以 `QCATUIKIT_INSTALL_LAYOUT=STANDALONE` 检查旧的 `<libdir>/qml` 布局和下列独立包入口。

### 独立包入口

现有消费者工程包括 `consumer`、`led-consumer`、`charts-consumer`、`attitude-consumer`、`dock-consumer`、`version_consumer`。它们也可直接使用安装到 Qt 套件中的独立包入口；Windows 使用同一 Qt 前缀及 Ninja Multi-Config，各构建 Debug／Release，并将该 Qt 的 bin 加入 PATH。离屏运行设置 `QT_QPA_PLATFORM=offscreen`、`QT_QUICK_BACKEND=software`、`QT_QPA_FONTDIR=%WINDIR%/Fonts`。`charts-consumer` 的参数为安装的 QML 根目录；`version-consumer` 的参数为预期版本字符串及渠道；其余无须参数。LED 消费者验证对象、数据和 QML 创建，不创建真实 GPU 窗口。

安装后的检查不依赖源码头文件路径。普通消费者见 [构建说明](BUILDING.md)。图表消费者：

```sh
cmake --install build/release --prefix "$PWD/stage"
cmake -S tests/auto/cmake/charts-consumer -B build/charts-consumer -GNinja \
  -DCMAKE_PREFIX_PATH="$PWD/stage;/path/to/Qt/6.x/gcc_64" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/charts-consumer
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software \
  LD_LIBRARY_PATH="$PWD/stage/lib:/path/to/Qt/6.x/gcc_64/lib" \
  ./build/charts-consumer/charts-consumer "$PWD/stage/qml"
```

按实际安装布局替换 lib 和 qml；`STANDALONE` 的 QML 根目录为 `<libdir>/qml`。Windows 将项目与 Qt 的 bin 目录加入 PATH。版本消费者的构建和运行参数见 [版本管理](VERSIONING.md)，已编译程序的库替换检查见 [ABI](ABI.md)。

安装消费者不属于常规 CTest 组；更改导出头、依赖、安装路径或 ABI 时单独运行，并记录结果。

Dock 公开 API 的独立消费者使用同一安装流程：

```sh
cmake --install build/release --prefix stage
cmake -S tests/auto/cmake/dock-consumer -B build/dock-consumer -GNinja \
  -DCMAKE_PREFIX_PATH="/absolute/stage;/path/to/Qt/6.x" -DCMAKE_BUILD_TYPE=Release
cmake --build build/dock-consumer
```

按上面的平台说明配置运行时 DLL／共享库路径，运行 `build/dock-consumer/dock-consumer`（Windows 为 `.exe`）。消费者验证三种公开头、`dockToWorkspace` 的 C++／QML 调用及安装后的 QML 模块；不依赖源码导入路径。

## 文档与示例检查

从仓库根目录运行 `python tools/check_docs.py`，检查本地 Markdown／HTML 资源链接、标题锚点、代码围栏、当前指南中的重复正文和 Cat 组件目录覆盖。它还从模块 CMake 的公开头列表提取 `Q_PROPERTY`／`Q_INVOKABLE` 名称，检查对应专题是否提到这些名称。历史验证记录允许不同日期重复描述同一结果，不参与重复正文拦截。

这只是结构和名称覆盖检查，不证明默认值、签名、线程要求或行为正确，也不访问外网。新增公开子系统时，同步维护脚本中的专题映射。

全库文档复查按以下顺序进行：

1. 检查根目录 README／贡献约定、文档目录、Gallery README 和技能参考，列出覆盖范围；历史记录只核对结构、链接与保留证据，不能把旧结果当作新测试。
2. 对照 CMake 的安装头／类型列表、公开头、QML 组件和相关实现，检查类型名、主要属性、方法、默认值、错误与所有权；用测试确认容易误读的行为。
3. 从当前指南提取 QML 对象示例，在 Qt Quick Test 中加载。没有 import 的对象片段补标准导入，多个并列对象放入 Item；属性赋值和事件处理器片段须放到说明的上下文，不当作独立文件加载。
4. 运行重复正文与名称覆盖检查，再检查差异。示例加载只能证明语法与创建，交互、几何、GPU 和部署另行验证；当前文档检查不要求把所有历史平台重新测一遍。

更新 Dock 文档时，对照 `qcatdockcontroller.h` 的 `Q_INVOKABLE`、`Q_PROPERTY` 和 signals，以及两个 CatDock QML 文件的应用接口，逐项检查名称、默认值、返回值和权限。内部状态属性不应列为应用接口。把 DOCKING.md 的最小 QML 示例放入临时 Qt Quick 测试的 Component，以 `createTemporaryObject` 创建；等待面板注册后，确认日志位于编辑器下方，再检查保存／关闭／恢复以及 `dockToWorkspace(..., Right, 0.75)` 的右侧面板和比例。使用上文的 `tst_qml -input ... -import ...` 方法运行，测试产物留在忽略的构建目录中。

修改技能后，检查 SKILL.md 的路由、参考链接和模板是否对应当前公开接口。模板构建与实际交互方法见 [技能构建参考](../../../skills/qcatuikit-ui/references/build-and-verify.md)。文档截图的生成与人工复核见 [截图指南](SCREENSHOTS.md)。

## 结果记录原则

只写实际执行的配置和结果。容量验证、CPU 数据处理、渲染性能、长时间稳定性分别记录；不同设备和构建模式的耗时不能直接混用。文档、翻译或低影响修改按相关性选择检查，无需把不相关平台写为已通过。

## 全新 Qt Creator 自动依赖验证（Windows MSVC）

使用新的输出目录运行，已有目录不会被删除：

```powershell
python tools/validate_fresh_bootstrap.py `
  --qt-prefix C:/Qt/6.x/msvc2022_64 `
  --vsdevcmd "C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/Tools/VsDevCmd.bat" `
  --qtcreator-helper build/qtcreator-debug/.qtcreator/cmake-helper/qtcreator-project.cmake `
  --output build/fresh-qtcreator-validation --managed-python
```

脚本从当前源码生成不带依赖缓存的副本，路径含空格；使用 Qt Creator helper 和无控制台 MSVC 环境配置 Debug／Release，不传本机 ThorVG 或 Meson 路径。检查自动下载与独立 Python、完整构建、QML lint、qmltypes、Ninja recompact、矢量相关 CTest 和原生 D3D11 画布截图。随后修改中文目录中的真实头文件，确认增量构建的程序输出从 1 变为 2。每步日志及最终 report.json 保存在输出目录。测试需联网、Windows 图形会话以及可用的 Qt Creator helper；不覆盖 Linux/macOS 或交叉编译。

验证被中断时，可对同一输出目录追加 `--resume`，复用原副本和缓存，旧日志会改名保留。恢复运行不代表再次完成无缓存下载验证；首次配置日志保留初始证据。界面测试应串行运行，避免不同测试窗口互相影响输入和暴露状态。

适配目录或其 CMake 路径调整后，在匹配编译器环境执行 `cmake -S . -B <build>` 和 `cmake --build <build> --target qcat-thorvg`，检查覆盖目录定位、内容指纹及 Meson 构建。核对 `adapters/qcatthorvgqrhi/PROVENANCE.json` 中每个文件的 SHA-256，确认当前适配源码与记录一致；`original_files` 保留最初实验实现的文件哈希，`files` 表示当前维护版本。

调整适配内部命名后，重新构建 QCatUiKit 并运行 `vector_contracts`；在原生图形会话设置 `QCAT_VECTOR_GPU=1` 后运行 `tst_vector`，检查像素、播放和跨窗口释放。检查生成的 Ninja 重新配置依赖包含适配源文件，并确认只安装当前适配入口 `qcatthorvgqrhicanvas.h`。

### 固定版本与自有 QRhi 后端一起验证

在匹配的编译器环境中执行：

```powershell
cmake -S tests/auto/cmake/thorvg-consumer -B build/thorvg-qrhi-check -GNinja `
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/path/to/Qt/kit `
  -DQCATUIKIT_THORVG_TEST_QRHI=ON
cmake --build build/thorvg-qrhi-check --parallel
ctest --test-dir build/thorvg-qrhi-check --output-on-failure
```

该模式直接链接含自有 QRhi 后端的静态库，检查运行时版本为 1.1.3、创建适配画布，并执行 SVG CPU 像素验证。再运行主项目原生 `tst_vector`，检查 QRhi 像素和播放，避免只修改版本字符串而未验证真实调用。不传该选项时，消费者仍验证 CPU-only 构建。

清理类型注册或适配入口后，运行 `ctest --test-dir <build> -R "^(version|qml_contracts|vector_contracts|gallery_smoke)$" --output-on-failure`，再运行原生矢量测试和两组 lint。重新生成 qmltypes 并核对公开类型；不要用旧安装目录或构建目录中残留的兼容文件判断当前接口。

## OpenSpec 基线检查

能力边界和证据入口见 [OpenSpec 覆盖索引](../../../openspec/README.md)。修改基线时对照所涉源码、现有测试和接口文档，只记录已实现行为；不把现有测试文件或历史记录当作本轮执行结果。

```sh
openspec list --specs
openspec validate --specs --strict --no-interactive
python tools/check_docs.py
git diff --check
```

检查 Purpose、Dependencies、Requirements、Data Model、Known Deviations / Technical Debt、Non-Goals、Assumptions 的顺序，及每个 Scenario 的 WHEN / THEN。结构校验不替代行为测试；涉及实现的后续变更仍按本页对应模块执行验证。
