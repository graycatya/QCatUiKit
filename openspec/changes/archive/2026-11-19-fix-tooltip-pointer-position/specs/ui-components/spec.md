# Spec Delta

## ADDED Requirements

### Requirement: R10 ToolTipPointerPlacement

`CatToolTip` 在悬停交互启用时 SHALL 在指针位置附近显示，并在显示后保持位置；提示 SHALL 按指针、输入和窗口状态关闭，且关闭后同一次悬停 SHALL NOT 自动重新显示。

#### Scenario: 指针定位

- WHEN 指针在父项内停稳并达到 delay
- THEN 提示 SHALL 在指针右下方显示，水平偏移 12、垂直偏移 16 逻辑像素。

#### Scenario: 显示后固定位置

- WHEN 提示已显示且指针在父项内继续移动
- THEN 提示位置 SHALL 保持不变。

#### Scenario: 移动重置延迟

- WHEN 提示尚未显示且指针移动
- THEN delay SHALL 重新计时，提示 SHALL NOT 在重新计时完成前显示。

#### Scenario: 点击关闭与取消待显示

- WHEN 用户在 delay 等待期间点击父项
- THEN 提示 SHALL NOT 显示。
- WHEN 提示已显示且用户点击
- THEN 提示 SHALL 关闭。

#### Scenario: Esc 与窗口失活

- WHEN 提示已显示且用户按 Esc
- THEN 提示 SHALL 关闭。
- WHEN 提示显示期间窗口失去激活状态
- THEN 提示 SHALL 关闭。

#### Scenario: 移出与重新进入

- WHEN 指针移出父项
- THEN 提示 SHALL 关闭。
- WHEN 指针重新进入父项并达到 delay
- THEN 提示 SHALL 再次显示。

#### Scenario: 关闭后同次悬停不重开

- WHEN 提示因点击、Esc 或 timeout 关闭且指针仍停留在父项内
- THEN 指针移动 SHALL NOT 使提示重新显示；指针移出后重新进入并达到 delay 后 SHALL 恢复显示。

#### Scenario: 窗口边缘避让

- WHEN 指针下方空间不足以容纳提示及 8 逻辑像素窗口边距
- THEN 提示 SHALL 显示在指针上方，垂直偏移 16 逻辑像素。
- WHEN 指针靠近窗口右侧
- THEN 提示 SHALL 保持在窗口内容区域内。

#### Scenario: 无悬停时回退定位

- WHEN 指针不在父项内而提示被打开（例如键盘焦点触发）
- THEN 提示 SHALL 显示在父项下方并水平居中。

#### Scenario: 应用控制模式

- WHEN `hoverInteractionEnabled=false`
- THEN 提示 SHALL 保留应用设置的可见性和 x/y，SHALL NOT 因指针移动重置延迟，也 SHALL NOT 因点击、移出或窗口失活被自动关闭；Esc 快捷关闭 SHALL 不生效。
