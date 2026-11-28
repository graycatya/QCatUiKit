# theme Specification (delta)

## ADDED Requirements

### Requirement: R06 StateTokens

系统 SHALL 在颜色 token 中提供 `surfacePressed`、`surfaceSelected` 和 `focusRing`，并 SHALL 提供取值 0–100 的整数 token `disabledOpacity`。

#### Scenario: 默认状态色

- WHEN 使用内置 Light 或 Dark 色板
- THEN `surfacePressed`、`surfaceSelected` 和 `surfaceHover` SHALL 提供由浅到深的三级交互底色
- THEN `disabledOpacity` SHALL 为 60

#### Scenario: 焦点环派生

- WHEN 调用方未显式提供 `focusRing`
- THEN `focusRing` SHALL 等于最终生效的 `primary`
- WHEN `primary` 经 `colorPalette` 或 `overrides` 改变且未显式提供 `focusRing`
- THEN `focusRing` SHALL 随之重新计算
- WHEN 调用方显式提供 `focusRing`
- THEN 该值 SHALL 优先于派生结果

#### Scenario: 状态色覆盖

- WHEN `colorPalette` 或 `overrides` 提供有效的 `surfacePressed`、`surfaceSelected` 或 `focusRing`
- THEN `tokens` 中对应项 SHALL 使用提供的颜色
- WHEN `overrides` 提供 `disabledOpacity`
- THEN 该值 SHALL 被钳制到 0–100 后写入 `tokens`
- WHEN 提供的状态色无效
- THEN 该项 SHALL 保留原值并发出可诊断警告

#### Scenario: 色板缺失项

- WHEN 自定义 `colorPalette` 未提供新状态 token
- THEN 这些 token SHALL 使用当前 `colorScheme` 的内置值
