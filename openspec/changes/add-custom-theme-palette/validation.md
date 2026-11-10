# Validation

执行环境：Linux x86_64，Qt 6.12.0 gcc_64，Release 单配置，Ninja，`build-release`。

## 自动测试

```sh
cmake --build build-release --parallel
ctest --test-dir build-release --output-on-failure
```

结果：**21/21 通过**，总耗时 193.21 s。

与本变更直接相关的套件：

| 套件 | 结果 | 覆盖内容 |
| --- | --- | --- |
| `models` | 通过 | 色板替换、回退、隔离、派生重算、扩展键保留、钳制与告警 |
| `qml_contracts` | 通过（31.55 s） | 同页两套主题隔离、运行时切换、局部主题销毁回退 |
| `gallery_contracts` | 通过（126.08 s） | 设置页三方案切换、双主题分区颜色、清空后回到内置色板 |

其余 18 个套件（图表、LED、Dock、矢量、桌宠、体感、版本等）未涉及本变更，随全量 `ctest` 通过。

## 静态检查

```sh
cmake --build build-release --target QCatUiKit_qmllint
cmake --build build-release --target qcat-gallery_qmllint
python tools/check_docs.py
git diff --check
openspec validate add-custom-theme-palette --strict --no-interactive
```

全部通过。`check_docs.py` 报告 38 个 Markdown 文件、528 个本地链接、123 个公开 Cat 类型检查通过。

## 翻译

`qcat-gallery_lupdate` 更新后新增 7 条文案，`qcatgallery_en.ts` 与 `qcatgallery_zh_CN.ts` 均已补齐。剩余未完成条目（`Cannot load sprite atlas: %1`、`Character unavailable`）为本次改动前既有项，未处理。

## 视觉检查

在离屏软件后端渲染 `GallerySettingsPage.qml`（960 逻辑像素宽），分别截取亮色与自定义色板两种状态：

- 亮色：出现三个方案按钮（亮色选中），"两套主题并排显示"分区中左侧为白底描边面板，右侧为深青面板且圆角更大，两侧按钮与次要文字均可辨认。
- 自定义色板：整页背景变为 `#002b36`，面板 `#073642`，正文与次要文字使用 `#93a1a1`／`#839496`，边框 `#586e75`；标题与说明对比正常。

采集限制：截图来自设置页单独渲染，不是完整 `qcat-gallery` 窗口，未覆盖窗口主题、导航栏与触摸密度；未在原生图形后端、macOS 或 Windows 复核，也未做屏幕阅读器验证。

## 未覆盖

- 三方色板在真实项目中的对比度取舍由应用决定，主题不校验最终合成对比度（既有边界）。
- `colorPalette` 只接受颜色 token；非颜色键与非法颜色输出 `qcatuikit.theme` 警告，这一行为在自动测试中通过结果断言覆盖，未断言告警文本本身。
- 字号／圆角分级、组件解剖注入不在本次范围。
