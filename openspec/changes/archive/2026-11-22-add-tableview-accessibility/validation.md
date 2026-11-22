# Table accessibility validation

日期：2026-11-22。环境：Windows 11、Qt 6.12.0 MSVC Debug/Release、offscreen/software。

## 语义

- `CatTableView`：`Accessible.Table`，默认名称 `Table`，可配置 `accessibleName`，描述包含当前行数和列数。
- `CatTableCell`：`Accessible.Cell`，名称默认取显示文本，`Accessible.selected` 跟随选中状态。
- `CatTableHeaderCell`：`Accessible.ColumnHeader`，名称默认取显示文本，`accessibleDescription` 暴露排序方向。
- Gallery 排序表头通过 `sortDescription()` 输出升序/降序描述，并提供 `Accessible.onPressAction`。
- 表格视图获得焦点时显示主题色 2px 焦点边框。

## 键盘

- 普通、冻结和 RTL 视图支持 `PageUp` / `PageDown`。
- `Ctrl+Home` / `Ctrl+End` 跳到首/末行和视觉首/末列；RTL 列方向正确。
- 目标单元格更新共享选择并滚动到可见区域。
- 编辑状态下跨页键不改变选择。

## 测试

- `tst_tableaccessibility.qml`：Table 角色、名称、描述、焦点边框、PageUp/PageDown、Ctrl+Home/End、RTL、冻结和编辑屏蔽。
- `tst_tablecells.qml`：Cell/ColumnHeader 角色、名称、选中状态和排序描述。
- Gallery `test_tableSorting`：自定义表头 Accessible 角色、名称和排序描述变化。
- Debug / Release `qml_contracts`：484 passed、0 failed、4 skipped。
- Debug / Release 完整 Gallery：865 passed、0 failed、3 skipped。
- `qmllint`、`check_docs.py`、`git diff --check`、OpenSpec 严格校验：通过。

## 人工验收

- 2026-11-22 Windows NVDA 人工检查通过：Table/ColumnHeader/Cell 角色、表头名称、单元格选中状态、排序方向描述、焦点边框、PageUp/PageDown、Ctrl+Home/Ctrl+End，以及 RTL 和冻结布局均正常朗读和导航。
- 未覆盖 Orca、VoiceOver 和真实桌面多屏环境。

## 已知边界

- 焦点边框为视觉提示，不改变 Qt 的虚拟焦点树。
- `Accessible.description` 的排序文案由应用提供；库不内置排序状态。
