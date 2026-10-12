# QCatUiKit Gallery

[项目首页](../../../README.md) · [文档索引](../../../src/uikit/doc/README.md)

Gallery 是组件体验和本地数据测试应用，按 16 个分类展示 101 个 Cat 前缀 QML 类型。每个页面包含可操作的预览、重置入口与可复制的 QML 示例。顶部提供搜索、语言和亮暗切换；设置页配置品牌色、桌面／触摸／移动密度和减少动效。

语言默认跟随系统：中文系统使用简体中文，其余使用英文。手动选择在本次运行中生效。切换语言保留当前页面、用户输入、消息、草稿和日期选择。窄窗口使用侧边抽屉导航。

## 构建与运行

Gallery 默认随项目构建，需要 Qt LinguistTools。完整步骤见 [构建与接入](../../../src/uikit/doc/BUILDING.md)。

```sh
cmake --build build/release --target qcat-gallery
./build/release/examples/uikit/gallery/qcat-gallery --language zh_CN
./build/release/examples/uikit/gallery/qcat-gallery --language en --page calendar
./build/release/examples/uikit/gallery/qcat-gallery --dark --density touch --page settings
```

Windows 单配置程序位于 `<build>/bin/qcat-gallery.exe`，多配置程序位于 `<build>/bin/<config>/qcat-gallery.exe`。

| 参数 | 含义 |
| --- | --- |
| `--language en\|zh_CN` | 界面语言；省略时跟随系统 |
| `--dark` | 暗色主题 |
| `--density desktop\|touch\|mobile` | 交互密度，默认 desktop |
| `--width <pixels>` | 初始宽度，默认 1200，最小 360 |
| `--page <id>` | home、all、settings 或组件 ID |
| `--chart-demo <0–14>` | 图表演示编号，须配合图表页面 |
| `--smoke-test` | 加载后自动退出 |
| `--screenshot <path>` | 保存窗口截图后退出 |

组件 ID 见 [GalleryCatalog.qml](GalleryCatalog.qml)，例如 `button`、`calendar`、`table`、`chat`、`chatinput`、`chart`、`chart-axis`、`chart-series`。可用 `--width 420 --density mobile` 检查窄屏。截图参数只保存窗口当前可见区域。

## 图表体验

CatChart、CatChartAxis、CatChartSeries 页面共享图表演示。选择器包含以下 15 个配置：

| 编号 | 配置 | 编号 | 配置 | 编号 | 配置 |
| --- | --- | --- | --- | --- | --- |
| 0 | 折线 | 5 | 气泡 | 10 | 柱状与折线 |
| 1 | 面积 | 6 | 饼图 | 11 | 时间轴 |
| 2 | 分组柱状 | 7 | 环形 | 12 | 对数轴 |
| 3 | 堆叠柱状 | 8 | 雷达 | 13 | 正负堆叠 |
| 4 | 散点 | 9 | 热力图 | 14 | 散点与趋势线 |

```sh
./build/release/examples/uikit/gallery/qcat-gallery --page chart --chart-demo 7
```

支持批量添加、删除最新数据、清空及恢复；滚轮缩放、拖动平移、双击／Home 重置，图例可控制显示。径向图不使用笛卡尔缩放和平移。

**百万数据体验**：点击“百万数据预设”，再启动压力测试。预设按整张图合计 1,000,000 条原始记录设置容量、每批 50,000 条总记录及 200 ms 间隔，逐步填充；趋势演示另有 2 个拟合端点。连续测试循环添加四批、删除一批，达到容量后覆盖最旧记录。

Gallery 完整保存源数据，同时采用峰谷采样、等距抽样或分组聚合生成概览。界面分别显示原始量、预览量及处理方式；概览不会随缩放重新细分。预览限制和每类数据语义见 [图表指南](../../../src/uikit/doc/CHARTS.md)。

“重置视图”恢复自动坐标范围，“恢复演示数据”恢复初始数据和参数。切换语言保留源数据与手动范围，切换演示则重新生成对应数据。

## 表格、对话与消息输入

三个页面都使用真实的 TableModel／ChatModel，记录完整保留在模型中；视图只按需创建可见项。

![十万行表格的操作与压力参数](../../../src/uikit/doc/images/gallery-table.png)

[一万条对话](../../../src/uikit/doc/images/gallery-chat.png) · [消息输入与提交历史](../../../src/uikit/doc/images/gallery-chatinput.png) · [暗色窄屏对话](../../../src/uikit/doc/images/gallery-mobile.png)

