# Wayland 离栏拖动与预览裁剪修复

日期：2026-11-17。修复标签离开原栏后仍要越过整个软件窗口才启动原生拖动的问题；无顶层拖动协议时，局部预览改放窗口覆盖层，保留工作区外的系统移动降级。非 Wayland 路径不变。

## 定向验证

- Qt 6.12.0 gcc_64 Release 与 Debug，完整构建通过。
- 库与 Gallery qmllint 通过。
- `ctest --test-dir <Release build> --output-on-failure -j 2`：21/21 通过，耗时 138.70 秒，包括 QML、Gallery、Dock 控制器与原生适配器契约。
- `QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software <build>/tests/auto/qml/tst_qml -input tests/auto/qml/data/tst_dockwayland.qml -import <build>/qml`：7 passed、0 failed、4 skipped。新增分支测试覆盖窗口内部离栏、栏内换序、无协议降级、释放保护；覆盖层测试检查坐标与工作区销毁后预览清理。
- 同一命令改用 `QT_QPA_PLATFORM=wayland`，连接当前桌面：11 passed、0 failed、0 skipped。原有窗口局部坐标、取消恢复、跨浮窗合并与权限检查通过。启动时有一条 Qt Wayland textinput 焦点诊断，不在测试用例执行期间；未屏蔽警告。
- 沙箱内首次 Wayland 启动因 `wl_display (Operation not permitted)` 失败，获准访问桌面后重试通过。
- `python tools/check_docs.py` 与 `git diff --check` 通过。

## 人工验收范围

用户在修复后确认原生 Wayland 拖动效果正常（“可以完美”），并进一步确认“dock的我测试ok了”，记为用户实测通过。代理未单独执行真实鼠标驱动的合成器视觉验收，用户未逐项报告拖回、Esc 等操作，因此不推断每个手工场景均已独立验收。

无顶层拖动协议的降级分支使用替身测试，未连接不支持该协议的第二种合成器。用户此前验证 xcb、macOS、Windows 正常；修复后未重新进行这些平台的实机验证，亦未覆盖 GPU 与不同 DPI 多屏。可重复步骤见 `src/uikit/doc/VALIDATION.md`。
