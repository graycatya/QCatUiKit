# 让 agent 使用 QCatUiKit 编写 UI

[文档索引](README.md)

仓库提供 [qcatuikit-ui 技能](../../../skills/qcatuikit-ui/SKILL.md)，用于创建或改进 QCatUiKit 的 Qt Quick 界面。技能会指导 agent 选择公开组件、绑定真实状态、处理响应式布局、主题、数据模型、二维／三维及统计图表、LED 工作流和姿态／体感输入，并验证构建及界面效果。

## 在本仓库使用

根目录 [AGENTS.md](../../../AGENTS.md) 提供技能入口。读取该文件的 agent 在处理相关 UI 任务时会按入口加载技能；不要求对其他维护任务应用 UI 流程。

可以直接明确要求读取技能，例如：

> 读取 skills/qcatuikit-ui/SKILL.md，使用 QCatUiKit 做一个设备管理页，包含搜索、状态筛选、表格和编辑表单。保持现有数据接口，适配 420 px 窄屏与触摸密度。

在已加载该技能的 agent 工具中，也可使用 `$qcatuikit-ui`：

> 使用 $qcatuikit-ui 给现有对话页增加发送中和失败重试状态，并在浏览历史时保留滚动位置。

## 在其他项目使用

将整个 `skills/qcatuikit-ui` 目录作为技能交给所用 agent 工具，或让 agent 直接读取其 SKILL.md。目录包含技能说明、按需阅读的参考和应用模板，不依赖本机绝对路径。接入时提供 QCatUiKit 源码或安装位置，以及应用所用 Qt 套件。

具体技能发现与安装位置由 agent 工具管理；本仓库不自动修改用户的全局 agent 配置。若技能复制到其他位置，其说明会引导 agent 从应用依赖中定位库文档和公开类型信息。

## 按任务读取技能参考

| 任务 | 参考 |
| --- | --- |
| 组件选择、表单、表格、对话、滑块编辑 | [UI 组合](../../../skills/qcatuikit-ui/references/ui-patterns.md) |
| 二维／三维、统计／扩展图表、区间联动与 PNG 导出 | [图表组合](../../../skills/qcatuikit-ui/references/charts.md) |
| LED 布局与多通道工作流、飞机显示与独立体感输入 | [LED 与体感](../../../skills/qcatuikit-ui/references/led-and-motion.md) |
| 安装包模板、构建、运行与验证 | [构建检查](../../../skills/qcatuikit-ui/references/build-and-verify.md) |

## 可运行模板

[assets/starter](../../../skills/qcatuikit-ui/assets/starter) 是使用已安装共享库的完整 CMake/C++/QML 应用。它包含响应式表单和表格、空白校验、添加／删除记录、亮暗及密度切换。模板中的演示功能可按产品要求替换，不是强制界面结构。

复制模板后，按 [构建与检查说明](../../../skills/qcatuikit-ui/references/build-and-verify.md) 配置 Qt 和 QCatUiKit 前缀。程序提供开发用的 `--qml-import-path`、`--smoke-test` 和 `--screenshot` 参数。

## 技能覆盖的关键约定

- 使用库的公开 Cat 组件和 `import QCatUiKit`，不把 Gallery 控制器当作应用 API。
- 使用 ThemePalette 和语义 Token，显式选择交互密度；局部主题显式传给应用创建的子控件。
- 布局由 anchors 或 Layout 管理各自范围，避免重复控制几何；窄屏正确换行和滚动。
- 模型承载数据，按钮连接真实行为；提交、错误、空态和发送状态都有可观察结果。
- 大数据分批处理，明确保留量和绘制量；不把 Gallery 的容量预设或数据耗时当作普遍性能承诺。
- 运行相关构建、QML 检查和交互验证，再检查实际渲染图像。