| 页面 | 手动体验 | 容量及满容量行为 |
| --- | --- | --- |
| 表格 | 批量增删改、鼠标选行、首尾／行号定位、编辑名称／状态／数值、删除选中行、调整列宽 | 最多 100,000 行，满后继续更新已有行，编号保持稳定 |
| 对话 | 双向消息、长文本、类型和状态切换、删除、首尾定位、历史浏览 | 最多 10,000 条，满后移除最旧消息 |
| 消息输入 | 按钮／回车提交、空白校验、长文本预填、草稿计数、清空草稿、查看提交历史 | 最多 10,000 条历史，满后移除最旧消息 |

对话可关闭“跟随最新消息”，在持续更新时阅读历史。生成消息长度可设为 16–4,096 字符；图片／音频／视频仅展示附件信息，不下载或播放媒体。

消息输入页的“填入长文本”预填 2,048 字符。连续测试调用实际输入框的 submit()，每第五次尝试纯空白输入并统计拦截量；停止后恢复测试前草稿。

### 配置与启动

1. 选择“十万行预设”或“一万条消息预设”，或手动调整容量、批次和间隔。
2. 表格／对话可选择持续添加、混合增删改或持续更新；混合模式循环添加四批、更新一批、删除一批。
3. 点击启动，测试持续运行；点击停止后保留当前数据与统计。

| 参数 | 表格 | 对话 | 消息输入 |
| --- | --- | --- | --- |
| 每批操作上限 | 1,000 | 100 | 25 |
| 最大保留量 | 100,000 | 10,000 | 10,000 |
| 可设更新间隔 | 16–2,000 ms | 16–2,000 ms | 16–2,000 ms |

预设只改参数，启动后逐步生成数据。批次达到约 12 ms 数据处理时间后让出界面线程，实际每批操作数可能小于设定上限。容量不能直接调到当前数量以下，先清空再调整。运行时配置和模型编辑暂时禁用，仍可停止或滚动；对话页允许手动发送。

## 停止与统计

所有持续测试均可手动停止。离开页面、重置演示、隐藏或最小化窗口时停止，不会自动恢复。再次启动会重置本次运行的统计。

- 图表统计更新次数、源数据添加量、删除／覆盖量和最慢批次。
- 数据与对话统计添加、更新、删除、空白拦截、耗时、操作频率与最慢批次。
- 累计操作量可以超过当前容量；数字以当前语言的完整整数显示。
- 耗时包含数据生成、模型修改及必要的概览／拟合处理，不包含后续完整渲染；这些数字不代表 FPS、GPU 帧耗时或网络吞吐量。

所有演示数据均在本地生成。容量与测试行为属于 Gallery 配置，不是公开模型的固定限制。

## 翻译与资源

文本使用 qsTr() 或 C++ tr()。翻译位于 `translations/qcatgallery_en.ts` 与 `translations/qcatgallery_zh_CN.ts`，同时收集 Gallery 与库组件文本。

```sh
cmake --build build/release --target qcat-gallery_lupdate
# 使用 Qt Linguist 补全两份 TS，保持 %1、%2 等占位符。
cmake --build build/release --target qcat-gallery_lrelease
cmake --build build/release --target qcat-gallery
```

GalleryLanguage 安装翻译、更新默认 QLocale 并刷新翻译绑定。项目 QM 文件嵌入资源；Qt 自带翻译在已部署时从 Qt 翻译目录加载。应用打包需要相应 Qt 模块和 SVG 图片插件。示例代码中的图片资源属于 Gallery，复制到其他应用时应换成自己的资源地址。

标识使用完整灰猫与组件角标，不随主题色重着色，详见 [品牌资源](../../../src/uikit/doc/images/brand/qcat-uikit/README.md)。

## 维护入口

| 文件 | 职责 |
| --- | --- |
| Main.qml | 窗口、搜索、导航和响应式布局 |
| GalleryCatalog.qml | 分类、组件说明和可复制示例 |
| GalleryPreview.qml / GalleryExtendedPreview.qml | 通用组件预览 |
| GalleryChartPreview.qml / gallerychartdata.cpp | 图表交互、源数据、概览和压力测试 |
| GalleryDataPreview.qml / GalleryDataStressPanel.qml / gallerydatademo.cpp | 表格、对话、输入和持续测试 |
| GalleryLanguage / gallerylanguage.cpp | 运行时语言切换 |
| GalleryLogo.qml / GalleryIcon.qml | 品牌图片与界面图标 |

自动测试位于 `tests/auto/gallery`，检查页面、示例创建、语言切换、输入保持、密度、弹出层和数据操作。当前覆盖范围见 [验证说明](../../../src/uikit/doc/VALIDATION.md)。

文档图片由实际页面生成，更新整套图片的方法见 [截图指南](../../../src/uikit/doc/SCREENSHOTS.md)。
