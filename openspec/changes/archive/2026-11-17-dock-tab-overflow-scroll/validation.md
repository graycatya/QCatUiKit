# Wayland 验证

日期：2026-11-17。Ubuntu 26.04、GNOME Wayland 会话、Qt 6.12.0 gcc_64、Debug 构建。显式设置 `QT_QPA_PLATFORM=wayland QT_QUICK_BACKEND=software`，不是 XWayland；软件渲染结果不代表 GPU 验收。

## 自动测试

使用 `build/Desktop_Qt_6_12_0-Debug` 下的测试程序和 `qml` 导入目录，直接运行而非通过 CTest 的 offscreen 环境：

- `tests/auto/docknative/tst_docknative`：5 passed、0 failed、0 skipped。日志为 `QPA: wayland`、`toplevel-drag: true`；覆盖原生缩略图捕获与窗口局部拖放事件。
- `tests/auto/qml/tst_qml -input tests/auto/qml/data/tst_dockwayland.qml -import <build>/qml`：6 passed、0 failed、0 skipped。覆盖窗口局部坐标、浮窗合并、取消恢复和移动权限。
- 同一 QML 程序运行 `tst_dock.qml`：15 passed、0 failed、0 skipped。包含溢出按钮、静止指针自动滚动、离栏返回及释放换序。首次使用旧库时为11 passed、4 failed；库重新链接后复测全部通过。
- 同一 QML 程序运行 `tst_dockfloating.qml`：5 passed、20 failed、0 skipped，不记为通过。失败包括严格警告检查捕获的 `qt.qpa.wayland.textinput` 焦点警告，以及全局坐标／显式窗口定位路径断言和空预览对象。用例中存在 `mapToGlobal`、设置浮窗 x、`updateGlobalDrag` 等操作；该结果不能直接推导真实 Wayland 拖放失败，也不能据平台专用测试通过而忽略。

原生测试需要访问当前 Wayland socket；沙箱内连接返回 Operation not permitted，获准访问桌面后正常运行。初次构建因 ccache 缓存目录只读失败，随后设置 `CCACHE_DISABLE=1` 重试。

`CCACHE_DISABLE=1 cmake --build <build> --parallel 4` 完整 Debug 构建通过；`python tools/check_docs.py`、`openspec validate dock-tab-overflow-scroll --strict`、`git diff --check` 通过。本轮未运行全套 CTest 或 qmllint。

## 未覆盖

真实鼠标触发的 `QDrag::exec()`、`xdg-toplevel-drag` 与 `startSystemMove()` 仍待人工验收，包括跨窗口／空白处拖放、浮窗标题与标签拖动、释放和 Esc。当前自动测试注入 Qt 事件，不能替代上述验收。GPU、多屏及不同 DPI 本轮未验证。
