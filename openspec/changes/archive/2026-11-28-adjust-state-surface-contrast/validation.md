# Validation

执行环境：Windows 11 x64，Qt 6.12.0 msvc2022_64，Release 单配置，Ninja，MSVC 19.39，`build-led-release`。

## 自动测试

```sh
cmake --build build-led-release --parallel
ctest --test-dir build-led-release -R "^(models|gallery_contracts|qml_contracts)$" --output-on-failure
```

结果：**3/3 通过**。`models` 0.19 s，`qml_contracts` 41.62 s，`gallery_contracts` 140.84 s。

新增断言：

- `tst_models.cpp::stateTokens` 使用 WCAG 相对亮度计算，断言 Light 的 `surfaceHover`、`surfaceSelected`、`surfacePressed` 与 `surface` 的亮度差均 ≥ 0.015，且依次变暗；Dark 依次变亮。
- 实测相对亮度差：Light hover 0.120、selected 0.200、pressed 0.296；Dark hover 0.019、selected 0.028、pressed 0.072。
- 默认值断言更新为 `#e9f2ff` / `#dbe8ff` / `#c6dcff` 和 `#333333` / `#2b3a52` / `#3f5470`。
- `tst_gallery.qml` 切回 Light 后的 `surfaceSelected` 断言更新为 `#dbe8ff`。

## 静态检查

```sh
python tools/check_docs.py
openspec validate adjust-state-surface-contrast --strict --no-interactive
git diff --check
```

均通过。

## 视觉检查

原生图形会话运行 `qcat-gallery --page settings --language zh_CN --screenshot <临时路径>`：

- Light 下 `surface` 为白，`surfaceHover`、`surfaceSelected`、`surfacePressed` 三级由浅到深均可区分，不再出现 hover 与白底几乎同色的情况。
- 「选中」按钮的背景与边框、禁用按钮、复选与开关显示正常。
- 临时截图为一次性证据，已删除，未加入仓库。

## 未覆盖

- 未在 Dark、macOS、Linux、触摸密度、RTL 下做视觉复核，只通过 token 断言验证 Dark。
- 0.015 是内部可辨识下限，不等同于 WCAG 3:1 非文字对比度要求；未评估打印、色觉障碍或高对比度模式。
- 未测量颜色变化对渲染性能的影响。
- 未更新历史归档 change 中记录的旧默认值。
