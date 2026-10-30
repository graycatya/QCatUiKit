# 验证范围与运行方法

[文档索引](README.md)

本文维护可重复执行的检查步骤。测试结果必须结合日期、平台和检查范围阅读；文档更新不表示产品测试已重新运行。具体变更的验证结果记录在对应 OpenSpec change 中并随变更归档，注明命令、日期、平台、结果及未覆盖范围。

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

真实后端测试可在配置时开启 `QCATUIKIT_TEST_CHART_GPU`／`QCATUIKIT_TEST_LED_GPU`，然后通过 `ctest --test-dir build/release -R '^(chart_render|chart_3d_render|chart_export_render|led_render)$' --output-on-failure` 运行。用 `QSG_RHI_BACKEND` 选择后端并核对日志；Linux 应使用实际图形会话的 XCB／Wayland 平台，而非强制 offscreen。实际执行环境、后端与结果记录在对应 OpenSpec change 中。

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

1. 检查根目录 README／贡献约定、文档目录、Gallery README 和技能参考，列出覆盖范围；OpenSpec 归档只核对结构、链接与保留证据，不能把旧结果当作新测试。
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

## README 专题截图

使用 [截图指南](SCREENSHOTS.md) 的 native 模式补充三维图表、飞机姿态、体感、Dock、LED 和三种矢量页面。构建 `tst_gallery` 后在可暴露窗口的图形会话执行：

```sh
QT_QPA_PLATFORM=xcb python3 tools/capture_gallery_docs.py \
  --build-dir build-release --config Release --mode native \
  --output-dir /tmp/qcat-native-doc-images
```

先检查八张图片的实际曲面、仪表、校准状态、面板布局、三路灯点和矢量 Ready 状态，再发布至文档图片目录。场景不会拼接，长页面只捕获当前可视区域；不以图片声明完整交互或物理设备验收。默认 software 模式继续用于原有九张基础截图。

## Mascot 桌面能力探针

探针是独立 Qt Quick 工具，不依赖 Gallery 或尚未交付的 Mascot 公共 API。在仓库根目录执行，使用与库一致的 Qt 6.8+ kit（通过本机 `CMAKE_PREFIX_PATH` 选择）：

```sh
cmake -S tests/manual/mascot-probe -B /tmp/qcat-mascot-probe -G Ninja
cmake --build /tmp/qcat-mascot-probe --parallel
ctest --test-dir /tmp/qcat-mascot-probe --output-on-failure
/tmp/qcat-mascot-probe/qcat-mascot-probe
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software /tmp/qcat-mascot-probe/qcat-mascot-probe --json
```

Windows 将可执行文件替换为相应配置目录下的 `.exe`。默认窗口显示诊断 JSON，场景图初始化后更新 `graphicsApi`（QSGRendererInterface::GraphicsApi 枚举值）；`--json` 只采集环境，不初始化场景图，报告 `NotInitialized`。`--self-test` 验证会话分类，由独立 CTest 调用。

### Linux 桌面窗口原型

使用本机 Qt prefix 独立构建，不修改既有 kit 缓存。需要 Qt Quick 和 Widgets；可选 Xlib 开发包启用 xcb 原生读回，运行时的 libXtst 启用自动输入。没有 Xlib 时仍可构建和运行原生 Wayland 窗口，xcb 输入检查保持 Pending。

```sh
cmake -S tests/manual/mascot-probe -B build/mascot-linux-probe -G Ninja -DCMAKE_PREFIX_PATH="<qt-prefix>" -DCMAKE_BUILD_TYPE=Release
cmake --build build/mascot-linux-probe --parallel
ctest --test-dir build/mascot-linux-probe --output-on-failure
QT_QPA_PLATFORM=xcb build/mascot-linux-probe/qcat-mascot-probe --linux-check build/linux-xcb-report.json
QT_QPA_PLATFORM=wayland build/mascot-linux-probe/qcat-mascot-probe --linux-check build/linux-wayland-report.json
QT_QPA_PLATFORM=wayland build/mascot-linux-probe/qcat-mascot-probe --linux-prototype
```

两个后端必须串行运行，等待前一进程退出再启动后一进程；自动输入期间勿操作鼠标／键盘。自动报告：1 为失败或保存失败，2 为仍有 Pending；当前不自动宣告完整通过。报告和相邻 PNG 为本机临时证据，不提交进源码。QPA=xcb 且 declaredSession=wayland 时结果是 XWayland，不能填入原生 X11 平台验收。

xcb 自动检查覆盖背后独立进程归属、渲染 alpha、空白和椭圆外角点击去向、角色不抢焦点、背后键盘、角色／气泡整窗穿透、隐藏气泡后输入区域、气泡 Tab／空格、真实拖动及原生位置读回、置顶开关、隐藏和宿主恢复。自动检查启动时将诊断窗口最小化，并额外发送 40 个连续拖动点，核对最终原生位置和隐藏期间诊断报告刷新次数；事件间隔指标不等于画面帧率或屏幕呈现延迟。人工复测应比较诊断窗口正常显示与最小化后的拖动手感。XWayland 不一定允许桌面抓图；只有截图尺寸及独立蓝色背景参考有效时才比较透明空白与角色像素，否则桌面合成项 Pending。窗口自身 PNG 不代替桌面合成。

