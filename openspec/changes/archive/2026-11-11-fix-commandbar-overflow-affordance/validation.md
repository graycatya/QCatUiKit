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

## Linux 补验（2026-11-11）

环境：Ubuntu 26.04、Linux x86_64、Qt 6.12.0 gcc_64、Release、Ninja，构建目录 `build-release`。

```sh
cmake --build build-release --target tst_qml --parallel 2
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software build-release/tests/auto/qml/tst_qml -import "$PWD/build-release/qml" -input "$PWD/tests/auto/qml/data/tst_commandbar.qml"
```

构建成功；定向测试 **5 passed、0 failed、0 skipped**（含初始化与清理）。三个行为用例分别验证字形空间、无次要操作时隐藏和点击打开菜单及菜单项数量。

构建完成前试跑旧产物时，三个用例均因找不到新增 objectName 失败；当前源码构建完成后使用相同命令全部通过。此前 Windows 运行器问题仍是历史环境限制，不能据此判断 Linux 运行结果。

随后执行 `ctest --test-dir build-release -R '^qml_contracts$' --output-on-failure`：**1/1 Passed**，35.07 s；覆盖完整 QML 契约组，包含命令栏及无障碍语义用例。本轮未执行全量 CTest 或重新采集视觉截图。

本轮最终 `python tools/check_docs.py`（38 个 Markdown、528 个本地链接）、本变更 OpenSpec 严格校验及 `git diff --check` 均通过。未执行全量 CTest；本轮仅更新验证文档和任务状态。

## 归档核查（2026-11-11）

差异规约已同步至对应主规格，并逐条核对需求与场景。主规格严格校验 14/14 通过，`python tools/check_docs.py` 与 `git diff --check` 通过。本次仅同步规约及归档，保留此前验证证据和未覆盖范围，未重新执行产品测试。


## macOS 补验（2026-11-11）

环境：macOS 26.6.2、arm64、Qt 6.10.2、Release、Ninja。构建当前源码的 `tst_accessibility`、`tst_qml` 后，在允许访问桌面会话的 Cocoa 原生后端运行：

```sh
QT_QPA_PLATFORM=cocoa QT_QUICK_BACKEND=software <build>/tests/auto/qml/tst_qml -import <build>/qml -input "$PWD/tests/auto/qml/data/tst_commandbar.qml" -o /tmp/qcat-macos-commandbar.txt,txt
```

结果：**5 passed、0 failed、0 skipped**（含初始化与清理），验证字形空间、无次要操作时隐藏、鼠标点击打开菜单及项数。原生执行有 `IMKCFRunLoopWakeUpReliable` 系统诊断输出，未导致测试失败。

同轮 Cocoa `accessibility_interfaces` 为 **1/1 Passed**（1.09 s），覆盖溢出按钮名称、角色及 press 动作打开菜单；完整离屏 `qml_contracts` 为 **1/1 Passed**（34.43 s）。无障碍接口范围与沙箱限制见 [无障碍补验记录](../2026-11-11-add-accessibility-checks/validation.md#macos-补验2026-11-11)。

本轮未重新采集明暗／密度视觉截图，未补验键盘焦点环、RTL、长菜单滚动或全量 CTest。

最终核查：`python3 tools/check_docs.py`（38 个 Markdown、528 个本地链接）、主规约严格校验（14/14）及 `git diff --check` 均通过。
