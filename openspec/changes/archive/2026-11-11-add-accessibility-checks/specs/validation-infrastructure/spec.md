# validation-infrastructure Specification (delta)

## ADDED Requirements

### Requirement: R05 AccessibilityOptIn

原生窗口下的 Qt 进程内无障碍接口检查 SHALL 通过独立选项注册；未启用时 SHALL NOT 运行。通过结果 SHALL NOT 表示操作系统桥接或外部辅助技术已经验证。

#### Scenario: 默认关闭

- WHEN 未设置 `QCATUIKIT_TEST_ACCESSIBILITY=ON`
- THEN 系统 SHALL NOT 注册 `accessibility_interfaces`，其他测试的通过结果 SHALL NOT 表示无障碍桥接已验证。

#### Scenario: 选项开启

- WHEN 配置 `QCATUIKIT_TEST_ACCESSIBILITY=ON` 且启用测试
- THEN 系统 SHALL 构建并注册 `accessibility_interfaces`，并加载构建树中的 QCatUiKit 模块。

#### Scenario: 不支持的平台

- WHEN 运行 `accessibility_interfaces` 且当前平台不在 `windows`、`cocoa`、`xcb`、`wayland` 之中
- THEN 套件 SHALL 在查询前跳过并在报告中写明平台名，CTest SHALL 将其标为 Skipped，SHALL NOT 崩溃，也 SHALL NOT 记为通过。

#### Scenario: 原生窗口平台

- WHEN 在支持的原生窗口平台运行 `accessibility_interfaces`
- THEN 套件 SHALL 查询 Qt 进程内接口并断言溢出按钮的名称、角色与按压动作，并 SHALL 验证在进程内调用按压动作后菜单进入已打开状态。结果 SHALL NOT 证明平台桥接已经启用或屏幕阅读器可以访问。

#### Scenario: 加载来源

- WHEN 运行任何无障碍检查
- THEN 加载的 QCatUiKit 模块 SHALL 来自构建目录；命中系统已安装副本时 SHALL 视为环境错误而非通过。
