# Tasks

## 1. 组件

- [x] 1.1 `CatDockWorkspace` 增加 `tabScrollLeftIcon`、`tabScrollRightIcon` 两个可选 `Component` 属性。
- [x] 1.2 `DockTabStrip` 增加对应属性，在按钮内容中优先显示覆盖组件，默认保留绘制 chevron。
- [x] 1.3 `CatDockWorkspace` 与 `DockFloatingWindow` 的主标签栏把配置传给 `DockTabStrip`。
- [x] 1.4 保持按钮宽度、可用状态、滚动步长和拖动自动滚动不变。

## 2. Gallery

- [x] 2.1 Dock 示例用 RemixIcon 的 `chevronLeft` / `chevronRight` 覆盖两个箭头。

## 3. 测试

- [x] 3.1 `tst_dock.qml` 断言默认绘制图标可见。
- [x] 3.2 提供组件后断言默认图标隐藏、自定义组件存在，滚动按钮仍可用。

## 4. 文档与验证

- [x] 4.1 `COMPONENTS.md` 与 `DOCKING.md` 记录两个新属性。
- [x] 4.2 构建并运行 `qml_contracts`、`gallery_contracts`。
- [x] 4.3 `qmllint` 修改文件、`python tools/check_docs.py`、`git diff --check`、`openspec validate add-dock-scroll-icons --strict --no-interactive` 通过。
- [x] 4.4 在 `validation.md` 记录命令、结果、视觉检查与未覆盖平台。
