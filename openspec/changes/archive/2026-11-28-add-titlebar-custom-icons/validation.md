# Validation

执行环境：Windows 11 x64，Qt 6.12.0 msvc2022_64，Release 单配置，Ninja，MSVC 19.39，`build-led-release`。

## 码位核对

用临时安装的 fontTools 4.69.1 读取 `examples/uikit/gallery/assets/remixicon.ttf` 的 cmap，筛出候选：

- `subtract-line` `0xf1af`
- `checkbox-blank-line` `0xeb7f` 与 `0xf3dc`
- `checkbox-multiple-blank-line` `0xeb87`
- `close-fill` `0xeb98` 与 `0xeb99`

再用 QML 渲染候选码位为对照图并目视确认：`f1af` 为横线、`f3dc` 为较细方框、`eb87` 为双框、`eb99` 为细叉。最终采用 `f1af` / `f3dc` / `eb87` / `eb99`。参照 `add-gallery-icon-font` 的做法，码位不按字形名推断。

## 自动测试

```sh
cmake --build build-led-release --parallel
ctest --test-dir build-led-release -R "^(qml_contracts|gallery_contracts)$" --output-on-failure
```

结果：**2/2 通过**。`qml_contracts` 42.51 s，`gallery_contracts` 150.27 s。

新增用例 `tst_visualstates.qml::test_titleBarCustomIcons`：

- 未提供图标时，`titleBarMinimizeGlyph`、`titleBarMaximizeGlyph`、`titleBarCloseGlyph` 均可见。
- 提供四个自定义组件后，三个默认绘制图标隐藏，自定义组件存在，最小化按钮宽度仍为 46。
- 原 `test_titleBarPressedState` 继续通过，状态背景未受影响。

## 静态检查

```sh
qmllint -I build-led-release/qml -I build-led-release/qml/QCatUiKit \
  -I D:/app/Qt/6.12.0/msvc2022_64/qml \
  src/uikit/qml/CatTitleBar.qml tests/auto/qml/data/tst_visualstates.qml
cmake --build build-led-release --target qcat-gallery_qmllint
python tools/check_docs.py
openspec validate add-titlebar-custom-icons --strict --no-interactive
git diff --check
```

均通过，库文件 `qmllint` 0 警告。

## 视觉检查

原生图形会话运行 `qcat-gallery --page title-bar --language zh_CN --screenshot <临时路径>`：

- 示例窗口标题栏显示 RemixIcon 的横线、方框和叉号，替换了原来的绘制图标。
- 按钮尺寸、布局和窗口位置未变。
- 临时截图为一次性证据，已删除，未加入仓库。

## 未覆盖

- 自定义图标不自动继承按钮的悬停、按下和禁用颜色；Gallery 示例固定使用 `textSecondary`。已写入 `COMPONENTS.md`。
- 未在最大化状态下目视复核 `restoreIcon` 的实际渲染，仅由绑定逻辑和测试覆盖。
- 未在 macOS、Linux、RTL、触摸密度和高 DPI 下复核。
- 未把 RemixIcon 引入库本身；字体仍只随 Gallery 分发。
