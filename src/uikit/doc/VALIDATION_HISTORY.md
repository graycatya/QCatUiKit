# 验证执行历史

可重复步骤见 [VALIDATION](VALIDATION.md)。变更的详细证据仍随对应 OpenSpec change 保存。

## 2026-10-29 macOS 桌宠探针

独立 Release 构建、探针 CTest 2/2、MacosProbe QML lint、OpenSpec 严格验证及差异空白检查通过。Cocoa 窗口检查 8 项 Passed，焦点基线及真实输入 Pending；系统事件发送权限不可用。完整 macOS 平台验收未完成，任务 1.3 保留未勾选。详细环境、命令、计数和限制见 [变更证据](../../../openspec/changes/add-mascot-pet/validation.md)。

## 2026-10-29 macOS 原型继续验收

用户授权临时辅助功能权限并完成系统验证后，通过 LaunchServices 运行最终构建，真实输入 19/19 通过；人工确认菜单栏隐藏／恢复／穿透和退出请求、区域之间显示背后窗口，原始菜单记录 9 次动作／2 次退出请求且宿主仍运行。最终已关闭探针临时辅助功能权限并确认进程无残留。独立 CTest 2/2（0.08 秒）、QML lint、格式检查通过；任务 1.3 完成，完整 macOS 平台矩阵仍 Pending。失败尝试、复验、命令及证据范围见 [变更证据](../../../openspec/changes/add-mascot-pet/validation.md)。

## 2026-10-29 公共气泡焦点与 Gallery 可访问性

macOS / Qt 6.10.2 Release 完整构建、库／Gallery QML lint、最终 CTest 20/20（88.72 秒）通过。Cocoa 原生窗口 QtTest 键盘生命周期 3 passed，Gallery 六组主题／密度及语言回归通过；已查看桌面亮色与窄屏深色截图。完成 6.2，进度 28/41；4.5 跨应用真实焦点、4.2 原生区域输入及完整平台矩阵仍待验收。具体失败、修复、测试边界及证据见 [变更记录](../../../openspec/changes/add-mascot-pet/validation.md)。


## 2026-10-30 macOS 公共区域输入

已实现私有 AppKit 区域输入面板、同实例事件转换与独立进程真实输入测试。Cocoa / Metal 原生结构、事件与椭圆角点检查 5 passed；真实输入因系统权限检查不可用及 LaunchServices 启动停顿仍 Pending，未将跳过记为通过。临时辅助功能权限已关闭。4.2 保留未完成，进度 29/41；详细失败、修复、环境和回归见 [变更证据](../../../openspec/changes/add-mascot-pet/validation.md)。

本轮最终完整构建、库／Gallery QML lint、CTest 22/22（157.05 秒）、安装消费者离屏运行、文档检查及 OpenSpec strict 均通过。


## 2026-10-30 macOS 公共区域真实输入续验

修复 Qt Cocoa 穿透标志同时拒绝键盘焦点的问题，视觉穿透改由 AppKit 承担，气泡显式申请本应用焦点。Metal 下 DPR=1／1.25 两轮真实输入均 6 passed，无失败／跳过；独立背后进程验证 10 组检查及隐藏／销毁清理。完成 4.2，进度 30/41；临时辅助功能权限已关闭，完整平台矩阵仍 Pending。失败尝试与最终证据见 [变更记录](../../../openspec/changes/add-mascot-pet/validation.md)。

最终完整 CTest 22/22（160.29 秒）、完整构建、库／Gallery QML lint、安装消费者、文档检查与 OpenSpec strict 通过；最终原生结构检查 5 passed。


## 2026-10-30 macOS 置顶与按需焦点

修复 Qt Tool 窗口关闭置顶后仍处浮动层的问题，明确 AppKit 普通／置顶层级。Metal 下 DPR=1／1.25 两轮真实输入均 6 passed，无失败／跳过，16 组记录包含跨应用被动焦点、置顶系统命中、Tab／Shift+Tab／空格关闭及隐藏／穿透／失活后的焦点策略。完成 4.5，进度 31/41；临时辅助功能权限已关闭，5.2 和完整平台矩阵继续待验收。失败与最终结果见 [变更证据](../../../openspec/changes/add-mascot-pet/validation.md)。


## 2026-10-30 Gallery 应用失活层级回归

修复角色视觉 NSPanel 仍使用 Tool 面板失活隐藏／浮动行为的缺口。新增“先激活普通宿主，再切换到独立 helper”原生检查，验证可见性、层级和系统命中，5 passed；无需辅助功能权限。具体环境与补充结果见 [变更证据](../../../openspec/changes/add-mascot-pet/validation.md)，完整平台矩阵仍待验收。
