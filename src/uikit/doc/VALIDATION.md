# 验证范围与运行方法

[文档索引](README.md) · [历史执行记录](VALIDATION_HISTORY.md)

本文维护可重复执行的检查步骤。测试结果必须结合日期、平台和检查范围阅读；文档更新不表示产品测试已重新运行。逐次执行摘要保留在[历史记录](VALIDATION_HISTORY.md)。

## 最近结果摘要

| 范围 | 最近记录 | 结果与限制 |
| --- | --- | --- |
| Wayland Dock 平台分支 | 2026-10-22，Ubuntu 26.04、Qt 6.12.0 Release | 构建、库／Gallery lint、CTest 18/18 通过；Wayland 适配器 5/5、QML 状态 6/6。XCB 原有浮动坐标失败在未修改基线也复现；真实鼠标拖放和 Windows/macOS 本次未验收，详见历史记录 |
| Dock、全库回归 | 2026-10-21，Windows 11、Qt 6.12.0、MSVC 2022 Release | Dock 控制器 16/16、工作区 12/12、浮动 25/25、Gallery Dock 15/15；构建及库／Gallery qmllint 通过。全库 CTest 为 15/17，仍有 LED 与下拉框宽度失败 |
| Windows 现存失败 | 同上 | `led_pipeline` 的 `layoutMappingAndValidation` 在 `m.applyNow(c)` 失败；`qml_contracts` 的 `SelectionControls::test_compactComboDisplayFits` 三种密度失败。全套 QML 为 383 通过、3 失败；未宣称全库通过 |
| Dock 安装与兼容 | 同上 | 独立安装消费者可调用 C++／QML 的 `dockToWorkspace`；保留原 1384 个导出符号，新增 1 个；旧测试程序加载新库 10/10 通过 |
| Linux 全库回归 | 2026-10-20，Ubuntu 26.04、Qt 6.12.0 Release | 当时 CTest 17/17、浮动 software/offscreen 与 XCB/OpenGL 各 8/8；早于最新跨窗口与外边缘修改，不作为最新版本的 Linux 全量验收 |
| Linux 图表 GPU | 2026-10-19，同一 Linux／Qt 环境 | AMD Radeon 780M／RADV、Mesa 26.1.6；Vulkan DPR 1／2 二维／三维／PNG 192／22／7，OpenGL 二维／三维 192／22；未开启 Khronos validation layer |

计数包含测试初始化／清理。Dock 的 Windows 原生窗口测试使用 QtTest 合成鼠标与 software 渲染；多屏混合 DPI、实际热插拔和 Wayland 仍需单独验收。不同日期、平台的通过结果不能互相替代；最低 Qt 6.8、其他 GPU／驱动、真机触屏和设备丢失也没有由这些结果获得通用验证。

## 自动测试入口

先按 [构建说明](BUILDING.md) 配置并构建：

```sh
cmake --build build/release --parallel
cmake --build build/release --target QCatUiKit_qmllint qcat-gallery_qmllint
ctest --test-dir build/release --output-on-failure
```

多配置生成器使用相同的 `--config Release` 与 `ctest -C Release`。关闭 Gallery 时不会注册 Gallery 相关测试。

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

安装后的检查不依赖源码头文件路径。普通消费者见 [构建说明](BUILDING.md)。图表消费者：

```sh
cmake --install build/release --prefix "$PWD/stage"
cmake -S tests/auto/cmake/charts-consumer -B build/charts-consumer -GNinja \
  -DCMAKE_PREFIX_PATH="$PWD/stage;/path/to/Qt/6.x/gcc_64"
cmake --build build/charts-consumer
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software \
  LD_LIBRARY_PATH="$PWD/stage/lib:/path/to/Qt/6.x/gcc_64/lib" \
  ./build/charts-consumer/charts-consumer "$PWD/stage/lib/qml"
```

按实际安装布局替换 lib。Windows 将项目与 Qt 的 bin 目录加入 PATH。版本消费者的构建和运行参数见 [版本管理](VERSIONING.md)，已编译程序的库替换检查见 [ABI](ABI.md)。

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
