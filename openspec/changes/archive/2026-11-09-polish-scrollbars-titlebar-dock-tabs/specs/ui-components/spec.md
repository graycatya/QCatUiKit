# Spec Delta

## ADDED Requirements

### Requirement: R06 OverlayScrollBars

CatScrollBar SHALL 暴露 `revealed` 属性并默认 `true`。当 `revealed` 为 `false` 且滑块未按下、未滚动、未悬停时，滚动条 SHALL 淡出到不可见；淡出状态 SHALL 仍能响应指针进入并重新显示，SHALL 仍可拖动。`AlwaysOn` 策略的滚动条 SHALL 始终可见。

CatScrollView SHALL 在指针位于视图范围内、滚动中、拖动中或悬停滑块时显示水平和垂直滚动条；指针离开且滚动停止后 SHALL 隐藏。滚动条 SHALL 以叠加方式呈现，SHALL NOT 因显示或隐藏改变内容可用宽度或高度。

#### Scenario: 指针进入与离开

- WHEN 指针进入 CatScrollView 视图范围
- THEN 水平和垂直滚动条 SHALL 按主题动效时长淡入
- WHEN 指针离开且没有滚动、拖动或滑块悬停
- THEN 两个滚动条 SHALL 淡出到不可见

#### Scenario: 视图外滚动

- WHEN 指针位于视图外时通过滚轮、拖动或触摸使视图滚动
- THEN 滚动条 SHALL 在滚动期间显示；滚动停止后 SHALL 恢复隐藏

#### Scenario: 独立使用与 AlwaysOn

- WHEN CatScrollBar 独立使用或策略为 AlwaysOn
- THEN 滚动条 SHALL 不受指针位置影响保持可见；`revealed` SHALL 默认为 `true`

#### Scenario: 减少动效

- WHEN 主题启用 reducedMotion
- THEN 滚动条的显示与隐藏 SHALL 直接切换，不产生动画过渡

### Requirement: R07 WindowCaptionButtons

CatTitleBar SHALL 以绘制图标提供最小化、最大化／还原和关闭按钮，并为正常、悬停、按下、禁用和键盘焦点状态提供可区分外观。关闭按钮悬停 SHALL 使用主题危险色背景和对比色叉号，按下 SHALL 使用更深的危险色；最小化和最大化按钮悬停 SHALL 使用 `surfaceHover`。禁用按钮 SHALL 使用禁用前景色且不执行窗口操作，键盘焦点 SHALL 显示主题主色描边。关闭按钮静止时 SHALL 使用标题栏表面色，SHALL NOT 显示常驻的危险色边框。

最大化按钮图标 SHALL 随 `targetWindow.visibility` 在最大化与还原之间切换，点击 SHALL 执行对应操作。

#### Scenario: 关闭按钮悬停与按下

- WHEN 指针位于关闭按钮上
- THEN 按钮背景 SHALL 变为主题危险色，叉号 SHALL 使用与背景对比的颜色
- WHEN 按下关闭按钮
- THEN 背景 SHALL 使用更深的危险色，松开后 SHALL 请求关闭窗口

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
