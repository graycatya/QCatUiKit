# Validation

执行环境：Linux x86_64，Qt 6.12.0 gcc_64，Release 单配置，Ninja，`build-release`。

## 自动测试

```sh
cmake --build build-release --parallel
ctest --test-dir build-release --output-on-failure
```

结果：**21/21 通过**，总耗时 203.22 s。

新增与更新用例：

- `tst_models.cpp::stateTokens`：Light/Dark 默认状态色；`focusRing` 跟随 `primaryColor`、`colorPalette` 和显式覆盖；`colorPalette` 状态色覆盖与缺失项回退；`disabledOpacity` 150→100、-5→0、空覆盖→60。
- `tst_extendedcontrols.qml::test_stateTokensDriveControls`：自定义色板下按钮悬停为 `surfaceHover`、按下为 `surfacePressed`、键盘焦点描边为 `focusRing`；复选框按下为 `surfacePressed`；禁用输入框透明度为 `disabledOpacity / 100`。
- `test_spinStepButtonStateVisibility`：步进按钮悬停/按下分别等于 `surfaceSelected` / `surfacePressed`。
- `test_comboPopupStateVisibility`：下拉打开背景与列表当前项及指针高亮项等于 `surfaceSelected`，普通项为表面色。

## 静态检查

```sh
qmllint -I build-release/qml <22 个修改的 QML 文件>   # 通过
python3 tools/check_docs.py                            # 通过
git diff --check                                       # 通过
openspec validate add-state-style-tokens --strict --no-interactive  # 通过
```

## 视觉检查

离屏软件后端分别渲染 Light/Dark 的按钮、复选框和下拉列表：

- Light：悬停底色极浅，按下为 `#d3e3ff`，与悬停可区分；下拉当前项 `#e6f0ff` 加左侧主色条。
- Dark：悬停为 `#303030`，按下为 `#3a4a63`；下拉当前项 `#26344d` 与列表底色可区分。

## 未覆盖

- 未在原生图形后端、macOS、Windows 复核。
- 未复核触摸密度（`hoverEnabled=false`）与 RTL。
- 未测量新增 Token 对渲染耗时的影响。
- `CatTitleBar` 最小化/最大化按钮的 pressed 仍使用 `disabled` 颜色，本次未改为 `surfacePressed`。
- `CatScrollBar` 滑块按下仍使用主色 `primary`，属于主色系反馈，不纳入普通表面状态映射。
