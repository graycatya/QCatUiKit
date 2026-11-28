# Tasks

## 1. 修复

- [x] 1.1 RTL 下标签 `leftPadding` 改为关闭目标宽度加 4，文字不再被覆盖。
- [x] 1.2 RTL 下单元宽度不再重复计入关闭目标；LTR 保持原计算。

## 2. 测试

- [x] 2.1 `tst_dock.qml::test_rtlTabStrip` 断言 `leftPadding >= closeButton.width + 4`。

## 3. 验证

- [x] 3.1 构建并运行 `qml_contracts`、`gallery_contracts`。
- [x] 3.2 `qmllint`、`python tools/check_docs.py`、`git diff --check`、`openspec validate fix-dock-rtl-tab-overlap --strict --no-interactive` 通过。
- [x] 3.3 用 `--rtl` 截图确认关闭目标不再覆盖标签文字。
- [x] 3.4 在 `validation.md` 记录命令、结果与未覆盖项。
