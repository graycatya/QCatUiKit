# Tasks

## 1. 实现

- [x] 1.1 新增内部类型 `src/uikit/qml/TabCellImpl.qml`：背景矩形 + 标题按钮 + 关闭目标，关闭符号改为两条旋转矩形。
- [x] 1.2 `CatTabView` 的 `Repeater` 委托改为 `TabCellImpl`，对外属性、信号与 `pages` 别名不变。
- [x] 1.3 `src/uikit/CMakeLists.txt` 注册内部类型并加入 `QML_FILES`。
- [x] 1.4 修正关闭符号垂直偏移：`Row` 按顶部对齐子项，20 逻辑像素的关闭目标在高 32 的单元格内偏上 6 像素；关闭目标改为撑满单元格高度，20 像素悬停面在背景中居中绘制。

## 2. 测试

- [x] 2.1 重写 `tst_extendedcontrols.qml::test_tabsPreserveStateAndRequestClose`：改为按 `tabTitle_<index>`／`tabClose_<index>` 定位，替代硬编码坐标。
- [x] 2.2 增加断言：点击非当前页签的关闭目标只发出 `closeRequested`，`selectedIndex` 保持不变。
- [x] 2.3 增加对齐与边界断言：关闭符号与标题的垂直中心一致，关闭目标右边界不超出单元格；已用旧实现复验该断言会失败。

## 3. 文档

- [x] 3.1 更新 `src/uikit/doc/COMPONENTS.md` 的 `CatTabView` 条目，说明单一单元格与关闭不切页。

## 4. 集成检查

- [x] 4.1 Release 全量构建，`ctest` 21/21 通过。
- [x] 4.2 `QCatUiKit_qmllint`、`qcat-gallery_qmllint`、`python tools/check_docs.py`、`git diff --check` 通过。
- [x] 4.3 离屏渲染亮色、暗色与关闭悬停三种状态，确认关闭符号落在页签边界内。
- [x] 4.4 `openspec validate polish-tabview-tab-cell --strict --no-interactive` 通过。

## Workflow follow-up

- 已创建 `validation.md` 记录本轮证据。
- 归档时把新增 Requirement R08 同步进 `ui-components` 主规格。
- 自定义页签 delegate 另行立项：需要稳定的委托模型与控件回引约定，见 design.md。
