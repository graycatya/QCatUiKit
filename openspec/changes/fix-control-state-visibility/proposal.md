# Proposal

## Why

Gallery 表格页的交互控件在浅色主题下状态区分不足，录屏中选中一行并进入单元格编辑后，同一区块的步进按钮与下拉列表状态难以辨认：

- `CatSpinBox` 的步进按钮（`+`／`−`）悬停和按下时使用 `surfaceHover`，而整个控件背景在悬停时同样是 `surfaceHover`。按钮反馈被同色背景完全覆盖，指针落在按钮上时没有任何可见变化。
- `CatComboBox` 弹出列表的悬停项与当前项使用 `surfaceHover` 作背景。浅色主题下 `surfaceHover`（`#f0f5ff`）与弹出层表面色（`#ffffff`）差异只有约 3%，当前项仅靠 3 逻辑像素主色条区分，选中和悬停状态不明显。

## What Changes

- `CatSpinBox` 的 up/down 指示器悬停时使用主题主色的低透明度叠加，按下时进一步加深；静止时保持透明，禁用时不显示交互背景。
- `CatComboBox` 弹出列表项的悬停与键盘高亮背景改用主题主色低透明度叠加，与弹出层表面色可区分；当前项保留左侧主色条。
- 增加 QML 行为测试，断言上述状态颜色与控件或弹出层背景不同。

不改变公开属性、默认尺寸和主题 Token 定义；`CatSpinBox` 的文本字形 `+`／`−` 本次不变。

## Capabilities

### New Capabilities

无。

### Modified Capabilities

- `ui-components`：增加数值步进与下拉选择控件交互状态的可辨识要求。

## Impact

影响 `src/uikit/qml/CatSpinBox.qml`、`src/uikit/qml/CatComboBox.qml`、`tests/auto/qml/data/tst_extendedcontrols.qml`。不涉及 C++ 接口、主题 Token 数值、Gallery 示例或第三方资源。
