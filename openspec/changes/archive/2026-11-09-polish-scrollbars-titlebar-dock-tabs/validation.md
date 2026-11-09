# Validation Record

## 2026-11-09 Windows 离屏验证

环境：Windows 11、Qt 6.12.0 MSVC 2022 64-bit、Release、Ninja，构建目录 `build-led-release`。QML 测试使用 `QT_QPA_PLATFORM=offscreen`、`QT_QUICK_BACKEND=software`、`QSG_RHI_BACKEND=software`；截图由 `qcat-gallery --screenshot` 在相同后端生成。未使用真实 GPU 渲染或实机指针。

本轮改动先在工作区完成实现，随后补建本 change 并重新执行下列检查。

## 构建与静态检查

- `cmake --build build-led-release --parallel`：通过，生成 `QCatUiKit.dll` 与 Gallery 资源。
- `cmake --build build-led-release --target QCatUiKit_qmllint qcat-gallery_qmllint`：退出 0，无 lint 诊断。
- `python tools/check_docs.py`：通过，检查 38 个 Markdown 文件、528 个本地链接、代码围栏、重复正文及 123 个公开 Cat 类型。
- `git diff --check`：退出 0，无空白错误。

## 自动化测试

- `ctest --test-dir build-led-release -R '^qml_contracts$' --output-on-failure`：1/1 通过，35.48 秒。
- `ctest --test-dir build-led-release -R '^gallery_contracts$' --output-on-failure`：1/1 通过，117.29 秒；包含 `tests/auto/gallery/data/tst_dock.qml` 的停靠示例回归。
- `ctest --test-dir build-led-release -R '^(gallery_smoke|gallery_contracts|dock_controller)$'`：3/3 通过（`gallery_smoke` 1.28 秒、`gallery_contracts` 130.60 秒、`dock_controller` 0.21 秒）。
- `tst_qml -input tests/auto/qml/data/tst_scrolling.qml -import build-led-release/qml`：32 passed，0 failed，8.3 秒。
- 定向运行 `Scrolling::test_scrollBarPolicy Scrolling::test_textSelectionAndScrollBar`：4 passed，0 failed。

补充调试用例（临时文件已删除，不作为仓库测试）：

- CatScrollView 滚动条尺寸：水平 `size=1`、垂直 `size=0.16`，确认 contentItem 延迟连接未被破坏；仅有 HoverHandler、未安装 `MouseScrollImpl` 的对照实例为 `size=0`。
- 指针进入/离开：`pointerInside` 由 false→true→false，滚动条 `opacity` 由 0→1→0，隐藏状态仍保持 `visible=true` 以保留命中区。
- 停靠标签几何：四字标题标签宽 72，关闭按钮 `x=74`、宽 26，单元格宽 102；重叠检查确认单元格背景覆盖标题与关闭按钮。

## 视觉检查

使用离屏截图人工核对，临时 PNG 已删除：

- Gallery 首页：侧栏静止时无滚动条；导航内容超出时滚动条不占据布局宽度。
- 停靠工作区（亮色/暗色）：活动标签表面色背景与 2px 主色下划线覆盖标题和关闭入口；非活动标签在灰色标签条上保持可读；关闭按钮贴近标题，无独立方块感。
- 标题栏（亮色/暗色）：最小化细线、最大化圆角方框、关闭圆角叉号清晰；关闭悬停为通栏危险色背景和浅色叉号，静止状态无红色边框。
- 停靠关闭悬停：整格高亮，关闭入口显示圆角淡红底和红色叉号；按下状态使用更深红色。

## Gallery 内悬停复验

使用 `tst_gallery` 加载真实 Gallery 窗口执行临时用例（检查后删除）：

- 侧栏：`galleryNavigationScroll` 的垂直滚动条 `size=0.74`，静止 `opacity=0`；指针进入后 `_pointerInside=true`、`opacity=1`；移出后恢复 `0`。局部截图对比确认静止无滑块、悬停出现滑块。
- 标题栏：定位 `title-bar` 页面中可见的内联 `CatTitleBar`，悬停关闭按钮后 `hovered=true`；截图确认危险色通栏背景和浅色叉号。
- 停靠：悬停 `dockTabClose_project` 后 `hovered=true`；截图确认活动标签表面色背景和主色下划线覆盖关闭入口，关闭入口显示圆角淡红底。
- 结果：5 passed，0 failed，约 0.9 秒。

## 未覆盖范围

- Shape 图标的抗锯齿只在 software 光栅化下检查；D3D11、Metal、Vulkan/OpenGL 路径未逐项截图。
- 悬停由 QtTest 合成鼠标事件触发，未使用真实硬件指针；未验证触摸板压力、笔输入或远程桌面。
- 未验证 macOS、Linux X11/Wayland 的窗口管理器装饰共存场景；`CatTitleBar` 为自绘控件，不依赖原生标题栏，但未做跨平台实机截图。
- 未覆盖 RTL、高 DPI 分数缩放、触摸密度下关闭入口的按下外观，以及宿主完全替换标题栏背景的局部主题组合。
- 未建立像素级截图基线，视觉结论来自人工检查。

## 结论

R06、R07 与 R05 的可观察行为在 Windows 离屏环境通过自动化与视觉检查；未覆盖平台和输入方式不据此宣告通过。后续归档前如有平台实机要求，应在对应环境补充证据后再更新结论。
