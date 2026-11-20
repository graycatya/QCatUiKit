# Tasks

## 1. 基类与 delegate 契约

- [ ] 1.1 新增 `CatTableCell` 与 `CatTableHeaderCell`，声明 row/column/display/selected/editing/theme 契约和主题背景；在 `src/uikit/CMakeLists.txt` 登记，并用 QML 测试验证继承后默认外观与选中/编辑态。
- [ ] 1.2 用 `CatTableCell` 重写 `CatTableView` 默认 delegate，保持现有视觉；用现有 QML 控件测试验证默认渲染和滚动不变。
- [ ] 1.3 在 `tst_controls.qml` 的组件导入清单加入两个新类型，用 QML 测试验证可导入。

## 2. 编辑委托

- [x] 2.1 新增 `editDelegate` 并转发到默认 delegate 的 `TableView.editDelegate`，设置双击和编辑键触发；用 QML 测试验证双击、F2、Enter 进入编辑。
- [x] 2.2 实现 `edit(row, column)`、`commitEdit(value)`、`cancelEdit()`，按 EditRole 写回并处理模型拒绝；用 QML 测试验证提交、取消、Esc 和只读 flags。
- [x] 2.3 验证冻结四区共享编辑态，提交/取消时关闭其他区编辑器；在冻结合并后补 QML 回归。

## 3. 状态插槽与工具条

- [ ] 3.1 新增 `loading`、`emptyState`、`loadingState` 和内容区覆盖层，未提供插槽时不显示；用 QML 测试验证空数据、加载、未提供插槽三种状态。
- [ ] 3.2 新增 `toolbar` 插槽，固定于表头之上且跨全宽；用 QML 测试验证位置、宽度、不随滚动和未提供时不占空间。
- [ ] 3.3 验证状态覆盖层不遮挡表头和 toolbar，且不阻断表格滚动；用 QML 测试验证点击/滚动穿透。

## 4. 表头委托与演示

- [ ] 4.1 新增 `headerDelegate` 并同时应用到冻结段和主体段表头；用 QML 测试验证两段都使用该 delegate 且列宽/文本同步。
- [ ] 4.2 在 Gallery 数据页增加编辑、空态、加载、toolbar 和表头定制示例，补齐中英文 TS；用 Gallery 测试验证页面装配和语言切换。
- [ ] 4.3 更新 `COMPONENTS.md`、`VALIDATION.md` 和组件目录条目。

## 5. 回归与验收

- [ ] 5.1 运行 QML 行为测试、库与 Gallery qmllint、`python tools/check_docs.py` 和 `git diff --check`，记录结果。
- [ ] 5.2 在无冻结和冻结两种布局下做默认行为回归，确认未设置新属性时布局、滚动和现有 delegate 不变。
