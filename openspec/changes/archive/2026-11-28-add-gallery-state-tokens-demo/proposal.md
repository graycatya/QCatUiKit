# Proposal

## Why

主题在 `add-state-style-tokens` 中新增了 `surfacePressed`、`surfaceSelected`、`focusRing` 和 `disabledOpacity`，但 Gallery 没有体现这些能力：

- 设置页的 solarized 自定义色板仍只定义旧 token，按下和选中回退到内置 Dark 值，与自定义配色不一致。
- Gallery 没有入口让使用者对比分别定义的交互状态，设置页只演示品牌色和局部主题。
- `THEMING.md` 的 `colorPalette` 示例与完整 token 列表不一致。

## What Changes

- 为设置页的 solarized 自定义色板补齐 `surfaceSelected`、`surfacePressed` 和 `focusRing`，并通过 `overrides` 演示非默认 `disabledOpacity`。
- 新增「交互状态」面板：并排展示 `surface`、`surfaceHover`、`surfaceSelected`、`surfacePressed`、`focusRing` 色块，并提供可交互控件与禁用示例。
- 扩展 Gallery 测试，断言自定义状态色、`disabledOpacity` 与色块取值，并在清理阶段重置 `Theme.overrides`。
- 同步 `THEMING.md` 的 `colorPalette` 示例。

不新增库 API，不改变主题默认值，不改变控件行为。

## Capabilities

### New Capabilities

无。

### Modified Capabilities

- `gallery`：增加状态 token 演示与自定义色板完整性要求。

## Impact

影响 `examples/uikit/gallery/GallerySettingsPage.qml`、Gallery 翻译文件、`tests/auto/gallery/data/tst_gallery.qml` 和 `src/uikit/doc/THEMING.md`。不涉及库源码、公开 C++ 接口或安装产物。切换 Light／Dark 时清空设置页写入的 `overrides`，属于该页已有的「恢复内置色板」语义。
