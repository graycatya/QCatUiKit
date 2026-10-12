# QCatUiKit 文档

[项目首页](../../../README.md) · [English README](../../../README.en.md)

本文档描述当前源码提供的接口、工具和运行方式。项目使用 `import QCatUiKit` 和 CMake 目标 `QCatUiKit::QCatUiKit`；当前数字版本为 1.0.0，默认发布渠道为 experimental。

## 按任务阅读

| 任务 | 文档 |
| --- | --- |
| 构建库、运行 Gallery、安装到应用 | [构建与接入](BUILDING.md) |
| 查找组件、属性和交互行为 | [组件目录](COMPONENTS.md) |
| 使用表格、消息、树模型 | [数据模型](MODELS.md) |
| 调整颜色、密度、局部主题和动效 | [主题与密度](THEMING.md) |
| 添加图表、设置坐标轴、处理实时数据 | [原生图表](CHARTS.md) |
| 体验组件、配置百万图表／十万行表格／一万条对话测试 | [Gallery 使用指南](../../../examples/uikit/gallery/README.md) |
| 了解模块、源码和线程边界 | [架构](ARCHITECTURE.md) |
| 查询版本、设置发布渠道 | [版本管理](VERSIONING.md) |
| 维护公开接口、验证动态库替换 | [ABI](ABI.md) |
| 运行检查、了解已验证范围 | [验证说明](VALIDATION.md) |
| 提交代码、维护示例与翻译 | [贡献指南](../../../CONTRIBUTING.md) |
| 使用项目标识 | [品牌资源](images/brand/qcat-uikit/README.md) |
| 让 agent 使用组件库编写 UI | [Agent UI 技能](AGENT_UI.md) |
| 查看或更新当前界面截图 | [文档截图](SCREENSHOTS.md) |

## 接口范围

GPU 灯点布局、实时颜色帧、相机及线程模型见 [LED 预览](LEDPREVIEW.md)。

- **QML**：101 个 Cat 前缀类型，以及 Theme、ThemePalette、TableModel、ChatModel、TreeModel 和版本信息单例。
- **C++**：主题、表格、消息、图表、坐标轴、序列和版本查询接口；公开头安装到 `include/QCatUiKit`。
- **Gallery**：独立示例应用，数据生成器、压力测试控制器、导航和翻译管理属于示例实现，不是安装库 API。

组件目录介绍本库的属性及行为；继承自 Qt Quick Controls 的组件同时支持对应基类的标准 API。私有头、内部 QML 类型、控件子对象树与示例控制器不作为应用接口。

数据容量预设用于复现 Gallery 的演示场景。实际内存、响应时间和渲染速度还取决于数据内容、视图、图形后端和设备；测试范围见 [验证说明](VALIDATION.md)。
