# gallery Specification

## Purpose

提供独立示例应用，用于浏览组件、搜索示例、切换主题语言、生成截图及执行演示基准。

该基线用于约束后续变更所涉及的现有对外行为，记录已核查的正常流程、输入边界和错误结果，不以模块名称代表全部接口均已逐项覆盖。

## Dependencies

- QCatUiKit 与内部 QML 模块 QCatUiKit.Gallery。
- GalleryCatalog、页面和演示控制器、翻译资源及可用 Qt Quick 环境。

## Requirements

### Requirement: R01 Navigation

Gallery SHALL 支持 home、all、settings 和目录组件 ID 启动入口。

#### Scenario: 指定页面

- WHEN --page 指定有效页面或组件 ID
- THEN Gallery SHALL 打开对应内容。

#### Scenario: 非法页面

- WHEN --page 指定未知 ID
- THEN 程序 SHALL 显示命令帮助并以状态码 2 退出。

#### Scenario: 搜索

- WHEN 输入搜索文本
- THEN 结果 SHALL 按当前分类以及名称、标题、描述和 searchTerms 的不区分大小写包含匹配筛选。

### Requirement: R02 StartupValidation

Gallery SHALL 校验命令行主题密度、语言和宽度。

#### Scenario: 非法参数

- WHEN density 不为 desktop/touch/mobile、显式 language 不为 en/zh_CN，或 width 不是至少 360 的整数
- THEN 程序 SHALL 以状态码 2 退出。

#### Scenario: 创建失败

- WHEN 根 QML 对象创建失败或未创建根对象
- THEN 程序 SHALL 以状态码 1 退出。

### Requirement: R03 Translation

Gallery SHALL 支持英文和简体中文的运行时翻译。

#### Scenario: 初始语言

- WHEN 未显式指定语言
- THEN 中文系统语言 SHALL 选择 zh_CN，其他系统语言 SHALL 选择 en。

#### Scenario: 翻译缺失

- WHEN 目标 Gallery 翻译目录加载失败
- THEN 系统 SHALL 保留原语言并输出警告。

#### Scenario: 切换成功

- WHEN 有效目标翻译目录加载成功
- THEN 系统 SHALL 更新语言、默认 locale 并重新翻译 QML。

### Requirement: R04 CaptureAndSmoke

Gallery SHALL 提供 smoke-test 和截图退出模式。

#### Scenario: 冒烟退出

- WHEN 指定 --smoke-test 且加载成功
- THEN 程序 SHALL 在启动计时约 700ms 后退出。

#### Scenario: 截图失败

- WHEN 指定 --screenshot 但窗口不存在或保存失败
- THEN 程序 SHALL 以状态码 2 退出。

### Requirement: R05 Benchmark

LED 基准 SHALL 校验参数、暖机并报告观测结果。

#### Scenario: 非法基准组合

- WHEN --led-report 无 --led-benchmark，或基准与 screenshot/smoke-test 同时使用
- THEN 程序 SHALL 以状态码 2 退出。

#### Scenario: 有效基准

- WHEN 指定有效 LED 基准时长
- THEN 程序 SHALL 暖机 2 秒后统计，并输出包含预览 pass、颜色更新和窗口呈现频率的 JSON。

#### Scenario: 报告失败

- WHEN 指定报告文件但无法完整写入
- THEN 程序 SHALL 以状态码 4 退出。

## Data Model

命令默认：page=home、density=desktop、width=1200；language 未指定时按系统语言选择。chart-demo 为 0..29；LED count 为 0..100000、interval 为 4..1000ms、layout 为 matrix/sphere、benchmark 为 1..3600 秒。目录项包含 id、name、category、title、description、icon 和示例 code。

## Known Deviations / Technical Debt

- smoke-test 的计时退出不逐个验证全部组件或 GPU 成功。
- 截图采用固定启动等待，不能作为每类异步内容 Ready 的保证。
- 窗口呈现与渲染 pass 指标不是 GPU 耗时或显示器扫描完成。
- 示例容量与性能结果不构成库的通用容量承诺。

## Non-Goals

- Gallery SHALL NOT 将内部演示 helper 作为安装库 API。
- Gallery SHALL NOT 实现真实传感器、飞控或物理灯光设备通信。

## Assumptions

- 运行 SHALL 已构建 examples，并能定位库、QML 插件和翻译资源；截图与 GPU 基准 SHALL 提供相应图形环境。
