# Proposal

## Why

`CatToolTip` 之前直接使用 Qt ToolTip 的默认定位，提示出现在父项附近而与鼠标所指位置无关，用户难以判断提示对应哪个元素；点击、移出和窗口失活也没有统一的关闭规则，关闭后指针仍在控件内时还会再次弹出。

## What Changes

- 悬停交互启用时，提示 SHALL 在指针停稳达到 delay 后显示于指针右下方，水平偏移 12、垂直偏移 16 逻辑像素；显示后位置固定，指针在父项内继续移动不重新定位。
- 提示显示前移动指针 SHALL 重置 delay；点击（包含 delay 等待期间）、Esc、移出父项或窗口失活 SHALL 关闭提示。
- 点击、Esc 或 timeout 关闭后，同一次悬停 SHALL NOT 重新显示；指针移出后重新进入并达到 delay 才再次显示。
- 指针下方空间不足时提示翻转到指针上方；窗口右侧由 Popup 窗口约束回移，保留 8 逻辑像素边距。
- 指针不在父项内（例如键盘焦点触发）时回退到父项下方居中。
- 新增公开 QML 属性 `hoverInteractionEnabled`（默认 `true`）；设为 `false` 时保留应用控制的可见性和 x/y，不启用悬停重置、点击／移出／窗口失活关闭和 Esc 快捷关闭。库内 `CatSlider` 的数值提示采用该模式。
- 不改变 `delay=600ms`、`timeout=3000ms` 默认值和主题外观。

## Capabilities

### New Capabilities

### Modified Capabilities

- `ui-components`: 新增 `CatToolTip` 指针定位与悬停关闭规则（R10）。

## Impact

仅修改 `src/uikit/qml/CatToolTip.qml` 与 `src/uikit/qml/SliderHandleImpl.qml`，新增一个公开 QML 属性；不改变 C++ 接口、ABI、构建配置和主题 tokens。同步 `src/uikit/doc/COMPONENTS.md`、`src/uikit/doc/VALIDATION.md`；回归测试位于 `tests/auto/qml/data/tst_tooltip.qml`。
