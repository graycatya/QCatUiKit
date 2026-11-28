# Proposal

## Why

Gallery 的表格示例用系统字体字符 `"◀"`、`"▶"` 表示列移动。这类字形依赖平台默认字体，字重、基线和对齐在不同系统上不一致；按钮宽度设为 28 逻辑像素却沿用默认 12 像素内边距，可用内容区只剩 4 像素，字形靠溢出绘制，同时与 32 逻辑像素高的复选框不在一条水平线上。

仓库已取得 RemixIcon 字体文件 `examples/uikit/gallery/assets/remixicon.ttf`。用它替换示例中的文本字形，可以消除字体依赖并统一样式。

## What Changes

- 将 `assets/remixicon.ttf` 纳入 Gallery QML 模块资源。
- 应用启动时同步注册字体，避免首帧字形回退为占位方框。
- 新增 `GalleryIcons` 单例：集中声明图标码位与字体族，调用处不再出现字符或魔法码点；单例内保留 `FontLoader`，使不经过应用启动入口的宿主（如自动测试）也能解析字体。
- 表格列移动按钮改为图标按钮：尺寸取 `controlHeight`，内边距为 0，图标来自随附字体。
- 登记第三方许可：`LICENSES/RemixIcon-1.0.txt` 与 `NOTICE` 条目。
- 增加测试：首列与末列的移动按钮启用状态、图标字体族与按钮尺寸。

## Capabilities

### New Capabilities

无。

### Modified Capabilities

- `gallery`: 增加示例图标渲染、字体加载与第三方资源登记要求。

## Impact

影响 `examples/uikit/gallery/CMakeLists.txt`、`main.cpp`、`GalleryDataPreview.qml`、新增 `GalleryIcons.qml`、`tests/auto/gallery/data/tst_data.qml`、`LICENSES/RemixIcon-1.0.txt`、`NOTICE`。

不改变库的公开 API、QML 类型或 C++ 接口。字体只随 Gallery 示例分发，不作为 QCatUiKit 的图标资源或库 API 的一部分。

许可影响：RemixIcon 采用 Remix Icon License v1.0（2026-01），允许在 UI kit 和设计系统中作为次要组件使用，禁止作为独立图标库分发或用作 Logo。字体保持该许可，不回授为 Apache-2.0。

## 待确认

- 库内部仍使用系统字形的位置（`CatMenuItem` 的勾选与箭头、`CatMultiSelectComboBox` 与 `CatDropDownButton` 的下折角、`CatStepper` 的勾、`CatBreadcrumb` 与 `CatCalendar` 的箭头、`CatAlert` 的叉号）不在本次范围。是否把字体引入库需要单独决策：随库分发会增加约 600 KB 体积并受上述许可约束，另一种做法是改为绘制图形。
