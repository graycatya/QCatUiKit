# ui-components Specification (delta)

## MODIFIED Requirements

### Requirement: R07 WindowCaptionButtons

CatTitleBar SHALL 以绘制图标提供最小化、最大化／还原和关闭按钮，并为正常、悬停、按下、禁用和键盘焦点状态提供可区分外观。关闭按钮悬停 SHALL 使用主题危险色背景和对比色叉号，按下 SHALL 使用更深的危险色；最小化和最大化按钮悬停 SHALL 使用 `surfaceHover`，按下 SHALL 使用 `surfacePressed`，SHALL NOT 使用禁用背景。禁用按钮 SHALL 使用禁用前景色且不执行窗口操作，键盘焦点 SHALL 显示主题主色描边。关闭按钮静止时 SHALL 使用标题栏表面色，SHALL NOT 显示常驻的危险色边框。

最大化按钮图标 SHALL 随 `targetWindow.visibility` 在最大化与还原之间切换，点击 SHALL 执行对应操作。

#### Scenario: 关闭按钮悬停与按下

- WHEN 指针位于关闭按钮上
- THEN 按钮背景 SHALL 变为主题危险色，叉号 SHALL 使用与背景对比的颜色
- WHEN 按下关闭按钮
- THEN 背景 SHALL 使用更深的危险色，松开后 SHALL 请求关闭窗口

#### Scenario: 最小化与最大化按下

- WHEN 指针悬停在可用的最小化或最大化按钮上
- THEN 背景 SHALL 使用 `surfaceHover`
- WHEN 按下该按钮
- THEN 背景 SHALL 使用 `surfacePressed`，且 SHALL 与悬停背景不同
- THEN 背景 SHALL NOT 等于禁用色 `disabled`

#### Scenario: 最大化状态

- WHEN targetWindow 进入最大化
- THEN 最大化按钮 SHALL 显示还原图标，点击 SHALL 还原窗口
- WHEN targetWindow 退出最大化
- THEN 按钮 SHALL 显示单框最大化图标，点击 SHALL 最大化窗口

#### Scenario: 禁用与焦点

- WHEN targetWindow 为空
- THEN 三个按钮 SHALL 显示禁用色且 SHALL NOT 执行窗口操作
- WHEN 通过键盘将焦点移动到任一按钮
- THEN 按钮 SHALL 显示主题主色焦点描边

## ADDED Requirements

### Requirement: R20 TitleBarIcons

CatTitleBar SHALL 允许通过 `minimizeIcon`、`maximizeIcon`、`restoreIcon` 和 `closeIcon` 组件覆盖对应图标；未提供时 SHALL 使用内置绘制。覆盖图标 SHALL NOT 改变按钮尺寸、可访问名称、状态背景或窗口操作。

#### Scenario: 自定义图标

- WHEN 应用为 `minimizeIcon`、`maximizeIcon`、`restoreIcon` 或 `closeIcon` 提供组件
- THEN 对应按钮 SHALL 显示该组件并隐藏内置绘制图标
- WHEN 未提供组件
- THEN 按钮 SHALL 继续显示内置绘制图标
- THEN 按钮尺寸、辅助名称、状态背景和窗口操作 SHALL 保持不变
