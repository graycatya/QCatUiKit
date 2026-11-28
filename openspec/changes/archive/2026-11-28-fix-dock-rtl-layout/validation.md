# Validation

执行环境：Windows 11 x64，Qt 6.12.0 msvc2022_64，Release 单配置，Ninja，MSVC 19.39，`build-led-release`。

## 问题定位

- `CatDockWorkspace` 没有 RTL 状态，`DockTabStrip` 的按钮位置、关闭目标、标签排列和滚动方向全部按 LTR 写死。
- Gallery 的 `LayoutMirroring` 在 `ScrollView` 之后没有传递到 `CatDockWorkspace`，仅靠继承得到 false。
- 先尝试 `control.mirrored` 与父项 `childrenInherit`，在 Gallery 与测试中都不稳定；最终改为显式 `rtl` 属性加 `LayoutMirroring` 兜底。

## 修复

- `CatDockWorkspace` 新增 `rtl`，`__rtl = rtl || LayoutMirroring.enabled`。
- `DockTabStrip`：RTL 下标签 `layoutDirection` 反转、关闭目标移到左端、滚动按钮交换位置与箭头方向、初始滚动到第一个标签、滚轮与按钮方向反转；延迟滚动回调增加销毁保护。
- `Row.layoutDirection` 仅在未启用 `LayoutMirroring` 时显式设置，避免双重镜像。

## 自动测试

```sh
cmake --build build-led-release --parallel
ctest --test-dir build-led-release -R "^(qml_contracts|gallery_contracts)$" --output-on-failure
```

结果：**2/2 通过**。`qml_contracts` 40.81 s，`gallery_contracts` 138.87 s。

- `tst_dock.qml::test_rtlTabStrip`：`rtl: true` 时 `__rtl` 为 true；220 像素宽、两个长标题时第一个标签在第二个标签右侧；关闭目标 `x < 28`。
- `tst_gallery.qml::test_rtlPreview`：Gallery 打开 RTL 后，Dock 示例的 `rtl` 与 `__rtl` 均为 true；关闭后恢复。
- 原 `test_overflowDragScrollsWhilePointerIsStill`、`test_scrollIconOverride` 继续通过。

## 静态检查

```sh
cmake --build build-led-release --target QCatUiKit_qmllint
cmake --build build-led-release --target qcat-gallery_qmllint
python tools/check_docs.py
openspec validate fix-dock-rtl-layout --strict --no-interactive
git diff --check
```

均通过。

## 视觉检查

`qcat-gallery --page dock-workspace --rtl --language zh_CN --width 900 --screenshot <临时路径>`：

- 第二组标签栏从「编辑器、图表预览」变为「图表预览、编辑器」，首个标签位于右侧。
- 关闭目标、滚动箭头位置与方向随 RTL 镜像。
- 标签宽度、选中下划线和面板内容未变。
- 临时截图为一次性证据，已删除，未加入仓库。

## 未覆盖

- RTL 下的拖动换序预览坐标（`preview()`、`scrollDirection`）仍按视觉 x 计算，未做完整镜像；跨栏拖动换序在 RTL 下需要单独验收。已在 R06 中不承诺该场景。
- 未截图验证浮动窗口内的 RTL 标签栏，只做了属性传递与同一 `DockTabStrip` 复用。
- 未验证真实 RTL 语言混排、RTL 下的触摸密度、macOS 与 Linux。
- 未把 Gallery 的镜像结果当作全部 Dock 布局（分割、浮动、回停）的 RTL 验收。
