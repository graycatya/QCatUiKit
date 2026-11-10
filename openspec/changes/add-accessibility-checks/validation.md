# Validation

执行环境：Windows 11，Qt 6.12.0 MSVC 2022 x64，Release 单配置，Ninja，`build/Desktop_Qt_6_12_0_MSVC2022_64bit_Release`。

## 控件语义检查

`collectSemanticFailures()` 通过临时外部运行器执行，避开本机无法启动的 Qt Quick Test 运行器：

```sh
semanticcheck <build>/qml tests/auto/qml/data/tst_accessibility.qml collectSemanticFailures
```

结果：`RESULT: PASS (0 failure(s))`，退出码 0。覆盖按钮、复选框、单选框、开关、菜单项的名称与超链接的 Link 角色。

静态检查：`tst_accessibility.qml` 通过 `qmllint -I <build>/qml`，退出码 0。

## Qt 进程内接口检查

构建：`cmake --build <build> --target tst_accessibility` 通过。

首次验证在 Windows、`QT_QPA_PLATFORM=offscreen`、software 后端运行：

```sh
tst_accessibility -o report.txt,txt
```

首次报告：`SKIP: platform 'offscreen' has no accessibility bridge`，`Totals: 0 passed, 0 failed, 1 skipped`。进程正常退出，没有出现 `0xC0000005`。该历史结果只证明 QtTest 跳过路径有效，不证明 CTest 正确分类或桥接可用；原报告中的桥接说法已在审查修正中纠正。

## 审查修正验证（2026-11-10）

使用上述 Windows／Qt／Release 构建目录，启用 `QCATUIKIT_TEST_ACCESSIBILITY=ON` 后重新配置并构建 `tst_accessibility` 成功。CTest 测试名改为 `accessibility_interfaces`；以下命令中的 `<build>` 代表该构建目录：

```sh
ctest --test-dir <build> -R '^accessibility_interfaces$' --output-on-failure --output-junit accessibility-review-offscreen.xml
```

分别设置 `QT_QPA_PLATFORM=offscreen`、`minimal` 与 `windows`，均使用 `QT_QUICK_BACKEND=software`：

| 平台 | CTest 结果 | JUnit 证据 |
| --- | --- | --- |
| offscreen | Skipped | `skipped=1`，`SKIP_REGULAR_EXPRESSION_MATCHED` |
| minimal | Skipped | `skipped=1`，`SKIP_REGULAR_EXPRESSION_MATCHED` |
| windows | 1/1 Passed | `failures=0`、`skipped=0`；名称／角色、press 动作与菜单打开均通过 |

Windows 首次原生执行时，从普通 QML Item 根节点查询找不到溢出按钮，两条断言失败；CTest 正确标为 Failed，未误判为 Skipped。将查询入口修正为 QQuickView 窗口接口后通过。此检查只遍历 Qt 进程内接口并直接调用动作，未验证操作系统桥接或屏幕阅读器。

将选项恢复为原有 `OFF` 并重新配置后，`ctest --test-dir <build> -N -R '^accessibility_(interfaces|bridge)$'` 显示 `Total Tests: 0`。

本轮 `python tools/check_docs.py`（38 个 Markdown、528 个本地链接）、`openspec validate add-accessibility-checks --strict --no-interactive`、`git diff --check` 全部通过。未重新执行全量 CTest、QML lint 或命令栏 Qt Quick Test；本轮没有改动 QML 控件与用例。

## 未覆盖

- Linux 与 macOS 的进程内接口检查尚未执行；所有平台的操作系统桥接与外部辅助技术客户端均未验证。
- 未验证库级翻译下发；接口检查按英文源字符串断言。
- 未覆盖输入类控件：其名称需要应用提供，不能由组件自行断言。
