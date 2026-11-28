# Validation

执行环境：Windows 11 x64，Qt 6.12.0 msvc2022_64，Release 单配置，Ninja，MSVC 19.39，`build-led-release`。

## 自动测试

```sh
cmake --build build-led-release --parallel
ctest --test-dir build-led-release -R "^gallery_contracts$" --output-on-failure
```

结果：**通过**，115.92 s。新增用例 `test_rtlPreview`：

- 初始 `GallerySettings.rtl` 为 false，侧边栏 `x` 为 0。
- 打开开关后 `rtl` 为 true，侧边栏 `x` 大于窗口宽度一半。
- 关闭开关后 `rtl` 恢复 false，侧边栏 `x` 回到 0。

`cleanup` 重置 `GallerySettings.rtl`，避免影响其他用例。

## 静态检查

```sh
cmake --build build-led-release --target qcat-gallery_qmllint
python tools/check_docs.py
openspec validate add-gallery-rtl-preview --strict --no-interactive
git diff --check
```

均通过。

## 视觉检查

原生图形会话运行 `qcat-gallery --page settings --rtl --language zh_CN --screenshot <临时路径>`：

- 侧边栏移到窗口右侧，内容区移到左侧。
- 顶部栏、导航项内部布局、面板文字对齐和状态色块顺序均镜像。
- 中文文案保持不变，说明只镜像布局方向。
- 临时截图为一次性证据，已删除，未加入仓库。

## 未覆盖

- 未在阿拉伯语、希伯来语等真实 RTL 语言下检查混排、数字与标点。
- 未复核触摸密度、窄屏折叠导航与 RTL 的组合。
- 未在 macOS、Linux 复核。
- 未把 Gallery 镜像结果当作库组件全部 RTL 场景的验收证据。
