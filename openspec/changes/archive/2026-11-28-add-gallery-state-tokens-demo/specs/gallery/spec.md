# gallery Specification (delta)

## ADDED Requirements

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
