# ui-components Specification (delta)

## ADDED Requirements

### Requirement: R18 ControlStateVisibility

`CatSpinBox` 的步进按钮与 `CatComboBox` 的弹出列表项 SHALL 用与静止表面色可区分的背景表达悬停、按下和选中状态，SHALL NOT 让状态背景与控件或弹出层的静止背景同色。

#### Scenario: 步进按钮悬停与按下

- WHEN 指针位于 `CatSpinBox` 的加号或减号按钮上且该按钮可用
- THEN 按钮 SHALL 显示主题主色低透明度的悬停背景
- WHEN 在该按钮上按下指针
- THEN 背景 SHALL 比悬停状态更深
- WHEN 指针离开按钮
- THEN 按钮背景 SHALL 恢复透明
- WHEN 按钮禁用或 `hoverEnabled=false`
- THEN 按钮 SHALL NOT 显示悬停背景

#### Scenario: 下拉列表选中与悬停

- WHEN `CatComboBox` 弹出列表打开
- THEN 当前项与指针悬停项的列表项背景 SHALL 与弹出层表面色不同
- THEN 当前项 SHALL 保留可辨认的主色标识
- WHEN 指针离开列表项
- THEN 该项背景 SHALL 恢复列表表面色
