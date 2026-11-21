# CatTableView 定制验证

日期：2026-11-21。环境：Windows 11、Qt 6.12.0 MSVC Debug、offscreen/software。

本轮完成任务 1.1-1.3、2.1-2.3、3.1-3.3、4.1-4.3、5.1-5.2：公开单元格/表头基类、编辑委托、空态/加载态、toolbar、headerDelegate 和 Gallery 示例。

- `CatTableCell`、`CatTableHeaderCell` 作为公开 QML 类型：自带 row/column/display/selected/editing/theme 契约、主题背景和默认外观；`CatTableView` 默认 delegate 与表头改用基类，视觉保持不变。
- 默认单元格通过 `TableView.editDelegate` 转发 `editDelegate`；`edit`/`commitEdit`/`cancelEdit` 按 EditRole 写回，冻结与 RTL 下由共享模型/代理映射。
- `toolbar` 固定在表头之上、跨全宽，不随表格滚动；高度由工具栏内容计算，未提供时不占空间。
- `emptyState`/`loadingState` 覆盖内容区（表头和 toolbar 除外）；`loading` 控制加载态，空模型显示空态，未提供插槽时不显示、不阻断内容。
- `headerDelegate` 同时应用到主体段和冻结段表头；动态切换会重新加载表头委托。
- Gallery 数据页增加自定义表头、工具栏、加载态、空态和恢复行控件，补齐中英文 TS。

## 测试

- 新增 `tests/auto/qml/data/tst_tablecells.qml`：基类契约、派生单元格、表头基类。
- 新增 `tests/auto/qml/data/tst_tablecustomization.qml`：toolbar 位置/宽度/不随滚动、无 toolbar 不占空间、空态/加载态、表头委托与动态切换。
- `tst_controls.qml` 导入清单加入两个公开基类；基类的 required 属性在导入测试中显式赋值。
- `ctest --test-dir <build> -R '^qml_contracts$' --output-on-failure`：470 passed、0 failed、4 skipped。
- 完整 Gallery 契约：863 passed、0 failed、3 skipped；`GalleryDataExamples::test_tableCustomization` 通过。
- `qmllint`、`check_docs.py`、`git diff --check`、OpenSpec 严格校验：通过。

## 已知问题

- Qt 6.12 的 `QQuickTableView::edit()` 缺少返回类型注解，QML 调用会输出 `QCRITICAL: ... should be coerced to void ...`。行为正确，测试通过，但日志有噪声；这是上游注解问题，不在本变更内修。
- `editDelegate` 只自动转发给默认 delegate；自定义 delegate 需要自己设置 `TableView.editDelegate`，这是公开契约的一部分。
- `CatTableHeaderCell.column` 是 required 属性；独立创建时需显式赋值，TableView/HeaderView 会设置。
- toolbar 高度取 `childrenRect.height`，自定义 toolbar 应提供明确高度或 implicitHeight。
- 真实桌面、多屏和读屏的人工检查仍需在 Gallery 中补做。
