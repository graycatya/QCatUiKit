# 桌宠平台原型与验收边界

[文档索引](README.md) · [角色包与播放](MASCOTPET.md) · [验证步骤](VALIDATION.md)

库尚未提供公共 CatMascotPet 桌面窗口。`tests/manual/mascot-probe` 用于验证平台窗口能力，不是应用接入 API，不依赖 Gallery、角色素材或私有库组件。

| 环境 | 当前原型 | 完整桌宠平台验收 |
| --- | --- | --- |
| Windows | 交互原型和跨进程输入检查；实测版本及结果保存在 OpenSpec change | Pending |
| macOS、X11 | 只有环境诊断；窗口原型待实现 | Pending |
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
