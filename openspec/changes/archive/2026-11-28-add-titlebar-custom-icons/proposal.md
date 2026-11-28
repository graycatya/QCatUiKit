# Proposal

## Why

`CatTitleBar` 的最小化、最大化、还原和关闭图标目前固定为内置 `Shape` / `Rectangle` 绘制，宿主无法替换。Gallery 的标题栏示例因此只能显示绘制样式，不能演示图标字体或其他视觉风格；`CatCheckBox`、`CatComboBox` 等已经采用绘制实现，但它们的图标语义固定，也没有必要开放。

标题栏是宿主品牌差异最明显的区域之一，提供可选图标组件比把字体引入库更合适：库仍不依赖字体，Gallery 可以用随附的 RemixIcon 演示。

## What Changes

- `CatTitleBar` 增加四个 `Component` 属性：`minimizeIcon`、`maximizeIcon`、`restoreIcon`、`closeIcon`。未提供时保持现有绘制，公开属性和窗口操作不变。
- Gallery 的 `title-bar` 示例用 RemixIcon 覆盖四个图标，并在示例代码中展示用法。
- 增加 QML 测试：默认绘制仍可见；提供自定义组件后替换默认图标。
- 更新 `COMPONENTS.md` 的标题栏说明。

不改变按钮尺寸、可访问名称、危险色语义、焦点描边或点击行为。

## Capabilities

### New Capabilities

无。

### Modified Capabilities

- `ui-components`：R07 `WindowCaptionButtons` 增加可覆盖图标的契约。

## Impact

影响 `src/uikit/qml/CatTitleBar.qml`、`tests/auto/qml/data/tst_visualstates.qml`、`examples/uikit/gallery/GalleryExtendedPreview.qml`、`GalleryCatalog.qml`、`GalleryIcons.qml`、`src/uikit/doc/COMPONENTS.md` 和 Gallery 翻译。库仍不随附图标字体。
