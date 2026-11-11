# Proposal

## Why

`CatTabView` 把每个页签拼成 `Row { CatToggleButton; CatButton }` 两个独立控件：边框和背景只画在标题按钮上，关闭按钮是旁边另一个 `flat` 按钮。结果关闭符号落在页签边框之外，看起来不属于该页签；页签关闭目标与切换目标之间也没有共享的视觉单元。

同类问题已在 2026-11-09 的停靠标签变更中解决（标题与关闭按钮共享单元格背景），`CatTabView` 没有同步，导致同一产品内两种页签外观。

## What Changes

- 新增内部类型 `TabCellImpl`：用一个背景矩形承载标题按钮与关闭目标，二者共享圆角、边框、悬停与选中外观。
- `CatTabView` 的页签改用该单元格；关闭符号改为绘制图形，不再依赖 `"×"` 字符。
- 关闭目标的命中区域独立于标题，点击关闭不切换当前页签。
- `tabsClosable` 为 false 时单元格不预留关闭空间。
- 更新 `src/uikit/doc/COMPONENTS.md` 的 `CatTabView` 说明。
- 重写 `tests/auto/qml/data/tst_extendedcontrols.qml` 中的页签用例：改用稳定 `objectName` 定位，替代硬编码坐标。

## Capabilities

### New Capabilities

无。

### Modified Capabilities

- `ui-components`: 增加页签单元格的外观、命中与键盘焦点要求。

## Impact

影响 `src/uikit/qml/CatTabView.qml`、新增 `src/uikit/qml/TabCellImpl.qml`、`src/uikit/CMakeLists.txt`、`src/uikit/doc/COMPONENTS.md`、`tests/auto/qml/data/tst_extendedcontrols.qml`。

`CatTabView` 的公开属性、信号与 `pages` 别名不变，不涉及 C++ 或 ABI。页签宽度随标题与关闭目标的内容计算，不再固定最小 80 逻辑像素；使用绝对坐标点击页签的外部测试需要改为按内容定位。

不涉及：自定义页签 delegate。该扩展点需要稳定的模型与控件回引约定，避免选中变化时重建委托；本次只修正内置实现，扩展点另行立项。

## 待确认

无。行为变更限于页签外观与宽度计算，已由组件目录说明。
