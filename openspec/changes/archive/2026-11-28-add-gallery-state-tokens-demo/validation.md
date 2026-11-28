# Validation

执行环境：Windows 11 x64，Qt 6.12.0 msvc2022_64，Release 单配置，Ninja，MSVC 19.39，`build-led-release`。

## 自动测试

```sh
cmake --build build-led-release --parallel
ctest --test-dir build-led-release -R "^(gallery_contracts|gallery_smoke)$" --output-on-failure
```

结果：**2/2 通过**，`gallery_smoke` 3.28 s，`gallery_contracts` 140.29 s。

新增与更新用例：

- `test_stateTokenDemo`：设置页五个状态色块的颜色分别等于 `Theme.tokens` 中同名项。
- `test_customThemeScheme`：自定义色板下 `surfaceSelected` 为 `#0d5263`、`surfacePressed` 为 `#105d72`、`focusRing` 为 `#2aa198`，`disabledOpacity` 为 45；切回 Light 后 `disabledOpacity` 恢复 60，状态色回到内置值。
- `cleanup` 重置 `Theme.overrides`，避免跨用例泄漏。

## 静态检查

```sh
cmake --build build-led-release --target qcat-gallery_qmllint
python tools/check_docs.py
openspec validate add-gallery-state-tokens-demo --strict --no-interactive
git diff --check
```

均通过。

## 翻译

运行 `cmake --build build-gallery-release --target qcat-gallery_lupdate`。两份 ts 均识别 1682 条源文本，其中 13 条新增、3 条过时条目移除。lupdate 遗留的 15 条未完成条目（含 8 条此前未同步的库字符串）已补齐译文并移除 `type="unfinished"`；`lrelease` 输出 1682 finished / 0 unfinished。

## 视觉检查

使用原生图形会话运行 `qcat-gallery --page settings --language zh_CN --screenshot <临时路径>`：

- 「交互状态」面板按预期显示，中文标题、说明与控件文案均来自新译文。
- 色块顺序为 `surface`、`surfaceHover`、`surfaceSelected`、`surfacePressed`、`focusRing`，颜色与 Light 默认值一致。
- 「选中」按钮显示选中背景与主色边框，「禁用」按钮呈禁用态，复选与开关状态正常。
- 临时截图为一次性证据，已删除，未加入仓库。

观察：Light 下 `surface`（`#ffffff`）与 `surfaceHover`（`#f0f5ff`）差异仍然很弱。本变更只做展示，不调整默认值；该项留待后续可辨识度变更处理。

## 未覆盖

- 未复核 Dark 与自定义色板的实际截图，只通过 token 断言和 Light 视觉检查。
- 未在触摸密度（`hoverEnabled=false`）、RTL、macOS、Linux 复核。
- 未测量新增面板对设置页布局与滚动性能的影响。
