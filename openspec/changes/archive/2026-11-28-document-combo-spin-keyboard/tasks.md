# Tasks

## 1. 测试

- [x] 1.1 `test_comboKeyboardNavigation`：Tab 焦点描边、Space 打开、Down/Up、Home/End、Esc 关闭且 `currentIndex` 不变。
- [x] 1.2 `test_spinKeyboardSteps`：Up/Down 按 `stepSize` 步进，到达 `to`/`from` 后保持边界值。

## 2. 文档

- [x] 2.1 `COMPONENTS.md` 的 `CatComboBox` 与 `CatSpinBox` 补充具体键位。

## 3. 验证

- [x] 3.1 运行 `qml_contracts`。
- [x] 3.2 `python tools/check_docs.py`、`git diff --check`、`openspec validate document-combo-spin-keyboard --strict --no-interactive` 通过。
- [x] 3.3 在 `validation.md` 记录命令、结果与未覆盖平台。
