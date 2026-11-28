# Proposal

## Why

应用层目前只能通过 `surfaceHover` 定义一个普通表面的交互状态。`pressed`、`checked`、`selected`、`open` 全部复用该颜色，`focus` 固定使用 `primary`，禁用透明度在各组件里写死为 `0.6`。结果是：

- 无法分别定义悬停、按下、选中和打开状态，任何一处调整都会同时改变所有状态。
- 主题只提供"一层"交互底色，复合控件内部的状态叠加无法与父背景区分。
- 焦点环颜色不随品牌色解析链统一，禁用透明度无法配置。

## What Changes

主题新增四个状态 token：

- `surfacePressed`：普通表面按下背景。
- `surfaceSelected`：选中、当前和高亮项背景。
- `focusRing`：键盘焦点描边颜色，默认跟随最终生效的 `primary`。
- `disabledOpacity`：禁用内容透明度，整数百分比 0–100，默认 60。

组件层把状态映射改为读取对应 token：

- 悬停继续使用 `surfaceHover`。
- 按下使用 `surfacePressed`。
- 选中、键盘高亮、下拉当前项和打开态使用 `surfaceSelected`。
- 焦点描边使用 `focusRing`。
- 禁用透明度使用 `disabledOpacity / 100`。

涉及 `SurfaceImpl`、`CatButton`、`CatCheckBox`、`CatRadioButton`、`CatSwitch`、`CatComboBox`、`CatSpinBox`、`CatTextField`、`CatTextArea`、`CatMenuItem`、`CatMenuBarItem`、`TabCellImpl`、`CatTableCell`、`CatTreeView`、`CatScrollBar`、`CatTitleBar`、`CatRatingControl`、`DockTabButton`、`DockTabStrip`。

## Capabilities

### New Capabilities

无。

### Modified Capabilities

- `theme`：新增状态 token 及派生、覆盖和范围契约。
- `ui-components`：页签选中背景改用 `surfaceSelected`，焦点描边改用 `focusRing`，并补充状态到 token 的映射要求。

## Impact

公开影响仅限于 `tokens` 新增键和视觉状态色变化；不新增组件属性、不改变尺寸或交互行为。应用自定义 `colorPalette` 未提供新 token 时使用内置值，`overrides` 可按类型覆盖。现有 `surfaceHover` 的悬停语义保持不变。

依赖 `fix-control-state-visibility` 已建立的步进按钮和下拉高亮可见性；该变更保持未归档时先于本变更归档。
