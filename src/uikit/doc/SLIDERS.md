# 滑块与精确输入

`import QCatUiKit` 提供三个公开类型：`CatSlider`（单值）、`CatRangeSlider`（区间）和 `CatSliderField`（滑块加数值框）。三者复用主题颜色与密度；新增刻度、提示等外观默认关闭，原有单滑块保持原来的外观。基础实现使用 Qt 6.8 已有的 Templates API。

## CatSlider

继承 Qt Quick Templates Slider，保留 `from/to/value`、`orientation`、`stepSize`、`snapMode`、`live`、`position/visualPosition`、`moved`、鼠标、触摸及方向键操作。

| 新增属性 | 默认值 | 用途 |
| --- | --- | --- |
| `trackThickness` / `handleDiameter` | `4` / `22` | 轨道厚度与手柄直径；按逻辑像素计算 |
| `tickInterval` | `0` | 均匀刻度的间隔；非正数关闭 |
| `tickValues` | `[]` | 显式刻度数值，非空时优先于间隔；忽略域外值和重复值 |
| `tickLabelsVisible` | `false` | 显示刻度标签并预留空间；密集标签自动省略 |
| `valueToolTipVisible` | `false` | 拖动、悬停、键盘焦点时显示数值提示 |
| `decimals` / `suffix` | `0` / `""` | 默认格式的小数位数与后缀；使用控件 `locale` |
| `valueFormatter` | `null` | 可选 `function(value) { return string; }`，同时用于标签与提示；覆盖默认格式 |
| `fillOrigin` | `from` | 选中轨道的填充起点；设为 `0` 可表示正负值 |
| `defaultValue` / `defaultIndicatorVisible` | `from` / `false` | 恢复目标及轨道上的默认值标记 |

`textFromValue(value)` 返回当前格式的文本。`reset()` 恢复默认值，`editValue(value)` 提交一次用户输入；数值超出范围时截断，不接受非有限值。精确输入和默认值不强制舍入到步长。`snapMode` 控制指针拖动的吸附，`stepSize` 同时影响键盘增减。

```qml
CatSlider {
    from: -100; to: 100; value: 20
    fillOrigin: 0
    stepSize: 5; snapMode: CatSlider.SnapAlways
    tickInterval: 20; tickLabelsVisible: true
    valueToolTipVisible: true
    defaultValue: 0; defaultIndicatorVisible: true
    Accessible.name: qsTr("Balance")
}
```

刻度最多创建 201 项，避免大数值域或极小间隔生成大量 QML 对象。水平刻度下方预留标签空间；垂直刻度右侧预留空间。很长的自定义标签应由应用缩短或调整控件宽度、右内边距。

## CatRangeSlider

继承 Qt Quick Templates RangeSlider，保留 `first.value`、`second.value`、两端的 `moved`、`setValues(first, second)`、方向、镜像、步长、吸附与 `live`。起始手柄、结束手柄按固定顺序获得键盘焦点，默认不可交叉。支持 `from > to` 的反向数值域；“起始”始终指向 `from` 一侧。

轨道、手柄、刻度、数值提示与格式属性与 `CatSlider` 一致，但区间填充由两个端点决定。

| 属性 | 默认值 | 用途 |
| --- | --- | --- |
| `minimumSpan` | `0` | 两端数值差的最小绝对值 |
| `maximumSpan` | `Infinity` | 最大跨度 |
| `fixedSpan` | `-1` | 非负值锁定跨度；任一手柄移动会带动另一端 |
| `rangeDragEnabled` | `false` | 拖动两手柄之间的选中轨道，整体移动区间 |
| `rangeDragging` | 只读 | 当前是否在整体拖动 |
| `defaultFirstValue` / `defaultSecondValue` | `from` / `to` | `reset()` 的目标端点 |

跨度以数值单位计算。最小跨度截断到整个数值域，最大跨度不能小于最小跨度，固定跨度再截断到这个有效范围。普通端点拖动遇到限制会停止；固定跨度或整体拖动遇到域边界会整体停止。程序设置值或改变约束后，会在下一次事件循环统一规范范围，不发出用户操作信号。

