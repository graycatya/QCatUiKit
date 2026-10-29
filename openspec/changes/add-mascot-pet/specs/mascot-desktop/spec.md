# Spec Delta

## Purpose

定义桌宠在桌面系统中的窗口、鼠标输入、移动、焦点、宿主生命周期与能力诊断行为，使宿主能跨平台控制悬浮角色。规定 Windows、macOS、X11 与指定 Wayland 桌面环境的实机验收边界及平台限制。

## ADDED Requirements

### Requirement: R01 FloatingWindow

系统 SHALL 提供透明无边框桌面悬浮窗口，默认请求置顶且不主动抢焦点；平台无法提供的窗口策略 SHALL 明确报告不可用；宿主 SHALL 能关闭置顶。窗口 SHALL 在支持的平台跳过系统任务栏，SHALL NOT 以原生 transient parent 绑定宿主窗口；宿主最小化 SHALL NOT 最小化桌宠。菜单与气泡操作 SHALL 按需支持焦点与键盘操作，不承诺覆盖系统安全界面或独占全屏。

#### Scenario: 显示与交互
- WHEN 正在其他应用输入时显示桌宠或更新动作
- THEN 原应用 SHALL 保持输入焦点；用户主动操作菜单或气泡时 SHALL 能完成按钮键盘操作。

### Requirement: R02 InputRegions

系统 SHALL 默认使整个桌宠窗口接收鼠标，默认 inputTransparent=false、inputRegionsEnabled=false；SHALL 提供显式整窗穿透开关和可选声明区域外穿透。宿主启用 inputRegionsEnabled 时，系统 SHALL 使角色声明区域与可见气泡控件接收输入，其他窗口空白区域穿透。输入范围 SHALL 与动作、朝向、缩放和气泡布局同步，SHALL NOT 使用逐帧透明度判定。

#### Scenario: 点击背后应用
- WHEN 宿主已启用 inputRegionsEnabled 并点击角色声明区域与气泡之外的透明空白
- THEN 背后应用 SHALL 收到该点击，桌宠 SHALL NOT 发出区域点击事件。

#### Scenario: 整窗穿透
- WHEN 整窗穿透已启用
- THEN 角色与气泡 SHALL 均不拦截输入；宿主接口 SHALL 仍能关闭穿透。

### Requirement: R03 Movement

系统 SHALL 支持用户拖拽、外部位置设置及可取消的平滑移动，并通知完成、取消或失败。默认 SHALL 将角色边界限制在屏幕可用区域，SHALL 支持宿主选择目标屏幕并跨屏移动。

#### Scenario: 拖拽打断移动
- WHEN 平滑移动中用户开始拖拽
- THEN 移动请求 SHALL 报告取消，窗口 SHALL 跟随拖拽，SHALL NOT 继续执行旧轨迹。

#### Scenario: 无法定位
- WHEN 当前会话无法执行定位请求
- THEN 系统 SHALL 返回明确失败原因，SHALL NOT 将仅改变内部坐标视为窗口已移动。

### Requirement: R04 ScreensAndBubblePlacement

系统 SHALL 使用逻辑坐标处理多屏和 DPI 变化；屏幕断开时 SHALL 将角色校正至可用屏幕。气泡 SHALL 自动调整方位并限制尺寸，SHALL NOT 通过调整布局改变角色锚点。

#### Scenario: 混合 DPI 与屏幕移除
- WHEN 跨不同 DPI 屏幕移动或移除当前屏幕
- THEN 角色及输入区域 SHALL 保持坐标一致，窗口 SHALL 处于剩余可用区域。

#### Scenario: 边缘长文本
- WHEN 屏幕边缘显示超出可用高度的文本
- THEN 气泡 SHALL 避让边缘并提供受限区域内的内容滚动，角色锚点 SHALL 保持原位。

### Requirement: R05 HostOwnedLifecycle

系统 SHALL 提供显示／隐藏和关闭默认隐藏策略，SHALL NOT 实现托盘图标、托盘菜单或进程退出。宿主 SHALL 实现自己的托盘、开机自启及应用启停策略；组件隐藏后 SHALL 保留宿主恢复显示入口。

#### Scenario: 宿主恢复桌宠
- WHEN 宿主最小化或通过自己实现的托盘恢复桌宠
- THEN 桌宠 SHALL 保持独立窗口状态并通过 show/hide 接口响应，SHALL NOT 管理宿主托盘或退出进程。

### Requirement: R06 PlatformIntegration

系统 SHALL 面向 Windows、macOS、Linux X11 与原生 Wayland 暴露实际能力及原因，SHALL NOT 依赖、安装或管理桌面扩展和配套服务；SHALL NOT 将 XWayland 验证代替原生 Wayland 验证。宿主 SHALL 管理实例生命周期、开机自启、应用启停及系统集成。

#### Scenario: 平台能力受限
- WHEN 当前平台无法直接提供定位、置顶或其他窗口能力
- THEN 对应能力 SHALL 标为不可用并提供原因；相关请求 SHALL 明确失败，SHALL NOT 假报完整支持。

#### Scenario: 宿主管理生命周期
- WHEN 宿主创建、隐藏或销毁桌宠实例
- THEN 系统 SHALL 仅管理该实例资源，SHALL NOT 设置开机自启、控制应用启停或更改桌面扩展。

### Requirement: R07 AcceptanceEvidence

系统 SHALL 提供可重复平台验证工具与步骤，允许分阶段验收。平台支持记录 SHALL 区分待验证、通过与失败，并记录系统、Qt、桌面环境、图形后端版本；未实机通过完整矩阵的平台 SHALL NOT 宣告完整支持。

#### Scenario: 仅自动化检查通过
- WHEN 平台仅通过构建、模拟或离屏测试
- THEN 支持记录 SHALL 仍标记为待实机验收。

#### Scenario: 完整平台验收
- WHEN 指定平台环境完成窗口、穿透、焦点、移动、多屏、气泡、宿主生命周期及受限能力失败验证
- THEN 记录 SHALL 附带可追溯结果后才将该版本环境标为通过；宿主恢复显示路径 SHALL 经过验证。
