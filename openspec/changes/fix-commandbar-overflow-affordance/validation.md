# Validation

执行环境：Windows 11，Qt 6.12.0 MSVC 2022 x64，Release 单配置，Ninja，`build/Desktop_Qt_6_12_0_MSVC2022_64bit_Release`。

## 自动测试

新增 `tests/auto/qml/data/tst_commandbar.qml`，断言可用宽度为正、无次要操作时隐藏、点击后菜单打开且项数为 1。

本机 `tst_qml` 运行器无法启动：即使把 `CatCommandBar.qml` 回退到 HEAD 重新构建，仍在 `-input tst_controls.qml` 时以 `0xC0000409` 崩溃且无输出，属于运行环境问题。因此该用例**尚未在测试套件中执行**。

替代证据：用临时 C++ 运行器加载同一场景，按相同断言检查——

- 溢出按钮存在，文本为 `⋯`，`availableWidth = 32`；
- 无次要操作时按钮不可见；
- 鼠标点击后菜单 `opened` 变为真（等待 600ms，覆盖 160ms 进入动画），菜单项数为 1。

## 静态检查

- `QCatUiKit_qmllint` 通过（退出码 0）。
- `tst_commandbar.qml` 单独 `qmllint -I <build>/qml` 通过（补充 `pragma ComponentBehavior: Bound` 消除嵌套组件访问外部 id 的警告）。

## 视觉检查

增量构建后使用 `qcat-gallery --page command-bar --screenshot` 采集：

- 亮色桌面：`打开 保存 ⋯`，省略号居中、无边框方块。
- 暗色桌面：同上，省略号可辨识。
- 触摸密度与 420 窄屏移动密度：按钮仍为正方形，省略号未贴边。

限制：截图来自 Gallery 预览区，不是完整页面；未在 macOS／Linux 原生后端复核；未验证键盘焦点环与 RTL。

## 未覆盖

- 完整 `ctest` 与 `python tools/check_docs.py` 未在本轮执行。
- 菜单项过多时的溢出或滚动未验证。