`live: false` 时，整体拖动只移动预览手柄，释放后提交数值；原生单端拖动也保持 Qt 的延迟提交语义。整体拖动只响应两端之间的轨道，不抢占手柄的标准操作；两端重合时可通过手柄和键盘重新展开。

```qml
CatRangeSlider {
    from: 0; to: 100
    first.value: 20; second.value: 70
    minimumSpan: 10; maximumSpan: 60
    rangeDragEnabled: true
    tickInterval: 20; tickLabelsVisible: true
    valueToolTipVisible: true
    Accessible.name: qsTr("Selected interval")
    onMoved: controller.setRange(first.value, second.value)
}
```

## CatSliderField

组合公开的 `CatSlider` 与 `CatNumberBox`，宽度不足时把数值输入移到下一行。

公开代理属性：`from/to/value`、`stepSize/snapMode/live`、`locale/decimals/suffix/valueFormatter`、`tickInterval/tickValues/tickLabelsVisible/valueToolTipVisible`、`fillOrigin/defaultValue/defaultIndicatorVisible`。`resetVisible` 默认 `false`；`inputWidth` 默认 `120`，表示宽布局的数值框宽度。`compact` 为只读布局状态。

只读 `slider` 和 `numberBox` 提供受支持的编辑器访问，例如设置 `slider.handleDiameter`、`numberBox.largeChange`。数值框按 locale 接受数值，输入中可保留无效文本；有效提交后限定在滑块范围内。后缀显示在输入框旁，不需要在输入时键入单位。

```qml
Item {
    width: 480; height: gainField.implicitHeight
    property real gain: -12
    CatSliderField {
        id: gainField
        width: parent.width
        from: -60; to: 12; value: parent.gain
        stepSize: 0.5; decimals: 1; suffix: " dB"
        defaultValue: -12; resetVisible: true
        Accessible.name: qsTr("Gain")
        onValueEdited: value => parent.gain = value
    }
}
```

## 同步与提交

| 事件 | CatSlider | CatRangeSlider | CatSliderField |
| --- | --- | --- | --- |
| 数值变化，包括程序同步 | `valueChanged` | `first.valueChanged` / `second.valueChanged` | `valueChanged` |
| 用户调整 | 原生 `moved()` | 汇总 `moved()`，覆盖任一端与整体拖动 | `valueEdited(value)` |
| 用户完成调整 | `valueCommitted(value)` | `valuesCommitted(firstValue, secondValue)` | `valueCommitted(value)` |

提交事件在拖动释放、方向键释放、失去编辑焦点或显式输入／恢复默认值时发出；`reset()` 已处于目标值时不重复提交。连续操作期间可用用户调整事件实时预览，提交事件用于较贵的后续处理。程序直接赋值、`setValues()` 和约束规范不会主动发出用户调整或提交事件。

外部模型绑定时，在 `onMoved` 或 `onValueEdited` 中写回模型。`editValue()`、`reset()` 和数值框会恢复调用方原有的值绑定；模型后续更新仍能传到编辑器。调用方不写回只读模型时，原绑定恢复后以模型值为准。

百分比参数可使用 `from: 0; to: 100; suffix: "%"; value: model.opacity * 100`，并在 `onValueEdited: value => model.opacity = value / 100` 中转换回原模型范围。`defaultValue` 同样使用显示范围的数值；`decimals` 控制显示精度，不会将已有模型值自动四舍五入。Gallery 的面积透明度、LED 亮度和径向尺寸采用此方式。

图表范围使用 `CatRangeSlider.onMoved` 写入轴的 `setRange()`，在 `viewChanged` 与系列数据变化时从实际绘图区同步回 `setValues()`。完整范围、选中视口应分别维护，详见 [图表外部双滑块](CHARTS.md)。Gallery 里的坐标轴适配器仍属于示例，公开滑块不依赖 Gallery。
