# 验证范围与运行方法

[文档索引](README.md)

本文记录当前源码的验证范围和复现入口。更新测试结果时同时记录平台、Qt 版本、构建配置和图形后端，避免将不同环境的结果混合为同一结论。

## 当前验证快照

截至 **2026-10-12**，最近一轮完整回归环境为 Linux、Qt 6.12.0、Debug、CMake／Ninja。图形自动测试使用 offscreen 平台插件和 Qt Quick software 后端。

| 检查 | 结果 |
| --- | --- |
| 共享库、Gallery 和全部测试构建 | 通过 |
| QCatUiKit_qmllint / qcat-gallery_qmllint | 通过 |
| 完整 CTest | 9/9 测试组通过 |
| QML 控件 | 226 项通过 |
| Gallery 页面与交互 | 573 项通过 |
| 图表核心 | 32 项通过 |
| Gallery 图表数据控制器 | 26 项通过 |
| Gallery 表格／对话控制器 | 13 项通过 |
| 翻译 | 中英文各 766 条完成，占位符一致 |
| 布局检查 | 1200 px 桌面与 420 px 窄屏，亮暗主题和桌面／触摸／移动密度 |

各 Qt Test 数量包含初始化和清理用例。当前快照不代表在所有平台、Qt 小版本和图形后端完成验证。

## macOS 输入与滚动复测：2026-10-14

环境：macOS arm64、Qt 6.10.2、Debug、CMake/Ninja；自动图形检查使用 offscreen 平台和 Qt Quick software 后端。

- 完整 CTest 11/11 组通过，其中 QML 控件 253 项、Gallery 581 项（包含初始化与清理）。
- 库与 Gallery 的两个 qmllint 目标无警告通过；Gallery 启动恢复正常。
- 验证数值输入的 Enter、Tab、点击其他控件提交、非法文本保留及纠正；整数输入与十六进制颜色输入包含失焦提交检查。
- LED 布局的 11 类数值参数均通过键盘编辑与应用检查；布局、预览设置、灯具映射覆盖直接应用、非法输入拦截、取消与重开，布局及预览还覆盖默认值恢复。
- Deck/Place 速度、设备发送间隔、图表及表格/对话压力测试参数包含无需 Enter 的提交检查。
- 滚动专项 32 项通过，包含按钮/复选框上开始的拖动、嵌套视图、文字选取与滚动条。
- 查看了实际 Gallery 的中文共享布局弹窗：1200 px 浅色桌面和 420 px 深色窄屏，输入框与操作按钮无裁切。

本轮没有验证 Metal 渲染、真实灯具输出或重新运行 Linux/Windows；不能将离屏结果视为这些平台/后端的验收。

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
| charts | 原始数据、容量、批次校验、坐标范围、采样、交互及对象生命周期 |
| led_source | RGBA8 长度、布局/帧版本、坐标校验、最新快照和跨线程通知合并 |
| led_pipeline | 布局点序/溢出、不可变多通道帧、独立 Deck 时钟和快照、Storage JSON、冻结/全黑、Patch/慢后端取消与无窗口输出 |
| led_render（可选） | 真实 GPU 三路像素、深度遮挡、颜色/混合、相机输入、DPR 和资源重建；需 `QCATUIKIT_TEST_LED_GPU=ON` |
| version | C++ 与 QML 的实际库版本、渠道和 ABI 信息 |
| qml_contracts | 类型创建、输入、状态、主题、布局、图表与密度 |
| qml_busy_timing | BusyIndicator 实际帧时序、可见性和减少动效 |
| gallery_smoke | Gallery 启动与加载 |
| gallery_contracts | 组件页面、示例创建、双语、导航、密度、数据操作与停止行为 |
| gallery_chart_data | 全部 15 个演示的百万源数据保留及概览 |
| gallery_data_demo | 十万行表格、一万条消息／提交历史、容量与持续操作 |

选择专项检查：

```sh
ctest --test-dir build/release -R '^gallery_data_demo$' --output-on-failure
ctest --test-dir build/release -R '^charts$' --output-on-failure
```

