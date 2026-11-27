# Tasks

## 1. 步进按钮反馈

- [x] 1.1 `CatSpinBox` 的 up/down 指示器悬停时显示 `primary` 低透明度背景，按下时加深，静止保持透明。
- [x] 1.2 禁用或 `hoverEnabled=false` 时指示器不显示悬停背景，按下状态仍按可用性着色。

## 2. 下拉选中与悬停反馈

- [x] 2.1 `CatComboBox` 弹出列表项的 `highlighted` 与 `hovered` 背景使用 `primary` 低透明度，与弹出层表面色可区分。
- [x] 2.2 当前项保留左侧主色条，叠加高亮背景后仍可辨认。

## 3. 测试

- [x] 3.1 断言 SpinBox 指示器静止为透明，悬停与按下时颜色不同于控件背景且按下更深。
- [x] 3.2 断言 ComboBox 弹出列表当前项与悬停项背景不同于弹出层表面色。

## 4. 验证

- [x] 4.1 `cmake --build build-release --parallel` 通过。
- [x] 4.2 受影响的 QML 测试通过。
- [x] 4.3 `qmllint` 修改的两个文件、`python tools/check_docs.py`、`git diff --check`、`openspec validate --strict --no-interactive` 通过。
- [x] 4.4 在 `validation.md` 记录命令、结果与未覆盖平台。

## Workflow follow-up

- 归档时把 R18 同步进 `ui-components` 主规格。
