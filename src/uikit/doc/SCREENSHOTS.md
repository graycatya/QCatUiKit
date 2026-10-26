# 文档截图与更新

[文档索引](README.md)

截图分两批采集：基础截图为 **2026-10-12**，新增专题截图为 **2026-10-26**，均来自本仓库 Gallery 的真实页面，未经拼接或内容重绘。品牌标识是独立设计资源。

基础截图环境为 Linux、Qt 6.12.0、Debug，使用 offscreen + software，关闭装饰动画。它们早于后续功能迭代，不能作为最新全部功能或图形后端验收证据。

| 图片 | 状态 |
| --- | --- |
| [中文首页](images/gallery-desktop.png) | 1200 × 900，亮色桌面 |
| [英文首页](images/gallery-desktop-en.png) | 1200 × 900，亮色桌面 |
| [触摸设置](images/gallery-touch.png) | 1200 × 1100，英文暗色触摸 |
| [窄屏对话](images/gallery-mobile.png) | 420 × 3000，中文暗色移动密度，10,000 条消息 |
| [桌面图表](images/gallery-charts-desktop.png) | 1280 × 1600，保留合计 1,000,000 条原始记录的折线概览 |
| [窄屏图表](images/gallery-charts-mobile.png) | 420 × 2400，同样保留 1,000,000 条记录，暗色移动密度 |
| [表格操作](images/gallery-table.png) | 1280 × 1500，100,000 行，显示编辑与压力参数 |
| [对话操作](images/gallery-chat.png) | 1280 × 1700，10,000 条消息，显示状态与压力参数 |
| [消息输入](images/gallery-chatinput.png) | 1280 × 1500，10,000 条提交历史，实际输入框连续提交 |

长页面使用较高的真实窗口展示操作区，没有拼接不同滚动位置。压力测试在达到容量后停止，截图保留真实统计；计时随机器而变，不用于性能排名。

## 当前专题截图

新增截图使用 Linux、Qt 6.12.0、Release、XWayland（Qt xcb）/ OpenGL，AMD Radeon 780M、DPR 1，英文、亮色、桌面密度。专题图片是实际窗口的一个可视区域，长页面通过正常滚动浏览，不拼接全页。三维图表使用曲面场景并滚动到图表；体感使用 Gallery 的模拟四元数，完成校准后显示双轴预览，保持 Ready，未接管控制。矢量组件在 Ready 后捕获，Lottie 暂停并定位到 10% 进度。减少动效开启；LED 效果及运行诊断仍为动态内容，具体数值不保证逐次相同。

| 图片 | 尺寸与内容 |
| --- | --- |
| [三维图表](images/gallery-charts-3d.png) | 1440 × 900，页面滚动至规则曲面与网格切片区域 |
| [飞机姿态](images/gallery-attitude.png) | 1280 × 900，俯仰 12°、横滚 25° |
| [体感输入](images/gallery-tilt.png) | 1280 × 900，校准后的手机右倾 12°、前倾 8°模拟输入 |
| [Dock 工作区](images/gallery-dock.png) | 1440 × 900，标签、分割及布局操作入口；不展示浮动窗拖动过程 |
| [LED 工作流](images/gallery-led.png) | 1440 × 900，A/B Deck、Mixer 与三个灯点预览 |
| [SVG](images/gallery-vector-svg.png) | 1280 × 900，Emotion_Joy.svg |
| [Lottie](images/gallery-vector-lottie.png) | 1280 × 900，CatGray_All_Actions.json 暂停帧 |
| [矢量画布](images/gallery-vector-canvas.png) | 1280 × 900，渐变填充与路径 |

截图工具检查三维 supported/error、LED 三路 renderedFrames/renderError、矢量 Ready 和绘制计数，以及体感校准状态；随后仍需查看图片确认实际内容。图片不验证全部交互、物理设备、平台和性能；实验矢量后端的状态见 [矢量文档](VECTOR.md)。

在可暴露窗口的真实图形会话中执行：

```sh
cmake --build build-release --target tst_gallery --parallel 4
QT_QPA_PLATFORM=xcb python3 tools/capture_gallery_docs.py --build-dir build-release --config Release --mode native
```

上面的 Linux 命令复现本次 XWayland 捕获；使用其他原生图形平台时按运行环境设置或省略 `QT_QPA_PLATFORM`。当前窗口场景需要至少 1440 × 900 的可用区域；工具检查窗口尺寸，窗口管理器强制缩放或最大化时捕获失败。

`native` 模式继承当前平台和图形后端，不强制 software 或 offscreen；显式设置这些无效捕获环境时会拒绝。使用远程或无显示终端时，先提供真实图形会话。审阅输出使用 `--output-dir /tmp/qcat-native-doc-images`。工具仅在八个场景全部成功且 PNG 有效后复制到输出目录，原有九张基础截图不会被此模式覆盖。

## 重新生成基础截图

构建时开启 Gallery 和测试，然后从仓库根目录执行：

```sh
cmake --build build/release --target tst_gallery
python3 tools/capture_gallery_docs.py --build-dir build/release
```

多配置生成器给构建命令添加 `--config Release`，给截图工具添加 `--config Release`。如需先审阅输出，使用 `--output-dir /tmp/qcat-doc-images`，默认输出位置为 `src/uikit/doc/images`。

工具通过 Gallery 测试程序加载真实页面，设置主题和密度，填充实际模型后捕获。所有场景成功才更新输出文件；不会后期改写图片中的数据或统计。完整捕获通常需要几十秒，机器较慢时会更久。

输出后检查数字、坐标轴、控件位置、裁剪及窄屏可达性，再更新相关文案。单张普通窗口截图也可使用 [Gallery 的 --screenshot 参数](../../../examples/uikit/gallery/README.md)。
