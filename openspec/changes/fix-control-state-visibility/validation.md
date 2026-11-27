# Validation

执行环境：Linux x86_64，Qt 6.12.0 gcc_64，Release 单配置，Ninja，`build-release`。

## 自动测试

```sh
cmake --build build-release --parallel
ctest --test-dir build-release -R qml_contracts --output-on-failure
```

结果：**通过**，37.74 s。`ExtendedControls` 新增用例：

- `test_spinStepButtonStateVisibility`：静止时指示器 alpha 为 0；悬停时颜色不同于控件背景且 alpha 大于 0；按下时 alpha 大于悬停；指针离开后恢复透明。
- `test_comboPopupStateVisibility`：打开前主背景为 `surface`；打开后主背景不同于 `surface`；当前项背景不同于弹出层表面色；非当前项为表面色；悬停项背景不同于弹出层表面色。

单独运行 `tst_extendedcontrols.qml`：31 passed / 0 failed。

## 静态检查

```sh
qmllint -I build-release/qml src/uikit/qml/CatSpinBox.qml src/uikit/qml/CatComboBox.qml
python3 tools/check_docs.py
git diff --check
openspec validate fix-control-state-visibility --strict --no-interactive
```

均通过。

## 视觉检查

离屏软件后端单独渲染确认：

- SpinBox 悬停时主背景为 `surfaceHover`，加号按钮为 `primary` 16% 背景，二者可区分；按下时按钮背景加深为 28%。
- ComboBox 打开时主背景为 `primary` 16% 背景；弹出列表当前项为 `primary` 16% 背景并保留左侧 `primary` 3 逻辑像素标识条，其余项为表面色。

采集限制：未在原生图形后端、macOS、Windows、深色主题、触摸密度和 RTL 下复核；未测量颜色变化对渲染耗时的影响。

## 未覆盖

- `CatSpinBox` 的 `+`／`−` 仍为文本字形，未纳入本次范围。
- 表格行悬停与选中背景（`CatTableCell`）未改动。

## 后续变更

`add-state-style-tokens` 已将 `CatSpinBox` 和 `CatComboBox` 的内部状态色迁移为主题 Token（`surfaceSelected` / `surfacePressed`），本变更的行为断言与可见性目标保持不变。
