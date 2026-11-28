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

### Requirement: R06 IconFont

Gallery SHALL 使用随附图标字体渲染图标控件，SHALL NOT 依赖平台默认字体提供符号字形。

#### Scenario: 字体加载

- WHEN Gallery 完成模块加载
- THEN 随附图标字体 SHALL 成功加载，图标控件使用的字体族 SHALL 为该字体的族名。
- WHEN 宿主未经过应用启动入口加载 Gallery，例如自动测试
- THEN 图标控件 SHALL 仍能解析到同一字体族。

#### Scenario: 图标按钮尺寸

- WHEN 渲染表格示例的列移动按钮
- THEN 按钮宽高 SHALL 等于主题的 `controlHeight`，内边距 SHALL 为零，SHALL NOT 出现字形溢出内容区的情况。

#### Scenario: 移动边界

- WHEN 列为第一列
- THEN 左移按钮 SHALL 禁用，右移按钮 SHALL 可用。
- WHEN 列为最后一列
- THEN 右移按钮 SHALL 禁用，左移按钮 SHALL 可用。
- WHEN 移动列后顺序改变
- THEN 边界状态 SHALL 按新顺序重新计算。

#### Scenario: 图标按钮可访问名称

- WHEN 图标按钮不含可见文本
- THEN 控件 SHALL 提供说明其操作的辅助名称。

#### Scenario: 第三方资源登记

- WHEN 分发包含该字体的 Gallery 示例
- THEN 仓库 SHALL 保留 `LICENSES/RemixIcon-1.0.txt`，并在 `NOTICE` 中说明来源、版本与许可。

### Requirement: R07 StateTokenDemo

Gallery 设置页 SHALL 展示普通表面控件的交互状态色，并 SHALL 让自定义色板覆盖 `surfaceHover`、`surfaceSelected`、`surfacePressed` 与 `focusRing`。

#### Scenario: 状态色展示

- WHEN 打开设置页
- THEN 页面 SHALL 显示 `surface`、`surfaceHover`、`surfaceSelected`、`surfacePressed` 和 `focusRing` 的色块
- THEN 每个色块的颜色 SHALL 等于当前主题 `tokens` 中同名项
- THEN 页面 SHALL 提供普通、选中、禁用控件供交互对比

#### Scenario: 自定义色板状态色

- WHEN 选择 `Custom palette`
- THEN `tokens.surfaceSelected`、`tokens.surfacePressed` 和 `tokens.focusRing` SHALL 等于该色板提供的值
- THEN `disabledOpacity` SHALL 等于设置页通过 `overrides` 提供的值
- THEN 按下和选中背景 SHALL NOT 回退到内置色板的值

#### Scenario: 恢复内置色板

- WHEN 从 `Custom palette` 切换回 `Light` 或 `Dark`
- THEN 色板覆盖 SHALL 被清空
- THEN `disabledOpacity` SHALL 恢复为内置默认值 60

#### Scenario: 自定义色板缺失项

- WHEN 自定义色板未提供某个颜色 token
- THEN 该项 SHALL 使用当前 `colorScheme` 的内置值
- THEN 页面 SHALL NOT 修改库的默认色板

### Requirement: R08 RtlPreview

Gallery SHALL 提供从右到左布局预览，并 SHALL 通过同一状态驱动设置页开关与命令行入口。该预览 SHALL 只镜像布局方向，SHALL NOT 新增 RTL 语言或翻译。

#### Scenario: 设置页切换

- WHEN 在设置页打开「从右到左布局」开关
- THEN 根布局 SHALL 启用 `LayoutMirroring` 并让子项继承
- THEN 侧边栏 SHALL 移到窗口右侧，内容区 SHALL 移到左侧
- WHEN 关闭开关
- THEN 布局 SHALL 恢复原始方向

#### Scenario: 命令行入口

- WHEN 使用 `--rtl` 启动 Gallery
- THEN 首帧 SHALL 以镜像布局显示，无需再操作设置页

#### Scenario: 范围

- WHEN RTL 预览开启
- THEN 界面语言 SHALL 保持当前选择，SHALL NOT 因镜像切换翻译
- THEN 库组件的 `LayoutMirroring` 与 `mirrored` 分支 SHALL 按既有契约工作

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
