# Proposal

## Why

`CatCommandBar` 的溢出按钮使用 `CatDropDownButton`，其右侧为下拉箭头预留 `padding + 18`，而按钮宽度固定为 `controlHeight`（桌面密度 32）。桌面密度下可用宽度为 `32 - 12 - 30 = -10`，文本标签被夹成 0 宽，界面只剩一个孤立的小三角，看起来像渲染错位；按钮也没有设置 `flat`，与左侧扁平主操作样式不一致。

## What Changes

- 溢出按钮改为扁平 `CatButton`，自行打开并定位 `CatMenu`，不再显示下拉箭头。
- 宽度改为 `max(controlHeight, implicitContentWidth)`，左右内边距归零，字号或密度变化时内容仍放得下。
- 为溢出按钮与菜单增加稳定 `objectName`，供回归测试定位。
- 新增 `tests/auto/qml/data/tst_commandbar.qml`：字形可用宽度为正、无次要操作时隐藏、点击后菜单打开。

## Capabilities

### New Capabilities

无。

### Modified Capabilities

- `ui-components`: 增加命令栏溢出按钮的可见字形、样式与辅助名称要求。

## Impact

影响 `src/uikit/qml/CatCommandBar.qml`，新增 `tests/auto/qml/data/tst_commandbar.qml`。

`CatCommandBar` 的公开属性、信号与隐含尺寸计算不变，不涉及 C++ 或 ABI。可见变化是溢出按钮由带边框的下拉按钮变为扁平省略号按钮；沿用旧外观的截图需要更新。

不涉及：菜单项排序、溢出折叠成多级菜单、键盘导航扩展。
