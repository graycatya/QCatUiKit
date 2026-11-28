# Proposal

## Why

库不随附图标字体，但多个控件用平台字体的字符绘制符号（勾选、箭头、折角、关闭、评分、步进）。这些依赖目前只散落在源码里，没有集中说明：

- 换系统字体或字体缺字时可能出现方框、宽度和基线偏移，使用者无法预知。
- 已有的绘制作法（`CatCheckBox`、`CatComboBox`、`CatTitleBar`）与字体字形的边界没有文档。
- `add-gallery-icon-font` 已把 Gallery 的图标改为随附字体，但库侧维持现状，需要明确记录，避免读者误以为库也内置字体。

## What Changes

- 在 `COMPONENTS.md` 增加「符号字形与平台字体」章节，列出使用字体字符的控件、字符、已知风险，以及已改为绘制的对照项。
- 在 `ui-components` 主规格的 Known Deviations / Technical Debt 记录该限制。

不修改任何 QML 或 C++ 实现，不新增字体资源，不改变控件行为。

## Capabilities

### New Capabilities

无。

### Modified Capabilities

无行为规约变化；只更新说明与技术债记录。

## Impact

影响 `src/uikit/doc/COMPONENTS.md` 和 `openspec/specs/ui-components/spec.md` 的技术债章节。无公开接口、构建或运行时变化。