原生 Wayland 自动检查不发送 XTest；没有合成器配套时验证独立进程、渲染 alpha、服务诊断及隐藏恢复，定位和置顶明确不可用。兼容 GNOME 配套连接后额外检查定位、置顶关闭／恢复及原生快照。人工运行控制界面后，分别点击角色、椭圆外角、空白、气泡及开启穿透后的相同位置，保存独立进程计数；真实拖动、被动显示焦点、原应用继续输入和气泡键盘操作均须记录。观察角色／气泡间实际显示背后蓝色窗口而非实色矩形，独立记录透明视觉结论。

托盘人工验收：从真实托盘图标打开菜单，依次执行 hide、show、passthrough、exit request，并在控制界面 Save evidence。核对菜单计数、最后动作、窗口显示及穿透状态；exit 只增加 exitRequests，宿主继续运行。没有托盘服务时记录不可用，通过控制界面恢复显示，不能把服务查询或直接按钮调用计为菜单实测。Close probe 后确认独立目标、角色、控制窗口及托盘已清理。平台范围见 [平台原型说明](MASCOTPLATFORMS.md)。

### GNOME/Mutter 配套安装及验收

GNOME 配套及 Qt 原型仅面向 Shell 50 原生 Wayland 候选环境。手动探针增加同 kit 的 Qt DBus 依赖，不改变公共库依赖。以下构建／测试／打包均不安装或启用扩展：

```sh
cmake --build build/mascot-linux-probe --parallel
ctest --test-dir build/mascot-linux-probe --output-on-failure
cmake --build build/mascot-linux-probe --target mascot_probe_gnome_package
```

打包目标需要本机 gnome-extensions，输出 `build/mascot-linux-probe/gnome/mascot-probe@qcatuikit.local.shell-extension.zip`，只包含 metadata、extension、service 和 protocol 四份文件。GJS 与 dbus-run-session 可用时注册私有会话总线集成测试，Node 可用时注册纯协议测试；检查 CTest 列表，缺少工具时不能宣称对应测试已运行。受限沙箱若禁止创建本地总线套接字，需在允许本地套接字的环境运行测试，不把该失败解释成扩展失败。

仅在用户明确选择后，在当前用户会话安装和启用：

```sh
gnome-extensions install build/mascot-linux-probe/gnome/mascot-probe@qcatuikit.local.shell-extension.zip
gnome-extensions enable mascot-probe@qcatuikit.local
gnome-extensions info mascot-probe@qcatuikit.local
QT_QPA_PLATFORM=wayland build/mascot-linux-probe/qcat-mascot-probe --linux-check build/linux-wayland-companion.json
QT_QPA_PLATFORM=wayland build/mascot-linux-probe/qcat-mascot-probe --linux-prototype
```

