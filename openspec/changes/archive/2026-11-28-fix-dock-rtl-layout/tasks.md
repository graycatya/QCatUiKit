# Tasks

## 1. 组件

- [x] 1.1 `CatDockWorkspace` 增加 `rtl` 属性，`__rtl` 在显式开关或 `LayoutMirroring` 任一为 true 时生效。
- [x] 1.2 `DockTabStrip` 在 RTL 下反转标签排列、关闭目标、滚动按钮位置与箭头方向。
- [x] 1.3 RTL 下初始显示第一个标签，滚轮与按钮滚动方向反转。
- [x] 1.4 为延迟滚动回调增加销毁保护。

## 2. Gallery 与测试

- [x] 2.1 Gallery 的 Dock 示例绑定 `rtl: GallerySettings.rtl`。
- [x] 2.2 `tst_dock.qml::test_rtlTabStrip` 断言 `__rtl`、标签顺序与关闭目标位置。
- [x] 2.3 `tst_gallery.qml::test_rtlPreview` 断言 Gallery 的 Dock 收到 RTL 状态。

## 3. 文档与验证

- [x] 3.1 `DOCKING.md` 与 `COMPONENTS.md` 记录 `rtl` 属性。
- [x] 3.2 构建并运行 `qml_contracts`、`gallery_contracts`。
- [x] 3.3 `qmllint` 修改文件、`python tools/check_docs.py`、`git diff --check`、`openspec validate fix-dock-rtl-layout --strict --no-interactive` 通过。
- [x] 3.4 用 `--rtl` 截图确认标签顺序、关闭目标与箭头位置。
- [x] 3.5 在 `validation.md` 记录命令、结果与未覆盖平台。
