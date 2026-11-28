# Tasks

## 1. 预览入口

- [x] 1.1 新增 `GallerySettings` 单例并注册到 Gallery 模块。
- [x] 1.2 `Main.qml` 根布局按 `GallerySettings.rtl` 设置 `LayoutMirroring.enabled` 与 `childrenInherit`。
- [x] 1.3 设置页语言面板增加「从右到左布局」开关。
- [x] 1.4 `main.cpp` 增加 `--rtl` 参数并写入单例；`README.md` 记录参数。

## 2. 测试与翻译

- [x] 2.1 `test_rtlPreview` 断言开关切换后侧边栏换侧并恢复。
- [x] 2.2 补充中英文翻译并移除未完成标记。

## 3. 验证

- [x] 3.1 构建并运行 `gallery_contracts`。
- [x] 3.2 `qcat-gallery_qmllint`、`python tools/check_docs.py`、`git diff --check`、`openspec validate add-gallery-rtl-preview --strict --no-interactive` 通过。
- [x] 3.3 用 `--rtl` 截图确认侧边栏与内容区换侧。
- [x] 3.4 在 `validation.md` 记录命令、结果与未覆盖平台。
