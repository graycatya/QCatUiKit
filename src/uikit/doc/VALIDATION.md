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
