# 桌宠平台原型与验收边界

公共 `CatMascotPet` 不依赖或管理系统扩展，也不提供托盘；本文托盘步骤仅用于独立研究探针。以下 GNOME 配套安装、启停和协议均为独立研究原型，不是应用集成步骤或公共组件验收前置。开机自启、应用启停、系统集成由宿主决定；原生 Wayland 的受限能力明确报告不可用。

[文档索引](README.md) · [角色包与播放](MASCOTPET.md) · [验证步骤](VALIDATION.md)

库已提供公共 CatMascotPet 桌面窗口，接入方式见角色包与播放专题。`tests/manual/mascot-probe` 用于独立验证平台窗口能力，不是应用接入 API，不依赖 Gallery、角色素材或私有库组件。

| 环境 | 当前原型 | 完整桌宠平台验收 |
| --- | --- | --- |
| Windows | 交互原型和跨进程输入检查；实测版本及结果保存在 OpenSpec change | Pending |
| macOS | 单屏矩形原型：真实输入检查通过，菜单与区域显示由实机人工确认 | Pending |
| X11 / XWayland | Linux 椭圆角色／矩形气泡交互原型；当前实机输入证据仅来自 GNOME Wayland 会话中的 xcb | Pending |
| 原生 Wayland KWin / Mutter | Linux Qt 窗口原型；GNOME Shell 50 研究配套已有握手及原生操作读回证据，不作为公共组件依赖；KWin 配套研究暂缓 | Pending |

## Linux 原型

`--linux-prototype` 打开控制窗口、独立背后输入进程、透明角色／气泡窗口和可选托盘服务探测。角色区域为椭圆，气泡区域为矩形，二者并集通过 QWindow.setMask 声明；空白不根据逐帧像素透明度命中。控制窗口和托盘菜单提供显示、隐藏及穿透切换；退出菜单只记录请求，Close probe 才结束探针。原生 Wayland 拖动尝试 startSystemMove，结果保存在报告中，未取得实际拖动证据前不标为支持。

`--linux-check <report.json>` 自动运行并保存逐项结果，附独立进程 PID、随机实例 token、QPA、输入计数和窗口状态。xcb 下可选 Xlib 读取窗口位置、焦点及置顶状态，运行时加载 XTest 发送输入；检查只覆盖单屏 DPR 1，输入点限制在独立测试窗口内，并检查指针命中属于本次目标或角色窗口。执行期间勿操作鼠标／键盘或启动其他窗口测试。结束时回收子进程及托盘，恢复此前光标和 X11 焦点。

Xlib 是手动探针的可选构建依赖，XTest 是可选运行依赖；缺失时保留 Pending。Qt Widgets 仅用于探针应用、托盘和菜单，不新增库依赖。自动检查成功的项可记录 Supported，但不外推到完整桌宠、多屏或原生 X11。XWayland 的通过只适用于该会话，不替代原生 X11 或原生 Wayland。

原生 Wayland 分支不使用 XTest 结果作为输入证据。GNOME 窗口配套缺失、禁用或握手失败时，定位和置顶报告 Unavailable；Qt 内部位置或 flags 不构成原生执行成功。配套连接成功只表示握手通过，操作须取得原生读回后才记录该项 Supported。平滑移动仍未实现。区域输入、整窗穿透、焦点及系统拖动需独立真实输入证据。托盘服务检测与窗口能力分开记录；服务可用不等于托盘菜单已经实操通过。渲染目标 alpha 和裁剪截图仅证明窗口自身渲染，不证明桌面合成透明。桌面截图没有有效的独立背景参考时，该项保留 Pending。

