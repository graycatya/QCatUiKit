# Tasks

## 1. 组件

- [x] 1.1 `CatTitleBar` 增加 `minimizeIcon`、`maximizeIcon`、`restoreIcon`、`closeIcon` 四个 `Component` 属性。
- [x] 1.2 四个按钮在属性为 null 时保留现有绘制，提供组件时用 `Loader` 替换显示。
- [x] 1.3 保持按钮尺寸、辅助名称、状态背景、焦点描边和窗口操作不变。

## 2. Gallery

- [x] 2.1 `GalleryIcons` 增加 `minimize`、`maximize`、`restore` 码位（已渲染核对）。
- [x] 2.2 `GalleryExtendedPreview` 的 `title-bar` 示例用 RemixIcon 覆盖四个图标。
- [x] 2.3 `GalleryCatalog` 的示例代码展示自定义图标写法。

## 3. 测试

- [x] 3.1 `tst_visualstates.qml` 断言默认绘制图标可见。
- [x] 3.2 提供自定义组件后断言默认图标隐藏、自定义组件存在，且按钮尺寸与点击行为不变。

## 4. 文档与验证

- [x] 4.1 `COMPONENTS.md` 的标题栏说明补充可覆盖图标。
- [x] 4.2 构建并运行 `qml_contracts`、`gallery_contracts`。
- [x] 4.3 `qmllint` 修改文件、`python tools/check_docs.py`、`git diff --check`、`openspec validate add-titlebar-custom-icons --strict --no-interactive` 通过。
- [x] 4.4 在 `validation.md` 记录命令、结果、视觉检查与未覆盖平台。
