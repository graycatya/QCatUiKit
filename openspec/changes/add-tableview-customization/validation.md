# CatTableView 定制验证

日期：2026-11-20。环境：Windows 11、Qt 6.12.0 MSVC Debug、offscreen/software。

本轮完成任务 2.1-2.3 的编辑闭环：`editDelegate`、默认编辑输入框、`edit(row, column)`、`commitEdit(value)`、`cancelEdit()`，以及冻结/RTL 下的编辑写回。

- `CatTableView` 新增 `editDelegate` Component，默认 `TextInput`；默认单元格通过 `TableView.editDelegate` 转发，五个 TableView 设置 `DoubleTapped | EditKeyPressed` 触发。
- 编辑项继承单元格位置和 `display`，提交仍走原生 `TableView.onCommit`；`commitEdit` 按 EditRole 经当前视图模型写回，RTL 下由代理映射到源列。
- `__closeEditors()` 在开始编辑、提交和取消时关闭四区编辑器，保证同一时间只有一个编辑项。
- 新增 `tests/auto/qml/data/tst_tableediting.qml`：编辑提交、取消、非法索引、冻结列/主体编辑切换。
- `tst_tablertl.qml` 新增 RTL 编辑提交，验证代理列写回源列。
- `ctest --test-dir <build> -R '^qml_contracts$' --output-on-failure`：455 passed、0 failed、4 skipped。
- `qmllint`、`check_docs.py`、`git diff --check`、OpenSpec 严格校验：通过。

## 已知问题

- Qt 6.12 的 `QQuickTableView::edit()` 缺少返回类型注解，QML 调用会输出 `QCRITICAL: ... should be coerced to void ...`。行为正确，测试通过，但日志有噪声；这是上游注解问题，不在本变更内修。
- `editDelegate` 目前只自动转发给默认 delegate；自定义 delegate 需要自己设置 `TableView.editDelegate`。基类 `CatTableCell` 和 `editDelegate` 的公开契约仍待任务 1.x。
- 空状态、加载状态、toolbar、headerDelegate 和 Gallery 编辑示例尚未实现（任务 3.x/4.x）。
