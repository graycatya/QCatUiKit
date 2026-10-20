# QCatUiKit 文档

[项目首页](../../../README.md) · [English README](../../../README.en.md)

本文档描述当前源码提供的接口、工具和运行方式。项目使用 `import QCatUiKit` 和 CMake 目标 `QCatUiKit::QCatUiKit`；当前数字版本为 1.0.0，默认发布渠道为 experimental。

## 按任务阅读

| 任务 | 文档 |
| --- | --- |
| 构建库、运行 Gallery、安装到应用 | [构建与接入](BUILDING.md) |
| Dock 分割／标签、浮动回停和布局保存恢复 | [Dock 工作区](DOCKING.md) |
| 定位维护入口、减少重复检索 | [维护入口](MAINTENANCE.md) |
| 查找组件、属性和交互行为 | [组件目录](COMPONENTS.md) |
| 使用单值／区间滑块、刻度、精确输入与图表范围联动 | [滑块与精确输入](SLIDERS.md) |
| 显示 GPU 灯点、接入布局／Deck／Storage／Mixer 与多通道输出 | [LED 预览](LEDPREVIEW.md)、[LED 工作流](LEDWORKFLOW.md) |
| 使用表格、消息、树模型 | [数据模型](MODELS.md) |
| 显示飞机姿态、处理应用提供的手机体感输入 | [姿态球与体感输入](ATTITUDE.md) |
| 调整颜色、密度、局部主题和动效 | [主题与密度](THEMING.md) |
| 添加图表、设置反向轴／次刻度、框选与触摸交互、处理实时数据及导出 PNG | [原生图表](CHARTS.md) |
| 添加拟合／残差带、瀑布图、堆叠面积及分箱统计 | [统计图表适配器](CHARTSTATISTICS.md) |
| 使用三维柱阵、散点、轨迹、曲面、相机、Blinn–Phong／Phong 材质和切片 | [三维图表](CHARTS3D.md) |
| 了解图表图层、线程、缓存、上传和未覆盖能力 | [图表渲染架构与边界](CHART_RENDERING.md) |
| 体验组件、配置百万图表／十万行表格／一万条对话测试 | [Gallery 使用指南](../../../examples/uikit/gallery/README.md) |
| 了解模块、源码和线程边界 | [架构](ARCHITECTURE.md) |
| 查询版本、设置发布渠道 | [版本管理](VERSIONING.md) |
| 维护公开接口、验证动态库替换 | [ABI](ABI.md) |
| 气泡／格网／等高线／OHLC／箱线／雷达增强 | [扩展图表](CHARTEXTENSIONS.md) |
| 运行检查、了解最近结果与历史平台范围 | [验证方法](VALIDATION.md)、[历史记录](VALIDATION_HISTORY.md) |
| 提交代码、维护示例与翻译 | [贡献指南](../../../CONTRIBUTING.md) |
| 使用项目标识 | [品牌资源](images/brand/qcat-uikit/README.md) |
| 让 agent 使用组件库编写 UI | [Agent UI 技能](AGENT_UI.md) |
| 查看或更新当前界面截图 | [文档截图](SCREENSHOTS.md) |

## 接口范围

GPU 渲染和兼容 RGBA8 接口见 [LED 预览](LEDPREVIEW.md)；共享布局、Deck/Storage、多通道混合及设备内存输出见 [LED 工作流](LEDWORKFLOW.md)。

- **QML**：119 个 Cat 前缀类型，以及 Theme、ThemePalette、TableModel、ChatModel、TreeModel、TiltController、DockController 和版本信息单例。
- **C++**：主题、表格、消息、图表、坐标轴、序列、LED 工作流、TiltController、DockController 和版本查询接口；公开头安装到 `include/QCatUiKit`。
- **Gallery**：独立示例应用，数据生成器、压力测试控制器、导航和翻译管理属于示例实现，不是安装库 API。

组件目录介绍本库的属性及行为；继承自 Qt Quick Controls 的组件同时支持对应基类的标准 API。私有头、内部 QML 类型、控件子对象树与示例控制器不作为应用接口。

数据容量预设用于复现 Gallery 的演示场景。实际内存、响应时间和渲染速度还取决于数据内容、视图、图形后端和设备；测试范围见 [验证说明](VALIDATION.md)。