扩展安装在用户的 GNOME extensions 目录，不需要 root。不自动覆盖已有同 UUID 安装；已有版本先核对来源、文件和当前验收证据。新装扩展如果当前 Shell 未发现，或更新 JS 后需重新载入，先保存工作，由用户自行注销并重新登录；Wayland 不能用重启 Shell 命令替代。不要自动注销或终止 Shell。具体重载约束见 [GNOME 调试文档](https://gjs.guide/extensions/development/debugging.html)。

诊断中检查 companion.state=Ready、protocol=1、version=0.1.0、shellVersion 为本机 50.x，服务 unique owner 与 org.gnome.Shell 相同；这只证明握手。使用 Reset position、Always on top 关闭／开启、Native snapshot，核对 lastCompanionRequest.applied 和 nativeWindow.rect／above，保存原生读回。自动入口仍保持整体 Pending，不把 Qt x/y 或窗口 flags 当成定位成功。原型坐标是全局逻辑坐标，当前不验收公共组件的多屏相对坐标及平滑移动。

保持原型运行时，执行 `gnome-extensions disable mascot-probe@qcatuikit.local`：检查状态变为 Disconnected、旧原生快照被移除、定位／置顶能力不可用，新请求明确失败；扩展应恢复其改变前的置顶状态。再执行 enable：状态重新 Ready，旧请求不得重放，用户重新点击定位或置顶后才执行。分别记录禁用、重新启用、操作结果、托盘服务状态及独立背后进程输入计数。无扩展时亦运行一次自动检查，保存 Missing 及不可用原因。版本不兼容和非 Shell 服务拒绝由私有总线测试覆盖；真实 Shell 升级后的不兼容仍需在对应系统验收，不通过修改真实扩展版本或关闭全局版本检查伪造通过。

结束后可禁用并卸载本次扩展：

```sh
gnome-extensions disable mascot-probe@qcatuikit.local
gnome-extensions uninstall mascot-probe@qcatuikit.local
```

卸载后再次确认客户端不可用，关闭原型清理测试进程与托盘。此配套不提供托盘服务，不能因窗口配套 Ready 就填托盘通过。权限、协议及恢复行为见 [平台说明](MASCOTPLATFORMS.md#gnomemutter-配套原型)。

### Windows 桌面窗口原型

范围及平台状态见 [平台原型说明](MASCOTPLATFORMS.md)。Windows 手动探针额外依赖同 kit 的 Qt Widgets（仅用于 QApplication、QSystemTrayIcon 和 QMenu，不为库新增依赖）。从已有 MSVC x64 开发环境和 Qt 运行环境执行：

```powershell
cmake -S tests/manual/mascot-probe -B build/mascot-probe -G Ninja -DCMAKE_PREFIX_PATH="<qt-prefix>"
cmake --build build/mascot-probe --parallel
ctest --test-dir build/mascot-probe --output-on-failure
& ./build/mascot-probe/qcat-mascot-probe.exe --windows-prototype
```

关闭交互原型，再在可交互且未锁定的桌面执行自动检查，期间不要操作鼠标或切换应用：

```powershell
& ./build/mascot-probe/qcat-mascot-probe.exe --windows-check ./build/windows-mascot-report.json
```

检查使用普通 Windows QPA 和原生 SendInput，退出码 0 表示本次原型检查全部通过；1 表示未通过或无法取得输入证据；2 表示当前平台／参数不支持。报告保存 prototypeAcceptance、始终 Pending 的完整 acceptance、系统／Qt／图形后端／屏幕、两个进程标识、逐项输入计数和前台 HWND。相邻 PNG 保存测试窗口合成及本程序托盘图标区域；需查看透明空白、角色区域、气泡按钮及图标。颜色比较使用同帧背后背景参考，不假定桌面色彩管理后的 RGB 固定不变。

真实输入检查覆盖空白到独立进程、角色接收且不激活、原应用继续键入、整窗穿透、气泡区域变化、拖动与原生位置读取、置顶开关、主动气泡焦点及 Tab／空格、隐藏／宿主恢复、托盘注册与隐藏区展开、原生 Context 和菜单键盘隐藏。穿透切换及退出请求的菜单回调另以 QAction 派发验证，不能将其称为所有菜单动作的鼠标实测。退出请求不结束宿主。多屏／混合 DPI／完整气泡等仍按完整矩阵另行验证。

常规 CTest 只运行会话分类和 headless 拒绝检查。后者先写入旧 Passed 证据，再以 offscreen/software 发起 Windows 检查，验证退出码 2、报告被替换为 Pending、能力仍 Unknown；不对正常 CTest 注入桌面输入。若沙箱拒绝 SendInput（例如错误 5），应在允许的交互桌面运行，不能改用 PostMessage 或 QtTest 合成点击后宣称真实穿透已通过。

分类以实际 QPA 为准：wayland 插件报告 NativeWayland；xcb 配合 Wayland 会话报告 XWayland；xcb 配合 X11 会话报告 X11；会话未知时不猜测是否使用 XWayland。offscreen/minimal/vnc 报告 Headless。能力初始 Unknown 且附原因，验收固定 Pending；环境采集不会证明透明、穿透、定位、焦点或托盘可用。此工具目前没有原生窗口交互测试装置，也未探测桌面配套。完整验收仍按变更中的矩阵执行并保存人工操作证据。

## 角色包与精灵播放

使用当前 CMakeCache 对应的 Qt kit 和构建目录，不使用无关工具链缓存。以下 `<build-dir>` 和 `<qt-prefix>` 为本机路径：

```sh
cmake -S . -B <build-dir> -DBUILD_TESTING=ON -DQCATUIKIT_BUILD_MANUAL_TESTS=ON
cmake --build <build-dir> --target tst_mascot qcat-mascot-player --parallel
ctest --test-dir <build-dir> -R '^mascot_contracts$' --output-on-failure
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software <build-dir>/tests/manual/mascot-player/qcat-mascot-player --action working --screenshot /tmp/mascot-working.png
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software <build-dir>/tests/manual/mascot-player/qcat-mascot-player --action thinking --dark --width 420 --height 520 --screenshot /tmp/mascot-thinking.png
cmake --install <build-dir> --prefix /tmp/qcat-mascot-install
cmake -S tests/manual/mascot-player -B /tmp/qcat-player-consumer -DCMAKE_PREFIX_PATH="/tmp/qcat-mascot-install;<qt-prefix>"
cmake --build /tmp/qcat-player-consumer --parallel
```

安装消费者运行时通过 `--import-path` 指向安装包的 QML 根目录，必要时按 BUILDING 设置平台动态库路径。Windows 使用实际配置子目录及 `.exe`，环境变量语法按 shell 调整。

`mascot_contracts` 检查本地/qrc 清单、CatGray 共享图集兼容、版本／数值／资源／路径／符号链接／帧范围拒绝、加载代次及销毁、FIFO／替换／忽略／队列上限和终态、暂停与隐藏恢复；实际渲染合成测试用可识别颜色验证跨行帧、镜像与缩放，并检查私有视觉在窗口隐藏和 reducedMotion 时停止时钟。离屏软件结果不证明 GPU、真实桌面输入或平台窗口能力。截图需人工检查角色轮廓、文字和窄屏布局；自动脚本成功不等于视觉审阅。

### Lottie 与角色交互

Mascot 的 GPU 测试为显式选项，避免在 software 环境误报可用。可配置 `QCATUIKIT_TEST_MASCOT_GPU=ON` 后运行 `mascot_render`，或在已有有效 QRhi 桌面直接运行测试可执行文件：

```sh
QCAT_MASCOT_GPU=1 <build-dir>/tests/auto/mascot/tst_mascot lottieRendering
<build-dir>/tests/manual/mascot-player/qcat-mascot-player --lottie --dark
```

按环境选择实际 QPA 和 QSG_RHI_BACKEND；不要强迫不支持 Vulkan 的 offscreen 插件创建 Vulkan 窗口。记录 rendererInterface 的实际图形 API、设备和 X11／XWayland／原生 Wayland 会话。该测试验证有效 QRhi 播放与反馈，不认证桌面悬浮能力。

测试覆盖 Lottie 的自然完成、暂停、隐藏恢复、拖拽反馈抢占、切包取消及旧反馈隔离。software 常规 `mascot_contracts` 检查错误和静态占位，不能替代 GPU 检查。另建禁用后端的构建目录，以 `QCATUIKIT_ENABLE_THORVG=OFF`、`QCATUIKIT_BUILD_THORVG=OFF`、`QCATUIKIT_BUILD_EXAMPLES=OFF` 配置并构建 tst_mascot，运行同名测试，验证关闭后端时也保留明确错误。

交互用例还检查反馈期间 InteractionBusy、有限动作作为无限拖拽反馈、无反馈拖拽、禁用／切包结束交互，以及矩形／椭圆／多边形、后声明区域优先、用途筛选、动作覆盖、镜像／缩放坐标、锚点和实际 Qt 点击信号。这里的区域命中不是原生窗口空白穿透证据。

### CatGray IP Lottie 实材验证

直接使用仓库内 CatGray 包，或按 [角色预览工具](../../../tests/manual/mascot-player/README.md) 生成角色包。使用支持 QRhi 的真实窗口环境执行（非 software）：

```sh
QT_QPA_PLATFORM=xcb QSG_RHI_BACKEND=vulkan QCAT_MASCOT_GPU=1 QCAT_CATGRAY_LOTTIE_PACKAGE=<生成包的绝对目录> <build-dir>/tests/auto/mascot/tst_mascot catgrayLottieRendering
<build-dir>/tests/manual/mascot-player/qcat-mascot-player --lottie --action working
```

测试验证真实包加载、两倍逻辑尺寸下的非空角色画面、播放进度、暂停恢复、全部十动作加载和播放、working 单次完成与返回 idle。该测试不替代桌面悬浮窗口的平台验收。常规测试未设置实材路径时跳过该项。启用 `QCATUIKIT_TEST_MASCOT_GPU=ON` 时，`mascot_render` 会执行仓库内完整 CatGray 包的播放契约、十动作渲染和异步场景切换测试。

### 大 Lottie 的异步加载与画面保留

常规 `mascot_contracts` 会读取实际大包，并验证校验期间 GUI 心跳仍运行、取消后不覆盖新包、加载期间销毁安全。`vector_contracts` 验证文件错误异步反馈、清空来源和读取期间销毁。

在有效 QRhi 桌面单独执行以下测试，避免同时运行其他图形窗口或耗时基准：

```sh
QCAT_MASCOT_GPU=1 <build-dir>/tests/auto/mascot/tst_mascot asynchronousLottieHandoff
```

该测试先显示 Canvas，再切换到实际 CatGray Lottie；采样加载期间的窗口，断言始终有可见内容，并记录 GUI 心跳最大间隔。还覆盖快速换源、解码期间销毁及 Null 清空。`catgrayLottieRendering` 覆盖完整包切换时的视觉保留，以及同一动作路径内容变化后的重新加载。启用 `QCATUIKIT_TEST_MASCOT_GPU` 后，`mascot_render` 包含上述场景。

可用 Qt 日志规则区分包校验、文件读取、Lottie CPU 准备、渲染场景挂接与首帧绘制命令耗时：

```sh
QT_LOGGING_RULES='qcatuikit.mascot.loading.debug=true;qcatuikit.vector.loading.debug=true' <build-dir>/tests/manual/mascot-player/qcat-mascot-player --lottie
```

两个类别默认不输出 debug 日志。首帧日志记录 CPU 侧绘制命令耗时，不是完整 GPU 执行时延。GUI 心跳数据用于记录指定环境的响应情况，不推导跨设备的固定帧时间承诺。加载总时长、峰值内存和素材减重仍需分别评估。

### 动作缓存与预热

在有效 QRhi 桌面执行 `tst_mascot actionCacheAndLatency`，设置 `QCAT_MASCOT_GPU=1`；该测试也注册在可选 `mascot_render` 中。Windows 可在 Qt kit 的运行环境中执行：

```powershell
$env:QCAT_MASCOT_GPU = '1'
& <build-dir>/bin/tst_mascot.exe actionCacheAndLatency
```

测试等待常用动作预热完成，验证预热命中、20 次往返切换复用场景且无重复 CPU 解析、非空画面、条目淘汰、清空切包失效及预热期间销毁，并输出冷包加载、预热动作和重复切换 P50/P95。冷包值包含完整包校验，不能直接当成相同边界的冷动作延迟；比较冷动作时查看 `action first rendered feedback ms`。不以固定 50 ms 断言作为跨设备测试门槛。

直接运行 `tst_mascot` 时，`lottieRendering` 根据实际 ThorVG 可用性和 Qt Quick 图形后端选择播放或错误契约；未设置 `QCAT_MASCOT_GPU` 不代表渲染不可用。该变量用于显式要求 GPU，并启用可选缓存及异步渲染测试；十动作实材检查另需 `QCAT_CATGRAY_LOTTIE_PACKAGE`。未启用时这些可选测试显示 SKIP 属于预期。CTest 的 `mascot_contracts` 显式使用 offscreen/software 验证错误反馈，`mascot_render` 则要求有效 QRhi 桌面。

诊断中 `resident` 表示复用已播放场景及画布，`prewarm` 表示接管预热任务（可能仍在运行），后者不能单凭命中日志认定预热已完成；应结合 `mascot prewarm finished` 和首帧反馈。内存测试应记录相同素材、分辨率、后端下冷启动及缓存稳定后的进程工作集／私有提交和峰值，不能把默认 150 MiB 文档准入预算当作实际内存上限。完整十动作、快速换源和同路径内容更新继续运行既有 `catgrayLottieRendering`、`asynchronousLottieHandoff`。

`cacheLimitConfiguration` 覆盖默认 150、实例隔离、重复设置、负值拒绝和 0 禁用。可选 GPU 测试 `configurableActionCache` 在默认预算下暂停播放并等待其余九动作预热，按固定排列跨十动作切换二十次，断言不淘汰且不重复 CPU 解码；随后降到 1 MiB 验证淘汰和超限动作按需加载，再设 0 验证禁用。`actionCacheAndLatency` 显式使用 40 MiB 总预算，保留预算不足时的淘汰测试；两个 GPU 测试均包含于 `mascot_render`。

## macOS 桌宠窗口探针

从现有 CMakeCache 取得 Qt prefix 和 Ninja 路径，独立配置以避免改变用户 kit：

```sh
cmake -S tests/manual/mascot-probe -B build/mascot-macos-probe -G Ninja -DCMAKE_PREFIX_PATH="<qt-prefix>" -DCMAKE_MAKE_PROGRAM="<ninja-path>" -DCMAKE_BUILD_TYPE=Release
cmake --build build/mascot-macos-probe --parallel
ctest --test-dir build/mascot-macos-probe --output-on-failure
build/mascot-macos-probe/qcat-mascot-probe.app/Contents/MacOS/qcat-mascot-probe --macos-prototype
open -n -W build/mascot-macos-probe/qcat-mascot-probe.app --args --macos-check "<absolute-report-path>"
```

先确认 Cocoa 会话存在真实屏幕。沙箱中的 no screens available 不算平台失败或验收通过。自动 CTest 验证会话分类和离屏拒绝实机检查，后者预写旧 Passed 再确认被 Pending 覆盖。检查返回 2 表示仍需实机证据，不能按成功处理。不要为运行测试自动修改隐私权限；系统事件权限缺失时，按平台说明手动操作并保留实际观察。

交互时先点击独立背后窗口，再显示角色，记录背后进程 keyWindow 和输入计数。分别点击角色矩形、两个区域之间的空白、气泡按钮；启用整窗穿透后重复角色和气泡点击，背后 mouse 增加而角色／气泡计数不增加。隐藏气泡后原区域点击也应抵达背后窗口。主动点击气泡后用 Tab／空格验证按钮，再拖动角色检查气泡同步。用菜单栏 QCat 项目检查隐藏、穿透、show 恢复和 exit 仅发宿主请求。save 保存当前计数，但不会自动把人工操作判定为通过；将步骤、前后计数、焦点和观察结果写入变更证据。关闭诊断窗口检查独立进程和菜单栏项目已清理。完整范围及未覆盖场景见 MASCOTPLATFORMS.md。

自动输入验收需用户明确授权的辅助功能权限。通过 LaunchServices（上述 open 命令）启动 app，使权限归属该探针；直接从终端启动二进制可能仍归属启动者。开发构建重新链接后，旧的临时权限身份可能失效；在用户已批准的验收范围内重新选择当前 app，并以报告 postEventAccess 为准，不以设置开关外观推断。验收结束恢复本次临时授权，普通窗口展示无需此权限。窗口穿透或显示状态变更后应等待 WindowServer 处理，再发送真实输入。

气泡按钮的主动操作允许激活探针应用；检查应先证明角色被动点击保持背后焦点、键盘仍到背后窗口，再证明主动气泡按钮可以接收 Tab 和空格。报告为每步保留角色／气泡及独立目标计数、气泡 keyWindow 和响应者类，不能用一次末态推断全部中间步骤。人工菜单验收还需保存 menuActions、lastMenuAction、exitRequests，并明确菜单栏真实操作与诊断窗口展开菜单的区别。


## 宿主桌宠窗口契约

Windows 公共区域输入：构建 tst_mascot 时会同时构建独立原生背后程序 mascot-input-target；两者须位于同一运行目录。在可交互 Windows 会话设置 QT_QPA_PLATFORM=windows、QCAT_MASCOT_DESKTOP=1，运行 `tst_mascot.exe windowsInputRouting`。默认离屏 CTest 会跳过该真实输入用例；沙箱无法访问桌面时不能把它记为通过。分别使用 QT_QUICK_BACKEND=software 和移除该变量、设置 QSG_RHI_BACKEND=d3d11 的路径检查，日志报告实际 graphics API。可用 QT_SCALE_FACTOR=1.25 验证分数 DPR 转换，但这不替代混合 DPI 硬件验收。

该测试使用 SendInput，只向本轮公共桌宠与独立背后进程注入输入；测试期间勿操作鼠标／键盘或切换窗口。背后测试窗口会临时置顶，随后将本轮桌宠放在其上，避免其他应用遮挡造成误投；每次鼠标按下前检查原生归属，不修改其他应用层级。结束回收子进程、输入窗口，恢复光标和此前前台窗口。检查默认阻挡、矩形／椭圆／多边形、镜像、缩放、动作覆盖、整窗穿透恢复、气泡及空格操作、被动焦点、真实拖动与隐藏／销毁。屏幕像素须与 Qt 画面参考一致，区域切换后继续保持；Qt 自身截图不代替屏幕采样。当前用例不覆盖系统触摸／笔、独占全屏或全部 Windows 平台矩阵。

`ctest --test-dir <build-dir> -R '^gallery_mascot_shutdown$' --output-on-failure` 运行独立 Gallery 进程，打开桌宠页并显示两个实例，再关闭主窗口；进程须在超时前正常退出。该回归通过 `--smoke-test --page mascot-pet` 触发实际主窗口关闭路径，检查辅助窗口不会阻止 Gallery 退出。桌宠自身关闭隐藏及宿主最小化独立性继续由 mascot_contracts 覆盖。

`tst_mascot independentWindowState` 验证两个公共窗口的控制器、动作队列及气泡隔离，切包与销毁不影响另一实例；检查导出快照字段、无效／缺失字段和不支持朝向的原子拒绝。离屏运行覆盖拒绝与隔离；使用 windows、cocoa 或 xcb QPA 再运行同一测试，覆盖失效屏幕标识回退及旧 v1 快照兼容。该测试不自动写入持久化存储，不代替真实拔屏或混合 DPI 验收。

使用当前配置的构建目录运行 `ctest --test-dir <build-dir> -R '^mascot_contracts$' --output-on-failure`。publicWindowLifecycle 检查公共 QML 类型加载、默认隐藏、关闭隐藏后角色状态保留及离屏定位明确失败；publicBubbleLifecycle 检查纯文本内容、隐藏冻结时限、超时、替换和过期标识拒绝。这些不验证原生点击穿透、桌面透明合成或焦点策略。

Gallery 的 mascot-pet 页面仅使用公共 CatMascotPet，提供两包、多个窗口、动作、气泡、移动和穿透操作；退出页面必须清理窗口。分别在原生 Wayland、xcb 及其他实际平台记录窗口、输入、焦点及宿主生命周期，不安装扩展。原生 Wayland 定位与置顶须返回受限原因，不能借助已安装的研究配套把公共组件标为支持。已有研究原型的扩展安装／禁用步骤仅适用于其独立手动程序。

安装公共窗口消费者：将库安装到独立临时前缀，然后以该前缀和同一 Qt kit 配置 `tests/auto/cmake/mascot-consumer`，构建并离屏运行 mascot-consumer。它只包含公开头、导入已安装模块，检查默认隐藏、控制器归属和关闭隐藏，不访问 Gallery 或私有视觉 URL。当前组件不提供托盘，托盘相关历史探针步骤仅适用于独立研究程序。

在真实桌面显式设置 `QCAT_MASCOT_DESKTOP=1` 并分别用 `QT_QPA_PLATFORM=wayland`／`xcb` 运行 `tst_mascot desktopWindowIntegration`。可设置 `QCAT_MASCOT_CAPTURE` 为 PNG 目标路径。检查定位限制、移动终态、无效状态拒绝、气泡尺寸持续有效，以及可定位环境中气泡前后角色原点相同、状态恢复与取消后无重复终态。PNG 只证明窗口渲染，不证明桌面透明合成或真实输入去向；原生 Wayland 必须保持精确定位不可用。


## 桌宠独立窗口与采样回归

mascot_contracts 的 publicWindowLifecycle 检查默认置顶、inputTransparent=false、inputRegionsEnabled=false、无托盘接口和无 transient parent。spriteSamplingStability 在 0.35、0.5、1 与 1.25 倍尺寸检查四个不透明帧的完整内部像素，验证无邻帧混入或透明裂缝；framesRendered 继续检查图集顺序、镜像、暂停和隐藏恢复。真实桌面需同时运行这两个像素测试，不能只以软件后端代替 GPU。

真实 xcb 下运行 desktopWindowIntegration，若安装 xprop 则读回 _NET_WM_STATE，检查 SKIP_TASKBAR 与 ABOVE；主动尝试绑定 transient parent 后应仍为空，最小化独立宿主窗口后桌宠应保持显示且非最小化，连续精灵帧期间窗口位置与尺寸保持不变。原生 Wayland 无法保证通用任务栏／置顶控制，记录限制；Gallery 的默认 xcb 选择不修改系统设置，并尊重显式 QPA。桌面面板中是否隐藏图标及播放主观流畅度还需实际观察。

Lottie 修改后运行既有原生 QRhi 的 lottieRendering、actionCacheAndLatency 等相关 GPU 检查，验证缓存命中、保留画面与动作终态；桌宠视觉树不应创建 CatBusyIndicator，普通 CatLottieAnimation 仍保留原有提示。

PNG 旧版包拖动回归：运行 `tst_mascot legacySpriteMouseDrag packages regionsAndAnchors`；在真实 xcb 桌面另运行 `QT_QPA_PLATFORM=xcb tst_mascot legacySpriteMouseDrag desktopWindowIntegration`。legacySpriteMouseDrag 使用 Gallery 同源旧版 PNG 包，通过合成鼠标按下、移动、松开事件验证默认 body 区域、拖动信号与 xcb 窗口位移；它不证明系统级真实鼠标输入路由。旧版包未声明 regions 时兼容生成全角色 body 区域，显式 regions（包括空数组）保持原义。

公共桌宠原生输入与移动回归：在 DPR=1 的真实 xcb 桌面设置 `QT_QPA_PLATFORM=xcb QCAT_MASCOT_DESKTOP=1`，运行 `tst_mascot desktopInputRouting desktopMovementInterruptions desktopBubbleEdges`。需要 X11 与 XTest 运行库；测试会暂时移动鼠标，结束恢复原位置。desktopInputRouting 用背后原生窗口计数验证默认阻挡、区域外穿透、镜像／缩放／动作覆盖、整窗穿透与恢复、气泡按钮及气泡关闭后的输入范围。desktopMovementInterruptions 验证新请求、定位、取消、隐藏、切包和真实拖动打断移动，拖动期间宿主位置请求被拒绝且无延迟重放，终态唯一；屏幕变化仅注入通知，不代表实际拔屏验收。desktopBubbleEdges 在明暗主题和四角显示长文字，检查窗口在可用区域内、显示／关闭气泡前后角色原点一致及原生画面可捕获；不代替窄屏、多屏和其他平台验收。

## 公共桌宠气泡键盘与 Gallery 可访问性

使用当前 CMakeCache 对应的构建。下列 QtTest 检查向本测试窗口投递事件，不申请辅助功能权限，不代表背后其他应用的真实系统输入验收：

```sh
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software <build>/tests/auto/mascot/tst_mascot publicBubbleKeyboardLifecycle
env -u QT_QUICK_BACKEND QT_QPA_PLATFORM=cocoa <build>/tests/auto/mascot/tst_mascot publicBubbleKeyboardLifecycle
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software <build>/tests/auto/gallery/tst_gallery -import <build>/qml GalleryMascot::test_accessibleStatusAndKeyboard GalleryMascot::test_hostControlsIndependentWindows
```

Cocoa 行需真实可用的 macOS 图形会话；离屏通过不能替代该行。用小型内置包验证操作按钮点击、空格触发、Shift+Tab 返回关闭按钮及空格关闭，确认关闭气泡／隐藏／整窗穿透后恢复不接受焦点标志。新气泡布局在原生渲染线程异步更新，测试应等待布局稳定再取按钮坐标。Gallery 数据行覆盖明暗主题、Desktop／Touch／Mobile 密度、680／400 逻辑像素宽度、中英运行时切换、键盘显示桌宠、状态和动作可访问名称；能力诊断应显示本地化标签并保留未知诊断原因。

视觉检查从 Gallery 生成桌宠页的桌面亮色及窄屏深色截图，检查按钮换行、页面滚动、文字裁切和绑定循环。角色动画软件后端纹理上限产生的警告须独立记录，不能将 Gallery 控件截图作为 GPU 角色播放证据。完整原生焦点验收另需独立背后应用验证显示／更新不抢输入以及主动气泡交互。

### 桌宠屏幕坐标与边界回归

`screenCoordinateRules` 随 mascot_contracts 离屏运行，使用负原点屏幕、窄可用区、非整数角色尺寸及气泡偏移验证逻辑坐标边界与缩放计算。坐标计算不乘屏幕 DPR；原生输入适配独立处理物理像素。

在 windows、cocoa 或 xcb 图形会话运行 `tst_mascot screenCoordinateRules desktopScreenCoordinates`。Windows 可执行文件通常位于 `<build>/bin/tst_mascot.exe`；其他布局以实际构建输出为准。desktopScreenCoordinates 检查带气泡定位到可用区两端、关闭气泡后角色位置稳定、重新开启约束时超大缩放被校正，以及注入 DPR 变化通知取消旧移动。可用 QCAT_MASCOT_GEOMETRY_CAPTURE 指定气泡窗口 PNG。该测试不改变系统显示配置，不注入其他应用输入；DPR 事件注入不是实际 DPI 切换，合成坐标不是多屏硬件验收。

实机仍需在混合 DPI 的多个屏幕间拖动、移动到负原点屏幕，并拔除当前屏幕，确认角色和输入区域对齐、有效缩放被校正且剩余屏幕内可恢复交互。分别记录各屏可用区、DPR、Qt／系统／图形后端和原生点击去向；未完成时保持对应平台及任务待验收。


## macOS 公共区域输入

在真实 Cocoa 图形会话构建 `tst_mascotmacosinput`；直接运行 `<build>/tests/auto/mascot/tst_mascotmacosinput.app/Contents/MacOS/tst_mascotmacosinput nativeLifecycle nativeEventDelivery nativeShapePlacement`，检查透明非激活面板、原生窗口销毁、同实例事件转换及椭圆角点系统命中。此路径无需辅助功能权限，不证明跨进程真实输入。`mascot_macos_reject_headless` 随 CTest 检查离屏运行返回 2 并覆盖旧报告为 Pending。

真实输入需用户临时授权该测试 app 的辅助功能权限，然后通过 `open -n -W <app> --args --native-input --report /tmp/mascot-macos-input.json -o /tmp/mascot-macos-input.txt,txt` 启动。测试启动独立的 mascot-macos-input-target 作为背后进程，发送前核对前台与点击窗口归属；执行时勿操作键鼠。检查报告 inputAcceptance=Passed、postEventAccess=true，且 routing 无跳过，覆盖矩形／椭圆／多边形、镜像、缩放、整窗穿透、气泡点击与空格、连续拖动、隐藏及销毁。acceptance 保持 Pending，表示完整平台矩阵尚未完成。结束须关闭该测试 app 的临时权限并确认两个进程退出。

若 LaunchServices 停在 dyld、未生成报告，不能算测试通过；直接启动时若 CGPreflightPostEventAccess=false，同样保持 Pending。可将两个 app 复制到临时目录，用当前 Qt kit 的 macdeployqt 部署：先部署 helper，再将其放在测试 app 的 Contents/Helpers 下，部署测试 app 时指定库构建目录的 -libpath、src/uikit/qml 的 -qmldir 和构建 qml 的 -qmlimport。必须等部署完成、`codesign --verify --deep --strict <app>` 成功后再运行；自包含部署仍不能保证系统许可可用，不扩大其他系统权限。多屏、实际混合 DPI、Spaces／全屏及其他系统版本另外验收。


### macOS 置顶与按需焦点

使用上节部署及授权流程运行 `tst_mascotmacosinput --native-input --report <json>`。routing 除区域输入外，还以独立背后应用真实按键计数检查显示／隐藏恢复、动作切换、气泡显示及增量更新均不抢焦点；关闭置顶须读回普通 AppKit 层级并让背后浮动窗口获得系统命中，恢复置顶须保留被动焦点。气泡主动点击后检查真实 Tab 到关闭按钮、Shift+Tab 返回操作按钮及空格关闭；分别检查隐藏、整窗穿透和失活后恢复拒绝焦点策略，再显示时背后应用仍接收按键。结果应包含 passive-show-action-bubble-update、always-on-top-toggle-without-activation、real-tab-backtab-and-keyboard-close、focus-release-hide／passthrough／deactivate 六组记录；任一失败或跳过均不能标记该轮完整通过。测试只激活本轮 helper，不切换或输入到用户的其他应用。完整屏幕／Spaces 及系统级窗口覆盖另列平台矩阵。

应用失活回归无需辅助功能权限：在真实 Cocoa 会话运行 `tst_mascotmacosinput nativeLifecycle`。它先激活模拟 Gallery 的宿主窗口，再启动独立 helper 获取前台与键盘焦点，确认应用失活后角色视觉窗口、输入面板仍可见，原生层级不降，系统命中仍为桌宠输入面板。分别以默认 DPR 和 QT_SCALE_FACTOR=1.25 执行；不把仅由背后应用起始就激活的检查替代这个切换过程。
