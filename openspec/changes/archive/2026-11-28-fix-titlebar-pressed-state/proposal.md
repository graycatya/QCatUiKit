# Proposal

## Why

`CatTitleBar` 的最小化和最大化按钮按下时使用 `tokens.disabled`（禁用灰），而普通表面控件按下使用 `surfacePressed`。`add-state-style-tokens` 的验证记录已把这一处列为未覆盖项。结果是同一窗口内，标题栏按钮按下与普通按钮按下呈现不同的语义：看起来像被禁用，而不是被按下。

关闭按钮保持危险色语义，不属于该问题。

## What Changes

- 最小化和最大化按钮按下背景从 `disabled` 改为 `surfacePressed`。
- 增加 QML 测试，断言两个按钮的静止、悬停、按下背景，以及关闭按钮的危险色行为不变。
- 在 `ui-components` 的 R07 中明确最小化/最大化按钮按下使用 `surfacePressed`。

不改变按钮尺寸、图标、关闭按钮的危险色、焦点描边或窗口操作。

## Capabilities

### New Capabilities

无。

### Modified Capabilities

- `ui-components`：R07 `WindowCaptionButtons` 补充最小化/最大化按钮的按下背景要求。

## Impact

影响 `src/uikit/qml/CatTitleBar.qml`、`tests/auto/qml/data/tst_visualstates.qml` 和 `openspec/specs/ui-components/spec.md`（归档时）。公开属性和窗口操作不变。
