# Proposal

## Why

Dock 标签栏的左右滚动按钮固定使用 `Shape` 绘制的 chevron，应用无法替换。Gallery 的 Dock 示例因此不能像标题栏那样使用随附的 RemixIcon，宿主也不能按自己的图标体系调整。

`CatTitleBar` 已经提供了 `Component` 覆盖模式，Dock 应保持一致：库不依赖字体，应用按需传入图标。

## What Changes

- `CatDockWorkspace` 增加 `tabScrollLeftIcon`、`tabScrollRightIcon` 两个 `Component` 属性，默认 null。
- `DockTabStrip` 在标签溢出时用 `Loader` 显示覆盖图标；未提供时继续绘制内置 chevron。浮动窗口继承同一设置。
- Gallery 的 Dock 示例用 RemixIcon 覆盖两个箭头。
- 增加 QML 测试：默认绘制可见；提供组件后替换，滚动按钮尺寸与滚动行为不变。
- 更新 `COMPONENTS.md` 与 `DOCKING.md`。

不改变滚动阈值、标签排列、拖动换序、浮动窗口或布局保存。

## Capabilities

### New Capabilities

无。

### Modified Capabilities

- `docking`：R06 `TabOverflowScroll` 增加滚动按钮图标可覆盖的契约。

## Impact

影响 `src/uikit/qml/CatDockWorkspace.qml`、`DockTabStrip.qml`、`DockFloatingWindow.qml`、`examples/uikit/gallery/GalleryDockPreview.qml`、`tests/auto/qml/data/tst_dock.qml` 和两份文档。公开 API 仅新增两个可选属性。