图形测试的 CTest 配置会设置离屏及软件后端。直接运行 Qt Quick 测试程序时，需要自行设置 `QT_QPA_PLATFORM=offscreen`、`QT_QUICK_BACKEND=software` 并传入构建目录的 QML 导入路径。

## 数据量与交互验证

图表数据测试实际为全部 15 个演示保留 1,000,000 条原始记录，检查概览数量、容量覆盖、删除、缩小容量和恢复；趋势演示另有拟合端点。还检查非整除分组、求和／均值、正负分离与曲线极值。页面测试验证完整整数、百万预设、反走样及持续运行后的停止。

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

软件后端的图表采用整图图像回退。物理 GPU、Vulkan／Metal／OpenGL／Direct3D、触摸手势、平台窗口操作、输入法与屏幕阅读器应在目标设备另行验收。Qt 6.8 是声明的最低版本；本次快照使用 Qt 6.12.0，不等于完成了最低版本运行时验收。

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

安装消费者不属于上述 9 个常规 CTest 组；更改导出头、依赖、安装路径或 ABI 时单独运行，并记录结果。

## 文档与示例检查

当前文档已检查本地链接、代码围栏和组件目录完整性；101 个 Cat 类型与 Gallery 目录一致。中英文 README 的构建／接入命令、功能数字和链接目标对应。

从 README、模型、图表和版本说明直接提取的 7 段完整 QML 示例均成功创建。构建指南的 CMake／C++ 应用使用独立安装包编译并加载窗口；普通 C++ 消费者在匹配的 Qt 运行时下加载安装目录中的库，主题和模型检查通过。此项使用上述 Linux / Qt 6.12.0 Debug 环境。

[UI 开发技能](../../../skills/qcatuikit-ui/SKILL.md) 通过技能格式检查。附带应用模板使用独立安装包构建，QML 检查通过；实际按钮操作验证空白拦截、添加、删除及密度切换后的草稿保持，并检查亮色桌面与暗色窄屏显示。文档的 9 个当前 Gallery 截图场景使用[截图工具](../../../tools/capture_gallery_docs.py)实际生成，包含百万图表、十万行表格及一万条消息／提交历史。

## 结果记录原则

只写实际执行的配置和结果。容量验证、CPU 数据处理、渲染性能、长时间稳定性分别记录；不同设备和构建模式的耗时不能直接混用。文档、翻译或低影响修改按相关性选择检查，无需把不相关平台写为已通过。


## LED 工作流迭代：2026-10-13

环境：Ubuntu 26.04 x86_64、GCC 15.2、Qt 6.12.0 Release，OpenGL 4.6 / Mesa 26.1.6，
AMD Radeon 780M，使用本机 X11/xcb 图形会话。源码仍声明 Qt 6.8+；本轮未在 Qt 6.8、
Windows、macOS、Vulkan、Metal 或 D3D 后端重测新增能力。

- 库、Gallery、测试构建成功；`QCatUiKit_qmllint` 和 `qcat-gallery_qmllint` 无警告通过。
- 11 组 CTest 全部通过；`led_pipeline` 当前 14 项（含初始化/清理）通过，覆盖旧 alpha 与 W 区分、
  冷暖白 50% 混合、所有通道置黑、独立时钟/工作副本、布局更新中保留待加载预设、停止状态保持黑帧、
  三种 Profile 地址与重排、慢输出取消和无窗口设备分支。
- `tst_ledrender` 在 OpenGL 的 DPR 1 和 DPR 2 各 8 项通过；新增立体几何/旋转、场景尺寸、显示间距、
  矩阵切片、独立预览限帧/停用和预览设置不修改设备通道，保留旧像素、透明背景、拖拽、MSAA、跨窗口与资源重建。
