# Proposal

## Why

RTL 下 `DockTabStrip` 已把关闭目标移到标签左端，但仍沿用 LTR 的 `leftPadding: 4`，标签文字从 4 像素处开始，被 28 像素宽的关闭目标覆盖，视觉上出现文字与关闭目标重叠。同时 `cell.width` 在 RTL 下把关闭目标宽度又加了一次，造成标签之间多出空隙。

这是 `fix-dock-rtl-layout` 之后的遗留缺陷：方向正确，但内边距与宽度计算没有随关闭目标位置调整。

## What Changes

- RTL 下 `leftPadding` 改为 `closeButton.width + 4`，为左侧关闭目标预留完整空间。
- RTL 下 `cell.width` 不再额外叠加关闭目标宽度；LTR 保持原计算。
- 扩展 `test_rtlTabStrip`，断言 `leftPadding` 覆盖关闭目标宽度。
- 补充 R06 的 RTL 契约：关闭目标不得覆盖文字，宽度不得重复计入。

不改变 LTR 布局、滚动、拖动换序或关闭行为。

## Capabilities

### New Capabilities

无。

### Modified Capabilities

- `docking`：R06 `TabOverflowScroll` 补充 RTL 标签内边距与宽度约束。

## Impact

影响 `src/uikit/qml/DockTabStrip.qml`、`tests/auto/qml/data/tst_dock.qml`。公开 API 不变。
