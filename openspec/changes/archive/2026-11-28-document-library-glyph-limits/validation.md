# Validation

本变更只修改文档与规格技术债记录，没有代码、构建或运行时改动。

## 源码核对

逐项确认字符来源：

- `CatMenuItem.qml`：`CatFontIcon` 使用 `✓`、`›`、`‹`。
- `CatStepper.qml`：完成步骤显示 `✓`。
- `CatBreadcrumb.qml`：`›` / `‹`。
- `CatCalendar.qml`：`‹` / `›`。
- `CatPipsPager.qml`：`‹` / `›`。
- `CatRatingControl.qml`：`★`。
- `CatExpander.qml`：`▾` / `▸`。
- `CatMultiSelectComboBox.qml`、`CatDropDownButton.qml`：`▾`。
- `CatAlert.qml`、`TabCellImpl.qml`：`×`。
- `CatSpinBox.qml`：`+` / `−`。
- 已绘制的对照：`CatCheckBox.qml` 与 `CatComboBox.qml` 使用 `Shape`，`CatTitleBar.qml` 使用自绘按钮。

## 检查

```sh
openspec validate document-library-glyph-limits --strict --no-interactive
openspec validate --specs --strict --no-interactive
python tools/check_docs.py
git diff --check
```

均通过。`check_docs.py` 检查 38 个 Markdown、531 个本地链接、代码围栏、重复正文和 126 个公开 Cat 类型。

## 未覆盖

- 未在 Windows、macOS、Linux 之间做字体差异截图对比。
- 未构造缺字字体环境验证方框回退。
- 未验证高 DPI、RTL 与触摸密度下的字符宽度变化。
- 本轮没有改为绘制或引入库字体；后续实施需单独立项并补充像素级验收。
