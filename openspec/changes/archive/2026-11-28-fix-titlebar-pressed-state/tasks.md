# Tasks

## 1. 实现

- [x] 1.1 最小化按钮按下背景改为 `surfacePressed`。
- [x] 1.2 最大化按钮按下背景改为 `surfacePressed`。

## 2. 测试

- [x] 2.1 新增 `tst_visualstates.qml` 用例：最小化/最大化按钮静止、悬停、按下背景分别等于 `surface`、`surfaceHover`、`surfacePressed`。
- [x] 2.2 同一用例断言关闭按钮悬停为危险色、按下为更深危险色，且最小化/最大化按下不等于 `disabled`。

## 3. 验证

- [x] 3.1 构建并运行 `qml_contracts` 与 `gallery_contracts`。
- [x] 3.2 `qmllint` 修改文件、`python tools/check_docs.py`、`git diff --check`、`openspec validate fix-titlebar-pressed-state --strict --no-interactive` 通过。
- [x] 3.3 在 `validation.md` 记录命令、结果与未覆盖平台。
