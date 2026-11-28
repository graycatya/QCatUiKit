# Proposal

## Why

Gallery 开启 RTL 后，Dock 标签栏仍然按从左到右排列：标签文字与关闭目标重叠、标签被截断、滚动按钮位置与方向不对应。原因是：

- `CatDockWorkspace` 没有 RTL 状态，`DockTabStrip` 无法判断方向。
- Gallery 的 `LayoutMirroring` 在 `ScrollView` 后没有传递到 `CatDockWorkspace`，仅靠继承不可靠。
- `DockTabStrip` 的按钮位置、关闭目标、标签排列和滚动方向全部写死为 LTR。

## What Changes

- `CatDockWorkspace` 增加 `rtl` 属性；`__rtl` 在显式 `rtl` 或 `LayoutMirroring` 任一为 true 时生效。
- `DockTabStrip` 在 RTL 下：标签从右到左排列、关闭目标移到标签左端、滚动按钮交换位置与图标方向、初始显示第一个标签、滚轮和按钮滚动方向反转。
- Gallery 的 Dock 示例绑定 `rtl: GallerySettings.rtl`。
- 增加 `test_rtlTabStrip`，断言 RTL 下标签顺序、关闭目标位置和 `__rtl` 状态。
- 更新 `DOCKING.md` 与 `COMPONENTS.md`。

不改变 LTR 布局、标签宽度、活动状态、布局保存或拖动换序的数据语义。

## Capabilities

### New Capabilities

无。

### Modified Capabilities

- `docking`：R06 `TabOverflowScroll` 增加 RTL 标签栏契约。

## Impact

影响 `src/uikit/qml/CatDockWorkspace.qml`、`DockTabStrip.qml`、`examples/uikit/gallery/GalleryDockPreview.qml`、`tests/auto/qml/data/tst_dock.qml`、`tests/auto/gallery/data/tst_gallery.qml` 和两份文档。公开 API 新增一个可选 `rtl` 属性。
