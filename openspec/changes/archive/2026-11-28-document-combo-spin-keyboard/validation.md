# Validation

执行环境：Windows 11 x64，Qt 6.12.0 msvc2022_64，Release 单配置，Ninja，MSVC 19.39，`build-led-release`。

## 自动测试

```sh
ctest --test-dir build-led-release -R "^qml_contracts$" --output-on-failure
```

结果：**通过**，40.33 s。新增用例：

- `test_comboKeyboardNavigation`：`forceActiveFocus(Qt.TabFocusReason)` 后 `visualFocus` 为 true，`background.border.color` 为 `focusRing`、宽度 2；Space 打开 popup；Down/Up 移动 `highlightedIndex`；Home/End 到首末；Esc 关闭且 `currentIndex` 保持 0。
- `test_spinKeyboardSteps`：Up 从 5 到 6、Down 回到 5；置为 10 后 Up 仍为 10；置为 0 后 Down 仍为 0。

## 探测记录

- 最初使用 `Alt+Down` 打开 ComboBox，在 Windows 测试会话中未触发弹层。改为不可编辑 ComboBox 的 Space 打开方式，行为稳定。未据此修改实现。
- 焦点环断言最初使用默认原因的 `forceActiveFocus()`，`visualFocus` 为 false；改为 `Qt.TabFocusReason` 后通过，符合「键盘焦点才显示焦点环」的设计。
- 两项都属于测试写法调整，不是控件缺陷。

## 静态检查

```sh
python tools/check_docs.py
openspec validate document-combo-spin-keyboard --strict --no-interactive
git diff --check
```

均通过。

## 未覆盖

- 未验证可编辑 ComboBox 的输入过滤与自定义校验器交互。
- 未覆盖 PageUp/PageDown、鼠标滚轮与弹出列表滚动边界的组合。
- 未在 RTL、触摸密度、macOS、Linux 下复核键盘行为。
- `CatSpinBox` 的焦点描边由 `SurfaceImpl` 负责，已在 R19 覆盖，本次未新增像素断言。
