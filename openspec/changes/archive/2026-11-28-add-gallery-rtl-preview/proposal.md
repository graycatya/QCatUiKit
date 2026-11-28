# Proposal

## Why

库内已有 RTL 分支（`LayoutMirroring`、`mirrored`、表格列反转代理、Dock 的 `__rtl`），但 Gallery 没有切换入口，也没有 RTL 语言。结果是这些分支只能靠单元测试覆盖，无法在界面上做视觉验收，历史 `validation` 多次记录「未覆盖 RTL」。

## What Changes

- 新增 `GallerySettings` 单例，提供 `rtl` 开关。
- `Main.qml` 的根布局按该开关设置 `LayoutMirroring.enabled` 与 `childrenInherit`。
- 设置页语言面板增加「从右到左布局」开关。
- Gallery 增加 `--rtl` 命令行参数，启动即镜像，便于截图与演示。
- 增加 `test_rtlPreview`，断言开关切换后侧边栏换侧并恢复。
- 补充 Gallery 参数文档与中文翻译。

不新增 RTL 语言的翻译，不改变库的 `LayoutMirroring` 契约，也不把 Gallery 布局镜像当作库能力证明。

## Capabilities

### New Capabilities

无。

### Modified Capabilities

- `gallery`：新增 R08 `RtlPreview`。

## Impact

影响 `examples/uikit/gallery/GallerySettings.qml`（新增）、`Main.qml`、`GallerySettingsPage.qml`、`main.cpp`、`CMakeLists.txt`、翻译、README 和 `tests/auto/gallery/data/tst_gallery.qml`。库源码不变。
