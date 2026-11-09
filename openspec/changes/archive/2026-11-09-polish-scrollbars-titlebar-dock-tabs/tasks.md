# Tasks

## 1. 叠加滚动条

- [x] 1.1 为 `CatScrollBar` 增加 `revealed`、按下/活动/悬停合成的显示状态和透明度淡出，保留 `AlwaysOn` 与独立使用常驻行为；验证：`Scrolling::test_scrollBarPolicy` 与 `Scrolling::test_textSelectionAndScrollBar` 通过，完整 `tst_scrolling.qml` 32 passed。
- [x] 1.2 在 `CatScrollView` 的 contentItem 创建后安装视口 `HoverHandler`，并把水平和垂直滚动条绑定到指针状态；验证：QML 用例确认滚动条 `size` 为 1/0.16 而非 0，指针移入/移出时 `opacity` 在 0 与 1 之间切换，`tst_scrolling.qml` 通过。

## 2. 标题栏窗口按钮

- [x] 2.1 用绘制图标替换最小化、最大化/还原、关闭的字符文本，增加 `maximized` 状态与还原图标，验证：`QCatUiKit_qmllint` 通过，离屏渲染截图中三个图标清晰且最大化状态切换绑定正确。
- [x] 2.2 实现关闭按钮危险色悬停/按下、最小化/最大化 `surfaceHover`、键盘焦点描边和禁用色，并移除 flat 危险按钮的常驻红边；验证：QML 悬停用例截图核对红底白叉与静止无色边框，`qml_contracts` 通过。

## 3. 停靠标签单元格

- [x] 3.1 将主工作区和浮动窗口的标签 delegate 改为共享背景的单元格，活动下划线覆盖标题与关闭入口，关闭按钮改为 26×24 内嵌工具按钮；验证：`gallery_contracts` 通过，亮暗截图显示下划线覆盖整格且关闭按钮无脱离感，几何检查关闭按钮位于标签右侧。
- [x] 3.2 调整 `DockTabButton` 工具按钮的内缩圆角悬停、`Shape` 叉号和标签最小宽度；验证：`QCatUiKit_qmllint` 通过，标签悬停/关闭悬停截图显示整格高亮和圆角危险色反馈。

## 4. 文档

- [x] 4.1 更新 `src/uikit/doc/COMPONENTS.md` 的 `CatScrollBar` 与 `CatScrollView` 行为说明；验证：`python tools/check_docs.py` 通过。

## 5. 集成检查

- [x] 5.1 完成 Release 构建、库与 Gallery qmllint、`qml_contracts`、`gallery_contracts`、`tst_scrolling.qml`、文档检查和 `git diff --check`；结果记录于 validation.md。
- [x] 5.2 检查亮色与暗色 Gallery 截图：侧栏滚动条静止隐藏、标题栏图标、停靠标签及关闭悬停反馈；记录未覆盖的硬件与平台范围。

## Workflow follow-up

- 在项目评审确认后归档 change，并同步 `ui-components` 与 `docking` 主规格。
- 归档后运行 `openspec validate --specs --strict --no-interactive` 并核对主规格中的新增 Requirement。
