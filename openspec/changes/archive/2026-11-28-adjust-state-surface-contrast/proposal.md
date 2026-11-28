# Proposal

## Why

内置 Light 色板的 `surfaceHover` 为 `#f0f5ff`，与纯白 `surface` 的差异只有约 3%，悬停几乎不可见；Dark 的 `surfaceHover` 为 `#303030`，与 `#1f1f1f` 的差距也偏弱。状态色虽然已经分开定义，但默认值的可辨识度不足，`R18 ControlStateVisibility` 的目标在实际默认外观下仍难达到。

## What Changes

- 加深 Light 的三级交互底色：`surfaceHover` `#e9f2ff`、`surfaceSelected` `#dbe8ff`、`surfacePressed` `#c6dcff`。
- 加深 Dark 的三级交互底色：`surfaceHover` `#333333`、`surfaceSelected` `#2b3a52`、`surfacePressed` `#3f5470`。
- 在 `theme` 规约中增加可测下限：每个交互状态与 `surface` 的相对亮度差 SHALL 不小于 0.015，且相邻状态单调变化。
- 更新 C++ 测试的默认值断言，并新增相对亮度阈值检查。

不新增 token、不改变 `focusRing` 派生规则、不改变组件的状态映射；只调整内置默认颜色。应用显式提供的 `colorPalette` 或 `overrides` 不受影响。

## Capabilities

### New Capabilities

无。

### Modified Capabilities

- `theme`：修改 R06 `StateTokens` 的内置默认值与可辨识下限。

## Impact

影响 `src/uikit/theme/qcatthemepalette.cpp`、`tests/auto/models/tst_models.cpp`、`tests/auto/gallery/data/tst_gallery.qml` 和 `src/uikit/doc/THEMING.md`。使用内置 Light/Dark 的应用会看到更明显的悬停、选中和按下背景，属于默认视觉变化；自定义色板的应用保持原值。
