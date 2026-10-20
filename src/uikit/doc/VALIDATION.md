# 验证范围与运行方法

[文档索引](README.md) · [历史执行记录](VALIDATION_HISTORY.md)

本文维护可复现的检查入口。最近已记录的完整回归为 **2026-10-19，Ubuntu 26.04 x86_64、Qt 6.12.0 Release**；以下为既有记录摘要，不代表每次文档修改都重新执行了产品测试。

## Dock 浮动窗口与回停（2026-10-20，P3 前两步）

环境：Ubuntu 26.04 x86_64、Qt 6.12.0 Release。离屏测试使用 software/offscreen；原生测试使用 XCB/OpenGL，
日志确认 AMD Radeon 780M Graphics、Mesa 26.1.6。

| 检查 | 结果 |
| --- | --- |
| 构建、lint、全量回归 | 构建与库／Gallery qmllint 通过；CTest 17/17 通过 |
| 浮动专项最终版 | software/offscreen 8/8、XCB/OpenGL 8/8（含初始化／清理） |
| 覆盖行为 | 输入实例与焦点保持、弹层重新打开、关闭请求拒绝／接受、标签组恢复、隐藏／销毁清理、窗口尺寸保存、屏幕外布局修正 |
| 图表迁移 | 浮动、回停、再次恢复后检查红色系列像素，验证实际渲染而非仅对象存在 |
| C++ 控制器 | 原标签索引／分割比例恢复、整组浮动、原目标消失回退、跨主树与浮动组的重复 ID 拒绝、v1 兼容读取 |
| Gallery 专项 | 4/4（含初始化／清理）：真实按钮浮动整组、不可关闭编辑器阻止系统关闭、回停、窄屏 |
| 安装与替换 | 新安装消费者通过；上一阶段已编译消费者加载新 libQCatUiKit.so.1 通过，动态库解析路径确认指向候选安装目录 |
| 视觉与翻译 | 明暗浮动窗口／图表预览；新增中英文文案完整；git diff --check 通过 |

浮动专项仅允许 offscreen 插件的已知 `propagateSizeHints()` 能力提示，其余警告导致失败；XCB 原生检查没有该提示。
未验证 Windows、macOS、Wayland、Vulkan、Qt 6.8、实际多显示器热插拔或混合 DPI。
本批仅实现浮动与按钮／API 回停，跨窗口拖拽尚未实现。接口见 [Dock 工作区](DOCKING.md)。

下面保留 P0–P2 阶段的验证记录。

## 单窗口 Dock（2026-10-20）

环境：Ubuntu 26.04 x86_64、Qt 6.12.0 Release；QML 与 Gallery 使用 offscreen/software。

| 检查 | 结果 |
| --- | --- |
| 全量构建与 CTest | 构建通过，17/17 个测试组通过 |
| 最终界面修改后的回归 | qml_contracts、gallery_contracts 2/2 通过；包含新增窄屏 Gallery 测试 |
| Dock QML 专项 | 6/6（含初始化／清理）：内容保留、恢复、取消、真实鼠标拖拽、分隔条及最小尺寸 |
| Gallery Dock 专项 | 3/3（含初始化／清理）：窄屏、关闭／打开、恢复初始布局；警告视为失败 |
| lint | QCatUiKit_qmllint 与 qcat-gallery_qmllint 通过 |
| 安装消费者 | tests/auto/cmake/dock-consumer 通过；三种公开头、导出类、安装后的 QML 类型可用 |
| 视觉检查 | 中文明色桌面、720px 暗色、360px mobile；窄屏示例使用横向滚动 |
| 其他 | 两种语言新增文案完整，git diff --check 通过 |

独立消费者运行时必须使用与构建一致的 Qt 库与 QML 导入目录，避免系统 Qt 和安装套件混用。
本批不包含浮动窗口／跨窗口拖拽，不声称验证 Windows、macOS、Wayland、Qt 6.8 或真实 GPU 后端。
接口与实施边界见 [Dock 工作区](DOCKING.md)。

## 姿态球与设备无关体感输入（2026-10-19）

环境：Ubuntu 26.04 x86_64、Qt 6.12.0 Release；software/offscreen，以及 XCB 上的实际 Vulkan/OpenGL。
Vulkan 日志确认 AMD Radeon 780M Graphics / RADV PHOENIX，Mesa 26.1.6；未开启 Khronos validation layer。
新增公开 CatAttitudeIndicator、CatTiltControlPanel 和 TiltController，接口与输入坐标约定见 [姿态与体感](ATTITUDE.md)。

| 检查 | 结果 |
| --- | --- |
| 完整 CTest | 16/16 组通过，含 tilt_controller；默认图形契约使用 software/offscreen |
| 控件 QML / Gallery | software 349/349；Gallery software 与 Vulkan DPR 1 各 811/811；表格列宽拖动曾有一次未稳定复现的波动 |
| 二维／三维／PNG RHI | Linux XCB、AMD Radeon 780M／RADV、Mesa 26.1.6；Vulkan DPR 1／2 分别为 192／22／7 项通过，OpenGL 二维／三维为 192／22 |
| Windows 历史覆盖 | Qt 6.12 MSVC Release 的 D3D11／OpenGL 图表记录；当时 led_pipeline 有失败，不能用后续 Linux 通过结果宣称 Windows 问题已修复 |
| macOS 历史覆盖 | 2026-10-14，Qt 6.10.2 arm64 Debug、software；早于后续图表及姿态接口，不代表 Metal 或新接口验收 |

详细环境、初次失败、复测及安装消费者证据见 [历史验证记录](VALIDATION_HISTORY.md)。Qt 6.8 最低版本、其他 GPU／驱动、Metal／D3D12、真机触屏、真实传感器及设备丢失没有由上述结果获得通用验证。

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

## 文档与示例检查

从仓库根目录运行 `python tools/check_docs.py`，检查本地 Markdown／HTML 资源链接、标题锚点、代码围栏和 Cat 组件目录覆盖。该工具不访问外网，不替代 QML 编译或运行验证。

修改技能后，检查 SKILL.md 的路由、参考链接和模板是否对应当前公开接口。模板构建与实际交互方法见 [技能构建参考](../../../skills/qcatuikit-ui/references/build-and-verify.md)。文档截图的生成与人工复核见 [截图指南](SCREENSHOTS.md)。

## 结果记录原则

只写实际执行的配置和结果。容量验证、CPU 数据处理、渲染性能、长时间稳定性分别记录；不同设备和构建模式的耗时不能直接混用。文档、翻译或低影响修改按相关性选择检查，无需把不相关平台写为已通过。
