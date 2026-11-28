# ui-components Specification (delta)

## ADDED Requirements

### Requirement: R21 ComboSpinKeyboard

`CatComboBox` SHALL 支持键盘打开、关闭和列表导航；`CatSpinBox` SHALL 支持方向键步进。键盘焦点 SHALL 通过 `focusRing` 描边表达。

#### Scenario: 下拉键盘焦点

- WHEN 通过 Tab 将键盘焦点移动到 `CatComboBox`
- THEN 控件 SHALL 显示宽 2 的 `focusRing` 描边

#### Scenario: 打开与关闭

- WHEN 不可编辑的 `CatComboBox` 获得键盘焦点并按下 Space
- THEN 弹出列表 SHALL 打开
- WHEN 弹出列表打开时按下 Esc
- THEN 列表 SHALL 关闭，`currentIndex` SHALL 保持不变

#### Scenario: 列表高亮导航

- WHEN 弹出列表已打开
- THEN Down 和 Up SHALL 分别把 `highlightedIndex` 移到下一项和上一项
- THEN Home 和 End SHALL 分别把 `highlightedIndex` 移到首项和末项

#### Scenario: 数值步进

- WHEN `CatSpinBox` 获得键盘焦点并按下 Up 或 Down
- THEN `value` SHALL 按 `stepSize` 增加或减少
- WHEN 已到达 `to` 或 `from`
- THEN 继续按对应方向键 SHALL 保持边界值，SHALL NOT 越界
