# 桌宠平台原型与验收边界

[文档索引](README.md) · [角色包与播放](MASCOTPET.md) · [验证步骤](VALIDATION.md)

库尚未提供公共 CatMascotPet 桌面窗口。`tests/manual/mascot-probe` 用于验证平台窗口能力，不是应用接入 API，不依赖 Gallery、角色素材或私有库组件。

| 环境 | 当前原型 | 完整桌宠平台验收 |
| --- | --- | --- |
| Windows | 交互原型和跨进程输入检查；实测版本及结果保存在 OpenSpec change | Pending |
| macOS | 单屏矩形原型：真实输入检查通过，菜单与区域显示由实机人工确认 | Pending |
| X11 | 只有环境诊断；窗口原型待实现 | Pending |
| 原生 Wayland KWin / Mutter | 只有环境诊断；合成器配套、协议及窗口原型待实现 | Pending |

## Windows 原型

Qt Quick 透明无边框窗口显示测试角色区域和两个气泡按钮。独立 Win32 进程在后方绘制背景，按原生鼠标、键盘及焦点消息计数。交互模式提供显示／隐藏、定位复位、置顶、整窗穿透和气泡区域切换；托盘提供显示／隐藏、穿透切换、退出请求。退出菜单只记录宿主请求，关闭诊断主窗口或按 Close probe 才结束原型。

原型以声明的矩形并集调用 [QWindow.setMask](https://doc.qt.io/qt-6/qwindow.html#setMask)，不做逐帧透明度扫描。掩码外的显示行为受窗口系统影响，视觉也应在区域外透明。整窗穿透使用 Qt.WindowTransparentForInput；Windows 分层窗口的 WS_EX_TRANSPARENT 规则见 [Windows 窗口说明](https://learn.microsoft.com/en-us/windows/win32/winmsg/window-features)。

被动显示和角色点击不申请焦点，用户点击气泡才允许激活。气泡支持 Tab 和空格；托盘菜单显式取得前台和键盘焦点。前台激活受 [SetForegroundWindow 条件](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setforegroundwindow) 约束，应记录实际结果，不能把请求发出当成已获焦点。

图标被收进隐藏区时，Qt 返回的位置可能是展开箭头。自动检查先点击该位置，再查询展开后的图标位置并右键打开本程序菜单；不修改系统托盘显示设置。证据记录初始和展开后区域、Context 通知及菜单动作结果。

## 证据与限制

`--json` 仍只采集环境并报告 Pending。`--windows-check <report.json>` 需要可交互的 Windows QPA 桌面，使用 SendInput 检查独立进程计数和前台 HWND。执行期间勿移动鼠标或切换窗口。普通输入和键盘目标受测试窗口归属检查，托盘操作限于 Qt 返回的本程序图标区域；结束时清理目标进程、托盘和测试窗口，恢复光标及先前前台窗口。若环境拒绝系统输入或没有交互桌面，保留待验收并返回非零。

原型检查全部通过时 prototypeAcceptance 为 Passed，顶层 acceptance 仍为 Pending；按项 Supported 附带单主屏原型范围。常规 offscreen 分类测试和拒绝 Windows 实机检查测试不发送系统输入，不构成桌面验收。

这不等于生产适配：公共桌宠窗口、角色动作区域／镜像／缩放联动、完整气泡、平滑移动、多实例及平台托盘生命周期仍待实现。跨屏、混合 DPI、屏幕移除、独占全屏和系统安全桌面不在原型通过范围内。完整验收仍须执行 OpenSpec 平台矩阵，不外推到 Windows 所有版本或其他操作系统。

## macOS 原型

`--macos-prototype` 打开诊断窗口、独立背后测试进程、原生角色面板、气泡面板和 QCat 菜单栏项目。诊断窗口提供显示、隐藏、复位、整窗穿透、置顶和气泡开关；save 保存当前观察记录。菜单提供 show、hide、passthrough 和 exit，exit 只增加宿主退出请求计数。关闭诊断窗口才结束探针并回收背后进程、面板和菜单栏项目。

角色与气泡采用两个独立矩形 NSPanel，对应两个声明输入区域，中间空白没有原生窗口。这是验证矩形区域并集的原型，不代表任意椭圆／多边形区域已经实现。角色面板使用 nonactivatingPanel 并拒绝成为 key window；气泡允许主动按钮和键盘操作，用户点击气泡时显式激活应用并设置按钮为原生响应者；两按钮提供 Tab／Shift+Tab 切换及空格触发，不修改系统键盘导航设置。整窗穿透同时设置两面板的 ignoresMouseEvents，拖动同步移动气泡，置顶使用 NSFloatingWindowLevel，关闭后使用 NSNormalWindowLevel。角色以 AppKit 绘制，诊断界面使用 Qt Quick；本探针不验证角色 Lottie、QRhi 或 Metal 渲染。窗口属性改变不能代替实际点击去向或合成像素证据。

Qt 6.10.2 Cocoa 的 setMask 实现裁剪视图图层，不能仅凭该调用认定区域输入已验证。因此本原型不依赖透明像素命中、光标轮询、事件转发或 AppKit 全局方法替换。后续完整适配仍需验证非矩形区域和真实角色视觉。

`--macos-check <report.json>` 仅在 Cocoa 图形会话运行。它先记录窗口结构检查，再通过 CGPreflightPostEventAccess 检查系统事件发送权限；权限不可用时保存 Pending 并返回 2，不弹权限申请、不修改系统设置、不把检查跳过记为通过。权限可用时会移动鼠标及发送测试点击／按键，执行期间勿切换窗口；运行后检查 JSON 中逐项结果。菜单栏真实操作和透明合成仍需独立交互证据，因此该入口目前不会自行给出完整原型 Passed。失败返回 1，写入失败须查看 writeError。离屏运行覆盖旧证据并返回 2。

本轮 macOS 实测版本和结果见 OpenSpec change 的 validation.md。跨屏、混合 DPI、任意命名区域、平滑移动、屏幕移除、气泡避让、Spaces／全屏及完整平台矩阵继续 Pending；不依赖合成器配套扩展，也不要求为正常窗口展示授予辅助功能或录屏权限。

诊断窗口的 menu 按钮可展开同一份原生 NSMenu，用于局部调试，不等于菜单栏图标被真实点击。菜单回调记录 menuActions、lastMenuAction 和 exitRequests；save 写入 outputPath 所示的系统临时目录。2026-10-29 在 macOS 26.6.2 / Qt 6.10.2 的单屏矩形原型完成自动输入及人工菜单验收；自动检查和人工证据分列记录。自动命令本身仍保留 Pending，最终原型结论需合并人工证据；完整平台矩阵继续 Pending。
