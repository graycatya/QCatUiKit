# Proposal

## Why

`ui-components` 当前只覆盖 `CatComboBox` 和 `CatSpinBox` 的状态色（R18、R19），没有键盘与焦点契约。这两个控件在 Gallery 的表格编辑页和设置页都是主要输入入口，现有测试也只有鼠标、焦点丢失和状态色用例，没有键盘打开、导航和步进的证据。

实际行为已经存在：ComboBox 支持 Space 打开、Esc 关闭、方向键与 Home/End 移动高亮；SpinBox 支持 Up/Down 步进并在边界停止。缺少的是可复核的规约和测试，不是实现。

## What Changes

- 新增 `ui-components` 需求 R21 `ComboSpinKeyboard`，写明 ComboBox 的键盘焦点、打开/关闭、高亮导航，以及 SpinBox 的方向键步进与边界。
- 在 `tst_extendedcontrols.qml` 增加 `test_comboKeyboardNavigation` 和 `test_spinKeyboardSteps`。
- 在 `COMPONENTS.md` 补充具体键位。

不修改 QML 或 C++ 实现；如果测试暴露缺陷，另行修复。

## Capabilities

### New Capabilities

无。

### Modified Capabilities

- `ui-components`：新增 R21 `ComboSpinKeyboard`。

## Impact

影响 `tests/auto/qml/data/tst_extendedcontrols.qml`、`src/uikit/doc/COMPONENTS.md` 和 `openspec/specs/ui-components/spec.md`（归档时）。公开 API 不变。
