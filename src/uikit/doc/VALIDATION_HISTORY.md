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


## 2026-10-30 气泡边缘与角色锚点

限制长文本首选视口高度，修复气泡接近整屏宽时二选一对齐导致的屏幕越界。macOS Metal 下正常及 320 逻辑像素视口，明暗主题四角长文本、增量更新与关闭的锚点／边界检查均通过；原生按钮命中、间隙穿透、完整控件范围及进程内滚动检查通过。完成 5.2，进度 32/41；无辅助功能权限变更，不把 Qt 缩放模拟当作硬件多屏验收。失败与最终结果见 [变更证据](../../../openspec/changes/add-mascot-pet/validation.md)。

本轮最终完整构建、库／Gallery QML lint、CTest 22/22（153.45 秒）、安装消费者、文档检查和 OpenSpec strict 通过。


## 2026-10-30 带气泡拖动位置回归

修复 Cocoa 整窗边界约束与气泡翻转形成的角色位置卡住，以及旧拖动起点／延迟布局锚点覆盖新位置的问题。新增 Gallery 同源包的首次消息与40步往返拖动检查，每步、松开和关闭后精确核对角色原点；正常与1.25倍缩放通过。事件为进程内 Qt 事件，无系统输入授权变更。具体失败轨迹与证据见 [变更记录](../../../openspec/changes/add-mascot-pet/validation.md)。


## 2026-10-30 Gallery 重复 show 导致显示消息上移

用户复测仍偏移后重新打开5.2。以实际 Gallery 页面按钮和 AppKit 原生画面坐标复现20像素上移，拆分操作确认是对已显示桌宠重复调用 show()；之前直接调用 showBubble 的组件测试漏了这一步。两个显示入口改为 visible=true。PNG/Lottie 两行默认与1.25倍缩放均通过，正常缩放精确稳定，分数缩放不超过单个原生像素且不累积；5.2 恢复完成，32/41。失败轨迹、精度与输入范围见 [变更记录](../../../openspec/changes/add-mascot-pet/validation.md)。

最终完整构建、库／Gallery QML lint、CTest 22/22（156.60 秒）、安装消费者、文档与 OpenSpec strict 检查均通过。


## 2026-10-31 macOS 约定范围矩阵

按用户要求排除多屏、硬件混合DPI及拔屏（Excluded，不记通过），其余单屏约定范围通过，完成7.3，进度33/41。实际Gallery生命周期/位置5 passed、独立全屏恢复3 passed、真实输入7 passed及16组检查通过；Metal渲染与生命周期的首次错误期望和缺少包路径已修正并补跑。普通Spaces与透明由用户确认，受控全屏Space另有两次通知往返记录，未覆盖首次Pending证据。完整构建、库/Gallery qmllint、CTest22/22（142.08秒）、最终无桌面拒绝检查、重新安装及安装消费者通过。

用户确认移除临时辅助功能条目，但新进程最后预检仍为true，API撤销核对未记通过；本轮验收app已删除、测试及helper进程均退出。正常组件不依赖该权限。范围、失败尝试、修正和清理差异见[变更证据](../../../openspec/changes/add-mascot-pet/validation.md)，重复步骤见[VALIDATION](VALIDATION.md)。
