# 验证执行历史

可重复步骤见 [VALIDATION](VALIDATION.md)。变更的详细证据仍随对应 OpenSpec change 保存。

## 2026-10-29 macOS 桌宠探针

独立 Release 构建、探针 CTest 2/2、MacosProbe QML lint、OpenSpec 严格验证及差异空白检查通过。Cocoa 窗口检查 8 项 Passed，焦点基线及真实输入 Pending；系统事件发送权限不可用。完整 macOS 平台验收未完成，任务 1.3 保留未勾选。详细环境、命令、计数和限制见 [变更证据](../../../openspec/changes/add-mascot-pet/validation.md)。

## 2026-10-29 macOS 原型继续验收

用户授权临时辅助功能权限并完成系统验证后，通过 LaunchServices 运行最终构建，真实输入 19/19 通过；人工确认菜单栏隐藏／恢复／穿透和退出请求、区域之间显示背后窗口，原始菜单记录 9 次动作／2 次退出请求且宿主仍运行。最终已关闭探针临时辅助功能权限并确认进程无残留。独立 CTest 2/2（0.08 秒）、QML lint、格式检查通过；任务 1.3 完成，完整 macOS 平台矩阵仍 Pending。失败尝试、复验、命令及证据范围见 [变更证据](../../../openspec/changes/add-mascot-pet/validation.md)。