- 新 QML 契约验证设置草稿/取消/默认/非法值、相机恢复、独立 Deck UI、只读数据源、键盘预选和跨数据源活动身份。
- 安装到独立目录后，LED 消费者编译并运行成功，包含旧 LedSource/LedPreview 和新公共头/转发头、LedDeck QML。
- 实际查看 1600px 宽深色 Gallery、430px 宽中文浅色 Gallery 和预览设置弹窗；全量 Gallery 契约覆盖中英文与 360px 页面。

一次 Circle 短时基准：三路各 2048 点、32×64 矩阵、pointSize=6、ScreenPixels、透视相机，
窗口 1600×840、DPR 1，A/B 视口 389×235、Output 390×235；Deck 请求间隔 16ms，
预览目标上限 60 FPS，暖机 2 秒、测量 4.948 秒：颜色更新约 62.45 Hz，三路 LED pass 各约 55.17 FPS，
窗口 frameSwapped 约 59.42 Hz。不是 GPU 时间戳或显示器扫描频率，不代表 Cube/Sphere 的性能。

可复现入口：

```sh
qcat-gallery --page led-preview --width 1600 --dark --led-count 2048 --led-benchmark 5 --led-report /tmp/led-benchmark.json
```

本轮验证仅使用 MemoryOutputBackend；没有进行实际灯具、Art-Net、sACN 或其它硬件协议联调。
日志、截图和原始基准 JSON 留在本地临时目录，不提交构建产物。

### 同日性能优化复测

使用上述相同平台、完整 Gallery、三个 Circle / ScreenPixels 预览，默认最大 60 FPS、
pointSize=6、16 ms 生产周期。窗口 1600×840、DPR 1，视口 A/B 为 389×235、Output 为 390×235。
将优化前的共享库保留在独立目录，以相同可执行文件和参数分别运行；各次暖机 2 秒，实际测量约 6.0–6.3 秒。

| 每路点数 | 优化前 A / Output / B 绘制 FPS | 优化后 A / Output / B 绘制 FPS |
| --- | --- | --- |
| 2048（32×64） | 54.13 / 54.60 / 54.60 | 59.19 / 59.19 / 59.19 |
| 100000（250×400） | 14.76 / 29.53 / 19.84 | 55.52 / 59.29 / 57.65 |

10 万点时，A/B 颜色生产从约 14.76/19.84 Hz 提升到 56.01/62.07 Hz；
窗口 frameSwapped 从约 40.64 Hz 提升到 59.61 Hz。以上是短时实测，不保证其它硬件或 Cube/Sphere 的帧率。

变化包括：按绝对时间安排限帧并补偿定时器迟到；内置效果在每帧开始时读取参数，避免每个灯点重复查字典；
白光直接转换成预览 RGBA，移除 RGB 中间缓冲；混合循环直接访问连续缓冲，跳过值为 1 的亮度计算。
公开帧仍深拷贝外部输入并保持不可变，独立通道、透明度、量化公式及默认形状保持原语义。

使用四种效果 × 三种 Schema × 四个正/负/零时间采样，对原始通道、三组参考色温预览、
目标 Schema 转换和独立透明度进行优化前后比较，输出字节一致。

优化后 11 组 CTest 全部通过（其中管线 15 项，含初始化/清理）；OpenGL 的 DPR 1/2 各 9 项通过。
新增覆盖限帧调整时重排等待、停用/恢复和空闲时不重绘，并校验直接 RGBA 转换的通道与透明度。
GPU 测试按实际经过时间验证限帧，截图等待三路帧及目标画面完成，避免用固定等待时间推断绘制完成。
两个 QML lint 目标无警告；安装后无需重新编译的独立消费者继续运行通过。
另以三个预览共享同一 2048 点 Source 验证独立限帧：A 设为 5 FPS、其余设为 60 FPS，
实测约 4.95 / 59.04 / 59.04 FPS，低帧率预览未拖慢其它实例。

复现当前版本的帧率：

```sh
qcat-gallery --page led-preview --width 1600 --dark --led-count 2048 --led-benchmark 6 --led-report /tmp/led-2048.json
qcat-gallery --page led-preview --width 1600 --dark --led-count 100000 --led-benchmark 6 --led-report /tmp/led-100000.json
```
