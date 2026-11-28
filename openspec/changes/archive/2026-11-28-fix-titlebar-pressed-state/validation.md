# Validation

执行环境：Windows 11 x64，Qt 6.12.0 msvc2022_64，Release 单配置，Ninja，MSVC 19.39，`build-led-release`。

## 自动测试

```sh
cmake --build build-led-release --parallel
ctest --test-dir build-led-release -R "^(qml_contracts|gallery_contracts)$" --output-on-failure
```

结果：**2/2 通过**。`qml_contracts` 41.52 s，`gallery_contracts` 143.96 s。

新增用例 `tst_visualstates.qml::test_titleBarPressedState`：

- 最小化与最大化按钮静止为 `surface`、悬停为 `surfaceHover`、按下为 `surfacePressed`，且按下不等于 `disabled`。
- 关闭按钮悬停为 `danger`、按下为 `danger` 加深 1.1，保持原有危险色语义。
- 按下后把 `targetWindow` 置空再释放，避免测试触发真实窗口最小化／最大化／关闭。

## 静态检查

```sh
qmllint -I build-led-release/qml -I build-led-release/qml/QCatUiKit \
  -I D:/app/Qt/6.12.0/msvc2022_64/qml \
  src/uikit/qml/CatTitleBar.qml tests/auto/qml/data/tst_visualstates.qml
python tools/check_docs.py
openspec validate fix-titlebar-pressed-state --strict --no-interactive
git diff --check
```

均通过，`qmllint` 0 警告。

## 未覆盖

- 未在 macOS、Linux 原生窗口标题栏复核。
- 未在触摸密度、RTL 和高 DPI 下检查按钮背景与图标。
- 未测量 `surfacePressed` 与 `surfaceHover` 在 Dark 主题下的实际观感，只通过 token 断言验证。