自动入口当前始终保留完整平台 acceptance=Pending；若检查失败返回 1，否则在仍有待验收项时返回 2。JSON 中的 checks 是本次证据，交互界面的按钮操作不会自动勾选平台验收。离屏调用覆盖旧的成功报告并返回 2，所有桌面能力仍为 Unknown。可重复命令和人工验收步骤见 [验证说明](VALIDATION.md#linux-桌面窗口原型)。

### GNOME/Mutter 配套原型

源码位于 `tests/manual/mascot-probe/gnome`，仅面向 GNOME Shell 50 的原生 Wayland 会话，扩展版本 0.1.0、协议 1。这是待实机验收的候选范围，不能据此声明所有 GNOME 50 系统受支持；其他 Shell 主版本在 metadata 和运行时均拒绝。KWin 使用独立配套，不能安装本扩展替代。

配套只提供窗口快照、逻辑坐标定位和置顶开关，不提供输入注入、自动激活、屏幕捕获、网络、业务服务或托盘。Qt 客户端先确认服务与当前 `org.gnome.Shell` 属于同一 D-Bus 连接，再验证协议及 Shell 版本；失败区分 Missing、Disconnected、Untrusted、Incompatible。托盘继续由 QSystemTrayIcon 单独诊断，不依赖本扩展注册。

接口固定为会话总线 `org.qcat.MascotProbe1`、路径 `/org/qcat/MascotProbe1`、同名 interface。`Hello(u)` 返回协议、扩展版本和 Shell 版本；`Call(u,s,s,s)` 接收协议、UUID 实例标识、操作和 JSON 参数，返回 JSON 原生状态。操作为 `snapshot`（空对象）、`move`（整数 x/y）和 `above`（布尔 enabled）；坐标为 Mutter stage 全局逻辑坐标，不是未来公共桌宠接口的屏幕相对坐标。参数不得携带 PID，未知字段、超限值和旧主协议被拒绝。

服务端通过 D-Bus 查询调用连接的真实 PID，只查找同 PID、原生 Wayland 类型且标题精确等于 `QCat mascot probe:<UUID>` 的单个窗口；跨 PID 借用 token、XWayland 窗口、重名歧义窗口均不能操作。客户端同时检查返回的 PID、窗口类型和矩形字段。UUID 用于实例选择，权限边界是调用连接的进程身份，不将 token 当作替代系统权限的秘密凭证。此机制只覆盖同一用户会话中的普通本地进程；未验证容器 PID 命名空间或远程应用。

窗口操作使用 Mutter 的 move_frame、make_above／unmake_above 并读回 frame rect／is_above；请求已发送不等于 applied=true。配套失联时客户端取消待完成请求，移除旧能力和原生快照；重连后只重新握手／读取快照，旧移动及置顶请求不重放。扩展禁用或调用进程断开时恢复配套改动前的置顶状态，已完成的位置变更保留；关闭窗口会移除记录。请求有超时和并发上限，迟到回调按连接代次丢弃。当前原型没有持续移动任务，公共组件的移动取消仍待后续实现。

独立测试在临时私有 D-Bus 上运行同一 GJS 服务，只替换 Meta.Window 对象，覆盖真实调用 PID、拒绝错误协议、跨实例、断开、禁用恢复与重连；不算 Mutter 实机验收。安装包生成、安装启用、禁用卸载和现场诊断见 [验证说明](VALIDATION.md#gnomemutter-配套安装及验收)。不自动安装、启用扩展或注销当前会话。

实现参考 [GJS D-Bus 服务指南](https://gjs.guide/guides/gio/dbus.html)、[GNOME 扩展结构](https://gjs.guide/extensions/overview/anatomy.html) 和 [Mutter Window API](https://mutter.gnome.org/meta/class.Window.html)；这些在线文档可能展示更新版本，实际兼容性须按上述候选 Shell 版本验证。

## Windows 原型

Qt Quick 透明无边框窗口显示测试角色区域和两个气泡按钮。独立 Win32 进程在后方绘制背景，按原生鼠标、键盘及焦点消息计数。交互模式提供显示／隐藏、定位复位、置顶、整窗穿透和气泡区域切换；托盘提供显示／隐藏、穿透切换、退出请求。退出菜单只记录宿主请求，关闭诊断主窗口或按 Close probe 才结束原型。

原型以声明的矩形并集调用 [QWindow.setMask](https://doc.qt.io/qt-6/qwindow.html#setMask)，不做逐帧透明度扫描。掩码外的显示行为受窗口系统影响，视觉也应在区域外透明。整窗穿透使用 Qt.WindowTransparentForInput；Windows 分层窗口的 WS_EX_TRANSPARENT 规则见 [Windows 窗口说明](https://learn.microsoft.com/en-us/windows/win32/winmsg/window-features)。

被动显示和角色点击不申请焦点，用户点击气泡才允许激活。气泡支持 Tab 和空格；托盘菜单显式取得前台和键盘焦点。前台激活受 [SetForegroundWindow 条件](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setforegroundwindow) 约束，应记录实际结果，不能把请求发出当成已获焦点。

图标被收进隐藏区时，Qt 返回的位置可能是展开箭头。自动检查先点击该位置，再查询展开后的图标位置并右键打开本程序菜单；不修改系统托盘显示设置。证据记录初始和展开后区域、Context 通知及菜单动作结果。

## 证据与限制

`--json` 仍只采集环境并报告 Pending。`--windows-check <report.json>` 需要可交互的 Windows QPA 桌面，使用 SendInput 检查独立进程计数和前台 HWND。执行期间勿移动鼠标或切换窗口。普通输入和键盘目标受测试窗口归属检查，托盘操作限于 Qt 返回的本程序图标区域；结束时清理目标进程、托盘和测试窗口，恢复光标及先前前台窗口。若环境拒绝系统输入或没有交互桌面，保留待验收并返回非零。

原型检查全部通过时 prototypeAcceptance 为 Passed，顶层 acceptance 仍为 Pending；按项 Supported 附带单主屏原型范围。常规 offscreen 分类测试和拒绝 Windows 实机检查测试不发送系统输入，不构成桌面验收。

这不等于公共组件验收：公共窗口已有动作、气泡、移动与多实例接口，Windows 声明区域输入已有独立进程回归，但完整实机矩阵仍待完成；托盘由宿主实现。跨屏、混合 DPI、屏幕移除、独占全屏和系统安全桌面不在原型通过范围内。完整验收仍须执行 OpenSpec 平台矩阵，不外推到 Windows 所有版本或其他操作系统。

## macOS 原型

`--macos-prototype` 打开诊断窗口、独立背后测试进程、原生角色面板、气泡面板和 QCat 菜单栏项目。诊断窗口提供显示、隐藏、复位、整窗穿透、置顶和气泡开关；save 保存当前观察记录。菜单提供 show、hide、passthrough 和 exit，exit 只增加宿主退出请求计数。关闭诊断窗口才结束探针并回收背后进程、面板和菜单栏项目。

角色与气泡采用两个独立矩形 NSPanel，对应两个声明输入区域，中间空白没有原生窗口。这是验证矩形区域并集的原型，不代表任意椭圆／多边形区域已经实现。角色面板使用 nonactivatingPanel 并拒绝成为 key window；气泡允许主动按钮和键盘操作，用户点击气泡时显式激活应用并设置按钮为原生响应者；两按钮提供 Tab／Shift+Tab 切换及空格触发，不修改系统键盘导航设置。整窗穿透同时设置两面板的 ignoresMouseEvents，拖动同步移动气泡，置顶使用 NSFloatingWindowLevel，关闭后使用 NSNormalWindowLevel。角色以 AppKit 绘制，诊断界面使用 Qt Quick；本探针不验证角色 Lottie、QRhi 或 Metal 渲染。窗口属性改变不能代替实际点击去向或合成像素证据。

Qt 6.10.2 Cocoa 的 setMask 实现裁剪视图图层，不能仅凭该调用认定区域输入已验证。因此本原型不依赖透明像素命中、光标轮询、事件转发或 AppKit 全局方法替换。后续完整适配仍需验证非矩形区域和真实角色视觉。

`--macos-check <report.json>` 仅在 Cocoa 图形会话运行。它先记录窗口结构检查，再通过 CGPreflightPostEventAccess 检查系统事件发送权限；权限不可用时保存 Pending 并返回 2，不弹权限申请、不修改系统设置、不把检查跳过记为通过。权限可用时会移动鼠标及发送测试点击／按键，执行期间勿切换窗口；运行后检查 JSON 中逐项结果。菜单栏真实操作和透明合成仍需独立交互证据，因此该入口目前不会自行给出完整原型 Passed。失败返回 1，写入失败须查看 writeError。离屏运行覆盖旧证据并返回 2。

本轮 macOS 实测版本和结果见 OpenSpec change 的 validation.md。跨屏、混合 DPI、任意命名区域、平滑移动、屏幕移除、气泡避让、Spaces／全屏及完整平台矩阵继续 Pending；不依赖合成器配套扩展，也不要求为正常窗口展示授予辅助功能或录屏权限。

诊断窗口的 menu 按钮可展开同一份原生 NSMenu，用于局部调试，不等于菜单栏图标被真实点击。菜单回调记录 menuActions、lastMenuAction 和 exitRequests；save 写入 outputPath 所示的系统临时目录。2026-10-29 在 macOS 26.6.2 / Qt 6.10.2 的单屏矩形原型完成自动输入及人工菜单验收；自动检查和人工证据分列记录。自动命令本身仍保留 Pending，最终原型结论需合并人工证据；完整平台矩阵继续 Pending。


## 公共窗口直接集成的当前边界

| 会话 | 公共组件使用的机制与依赖 | 当前限制 |
| --- | --- | --- |
| Windows | Qt Quick 视觉窗口整窗穿透；私有 Win32 无绘制输入窗口使用声明区域并转换同实例鼠标消息；依赖系统 user32／gdi32 | 已有独立进程真实输入回归；完整平台矩阵仍 Pending |
| Cocoa | Qt Quick Window、Qt 窗口位置及标志；不引入探针的 Widgets／托盘依赖 | 声明区域外穿透尚未接入公共适配；完整焦点、多屏与桌面输入待验收 |
| xcb（原生 X11 或 XWayland） | Qt 定位及窗口标志；可选 X11 构建依赖、Xext 运行库及 ShapeInput 1.1；EWMH 任务栏提示 | 缺少 ShapeInput 时区域输入明确不可用；窗口管理器可限制层级；已有 XWayland 证据不能替代原生 X11 |
| 原生 Wayland | Qt surface 输入区域与系统拖动请求；不连接 D-Bus 配套 | 精确定位、平滑定位、通用置顶及跳过任务栏不可用；真实输入与合成器行为需逐会话验收 |

能力接口中的 Unknown 表示存在可请求路径但尚需桌面验收；Unavailable 附原因并使受限请求失败。它们不等同于平台验收 Passed。宿主须保留显示／隐藏及关闭穿透的入口，并按能力选择移动和窗口策略；重复验证步骤统一维护在 VALIDATION，具体系统、Qt、图形后端和逐项结果保存在对应 OpenSpec change。

CatMascotPet 不连接本文的研究配套。Linux xcb 可选链接 X11 并检测 Xext 运行库，使用 ShapeInput 单独设置输入区域；不调用 Qt xcb 的 bounding mask 裁切画面。原生 Wayland 通过 Qt 的 surface 输入区域请求，定位和置顶报告 Unavailable。Windows 公共输入区域采用独立的无绘制原生窗口，不对角色视觉窗口应用形状掩码。macOS 的公共输入区域私有适配仍待完成；既有手动原型的通过不代表公共组件已通过。

Windows 输入窗口使用 WS_EX_NOREDIRECTIONBITMAP，不创建重定向绘制表面；SetWindowRgn 仅作用于该输入窗口。鼠标捕获保持拖动期间的连续事件，失去捕获时释放 Qt 抓取；原生物理坐标按当前窗口 DPR 转为逻辑坐标。窗口由同实例视觉窗口拥有，不独立激活、不进入任务栏，按钮显式申请的键盘焦点仍属于公共窗口。不使用全局鼠标钩子、光标轮询或向背后应用转发消息；区域外输入由系统自然投递。机制依据 [Windows 扩展样式](https://learn.microsoft.com/en-us/windows/win32/winmsg/extended-window-styles)及 [WM_NCHITTEST 的线程边界](https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-nchittest)。

实现核对参考 Qt 6.12 官方源码：[xcb setMask](https://github.com/qt/qtbase/blob/6.12/src/plugins/platforms/xcb/qxcbwindow.cpp)、[Wayland setMask](https://github.com/qt/qtbase/blob/6.12/src/plugins/platforms/wayland/qwaylandwindow.cpp)。真实输入、焦点和混合 DPI 结果继续按具体版本记录，不外推支持。
