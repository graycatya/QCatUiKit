# theme Specification

## Purpose

管理默认及局部主题配置，向组件提供颜色、尺寸、交互密度和动效 tokens。

该基线用于约束后续变更所涉及的现有对外行为，记录已核查的正常流程、输入边界和错误结果，不以模块名称代表全部接口均已逐项覆盖。

## Dependencies

- Qt Core、Gui、Qml。
- QML Theme 单例与 ThemePalette。

## Requirements

### Requirement: R01 Defaults

系统 SHALL 提供 Light/Dark 和 Desktop/Touch/Mobile 显式配置。

#### Scenario: 默认配置

- WHEN 创建 ThemePalette
- THEN 配置 SHALL 为 Light、Desktop、primaryColor=#1677ff、radius=6、fontSize=14、reducedMotion=false。

#### Scenario: 密度

- WHEN 在未覆盖尺寸 tokens 时切换 Desktop、Touch、Mobile
- THEN controlHeight SHALL 分别为 32、48、44；padding SHALL 为 12、16、16，spacing SHALL 为 8、12、12，hoverEnabled SHALL 为 true、false、false。

### Requirement: R02 Validation

系统 SHALL 校验主题输入并通知有效变化。

#### Scenario: 非法值

- WHEN 设置非法颜色或枚举
- THEN 系统 SHALL 保留原值。

#### Scenario: 边界截断

- WHEN 设置越界 radius 或 fontSize
- THEN 系统 SHALL 分别截断至 0..32 和 10..32。

#### Scenario: 通知

- WHEN 有效配置实际变化
- THEN 系统 SHALL 发出对应属性通知及 tokensChanged；相同值 SHALL NOT 重复发出这些通知。

### Requirement: R03 Overrides

系统 SHALL 仅将已识别且类型有效的覆盖值应用到 tokens。

#### Scenario: 未知键

- WHEN overrides 含未知 token 或不符合约束的值
- THEN 原始 overrides SHALL 被保存，但该项 SHALL NOT 改变计算所得 token。

#### Scenario: 数值覆盖

- WHEN 已识别数值 token 的覆盖值可转换为 0..4096 整数
- THEN 该 token SHALL 使用转换值。

#### Scenario: 布尔覆盖

- WHEN 布尔 token 被赋予非 bool 覆盖值
- THEN 系统 SHALL 忽略该项覆盖。

#### Scenario: 减少动效

- WHEN reducedMotion=true 且 overrides 包含非零 motionDuration
- THEN motionDuration SHALL 为零。

### Requirement: R04 ThemeIsolation

默认 Theme SHALL 是 QML 单例；调用方 MAY 显式传递局部 ThemePalette。

#### Scenario: 局部配置

- WHEN 修改独立 ThemePalette
- THEN 默认 Theme SHALL NOT 因此改变。

#### Scenario: 品牌前景

- WHEN 不覆盖 onPrimary 且修改有效 primaryColor
- THEN onPrimary SHALL 按品牌色亮度选择黑色或白色。

## Data Model

配置字段：colorScheme、density 为枚举；primaryColor 为 QColor；radius、fontSize 为 int；reducedMotion 为 bool；overrides 为 QVariantMap，默认空；tokens 为只读 QVariantMap。颜色 tokens 包括 primary、onPrimary、background、surface、text、border、success、warning、danger 及对应交互语义；尺寸行为 tokens 包括 radius、fontSize、controlHeight、spacing、padding、motionDuration、hoverEnabled。

## Known Deviations / Technical Debt

- 覆盖 primary 不会重新推导 onPrimary 等关联 token。
- 密度不随窗口宽度或设备自动切换。
- 透明品牌色的实际对比度取决于宿主背景，主题不验证最终合成对比度。

## Non-Goals

- 本模块 SHALL NOT 遍历任意应用对象树传播局部主题。
- 本模块 SHALL NOT 自动选择系统配色或执行布局响应。

## Assumptions

- 调用方 SHALL 在对象所属线程修改配置，并维持局部主题对象生命周期。
