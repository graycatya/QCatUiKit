# theme Specification (delta)

## ADDED Requirements

### Requirement: R05 CustomColorPalette

系统 SHALL 允许调用方在不修改库源码的前提下提供完整颜色色板，替换内置 Light/Dark 色板，且 SHALL 保留 `colorScheme` 作为内置色板选择与缺失项回退来源。

#### Scenario: 应用自定义色板

- WHEN 调用方设置包含有效颜色的 `colorPalette`
- THEN `tokens` 中对应颜色 SHALL 取 `colorPalette` 的值。
- WHEN `colorPalette` 未提供某些颜色 token
- THEN 缺失 token SHALL 取当前 `colorScheme` 的内置值。

#### Scenario: 第三套主题并存

- WHEN 同一应用中默认 `Theme` 使用内置色板，局部 `ThemePalette` 使用自定义 `colorPalette`
- THEN 两者的 `tokens` SHALL 互不影响，各自控件 SHALL 呈现各自色板。

#### Scenario: 清空色板

- WHEN `colorPalette` 被设为空 map
- THEN 全部颜色 tokens SHALL 恢复为当前 `colorScheme` 的内置值。

#### Scenario: 非法色板值

- WHEN `colorPalette` 中某项不是有效颜色
- THEN 该项 SHALL 回退为当前 `colorScheme` 的内置值，系统 SHALL 发出可诊断警告，其余 token SHALL NOT 受影响。

#### Scenario: 色板与减少动效

- WHEN `colorPalette` 与 `reducedMotion` 同时生效
- THEN 色板 SHALL 只影响颜色 tokens，`motionDuration` SHALL 仍为零。

## MODIFIED Requirements

### Requirement: R03 Overrides

系统 SHALL 将类型有效且已识别的覆盖值应用到 tokens，SHALL 保留未知 token 覆盖供调用方扩展读取，并 SHALL 对无法应用的输入发出可诊断警告。

#### Scenario: 未知键

- WHEN `overrides` 含未知 token
- THEN 该键与值 SHALL 出现在 `tokens` 中，供调用方组件读取。
- AND 系统 SHALL 发出可诊断警告。
- AND 该键 SHALL NOT 影响任何内置 token 的取值。

#### Scenario: 数值覆盖

- WHEN 已识别数值 token 的覆盖值可转换为整数
- THEN 该 token SHALL 使用按该 token 有效范围钳制后的值。
- WHEN 覆盖值无法转换为整数
- THEN 系统 SHALL 忽略该项并发出可诊断警告。

#### Scenario: 布尔覆盖

- WHEN 布尔 token 被赋予非 bool 覆盖值
- THEN 系统 SHALL 忽略该项并发出可诊断警告。

#### Scenario: 颜色覆盖

- WHEN 颜色 token 被赋予无效颜色
- THEN 系统 SHALL 保留该 token 的原值并发出可诊断警告。

#### Scenario: 减少动效

- WHEN `reducedMotion=true` 且 `overrides` 包含非零 `motionDuration`
- THEN `motionDuration` SHALL 为零。

### Requirement: R04 ThemeIsolation

默认 Theme SHALL 是 QML 单例；调用方 MAY 显式传递局部 ThemePalette；派生颜色 SHALL 基于最终生效的品牌色计算。

#### Scenario: 局部配置

- WHEN 修改独立 `ThemePalette`
- THEN 默认 `Theme` SHALL NOT 因此改变。

#### Scenario: 品牌前景

- WHEN 调用方未显式提供 `onPrimary`
- THEN `onPrimary` SHALL 按最终生效 `primary` 的亮度选择黑色或白色。
- WHEN 调用方显式提供 `onPrimary`
- THEN 该值 SHALL 优先于推导结果。

#### Scenario: 品牌交互色

- WHEN 调用方未显式提供 `primaryHover` 或 `primaryPressed`
- THEN 二者 SHALL 从最终生效 `primary` 派生。
- WHEN 调用方显式提供 `primaryHover` 或 `primaryPressed`
- THEN 该值 SHALL 优先于推导结果。

#### Scenario: 派生跟随覆盖

- WHEN `primary` 经 `colorPalette` 或 `overrides` 改变，且未显式提供派生色
- THEN `onPrimary`、`primaryHover`、`primaryPressed` SHALL 随之重新计算，`tokensChanged` SHALL 发出一次。
