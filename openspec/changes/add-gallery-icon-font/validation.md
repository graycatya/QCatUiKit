# Validation

执行环境：Linux x86_64，Qt 6.12.0 gcc_64，Release 单配置，Ninja，`build-release`。

## 码位核对

字体 `post` 表的字形名不可靠：`close-line`、`check-line`、`add-line` 缺失，`close-fill`、`check-fill` 各对应两个码位且粗细不同。因此把候选码位区间（`ea49`–`ea71`、`eb76`–`eba2`、`ea0c`–`ea16`）渲染成对照图逐项目视确认，最终取：

| 用途 | 码位 |
| --- | --- |
| 左折角 | `ea64` |
| 右折角 | `ea6e` |
| 下折角 | `ea4e` |
| 细勾 | `eb7b` |
| 细叉 | `eb99` |
| 细加号 | `ea13` |

对照图同时确认 `ea5b`、`eb98` 等码位是实心/粗体变体，不是所需字形。

## 自动测试

```sh
cmake --build build-release --parallel
ctest --test-dir build-release --output-on-failure
```

结果：**21/21 通过**，总耗时 189.90 s。`gallery_contracts` 中新增用例 `test_tableColumnMoveIcons` 通过，覆盖：

- `GalleryIcons.loader.status == FontLoader.Ready`
- 首列左移禁用、右移可用；末列反之
- 移动一列后新的首列左移禁用
- 按钮宽高等于 `Theme.tokens.controlHeight`
- 图标 `font.family == "remixicon"`
- 禁用色为 `textDisabled`、可用色为 `text`

## 静态检查

```sh
cmake --build build-release --target qcat-gallery_qmllint      # 通过
python tools/check_docs.py                                      # 通过
git diff --check                                                # 通过
openspec validate add-gallery-icon-font --strict --no-interactive  # 通过
```

### 未通过的检查

`QCatUiKit_qmllint` 目标失败（退出码 255，无诊断输出）。定位结果为 `src/uikit/qml/CatTableView.qml`：

```sh
qmllint qml/CatTableView.qml
# Warning: 518:28 / 572:28 / 584:28 Type "ThemePalette" of property "theme" not found [unresolved-type]
# 退出码 255
```

该文件本次未修改（最后一次变更为 `d7ac6e8`），`src/uikit` 在本次变更中没有改动，因此判断为既有问题，未在本次处理。`build-release/qml/QCatUiKit/QCatUiKit.qmltypes` 已包含 `ThemePalette` 与新增的 `colorPalette`，同一批次中其它文件可正常解析，问题局限于该文件。

## 视觉检查

- 码位对照图：确认六个目标字形与预期一致。
- 表格页截图（离屏软件后端）：列移动按钮显示为折角图标，与相邻复选框同高，首列左折角呈禁用色。截图来自页面局部渲染，底图为透明，不能据此判断整页配色。

采集限制：未在原生图形后端、macOS 或 Windows 复核；未验证触摸密度与 RTL；未测量 613 KB 字体对启动时间的影响。

## 许可

Remix Icon License v1.0（2026-01，字体版本 4.9）允许在 UI kit 与设计系统中作为次要组件使用，禁止作为独立图标库分发或用作 Logo。仓库已保留 `LICENSES/RemixIcon-1.0.txt` 原文并在 `NOTICE` 中登记；字体仅随 Gallery 示例分发，不作为库资源。

## 未覆盖

- 库内部的系统字形（`CatMenuItem`、`CatMultiSelectComboBox`、`CatDropDownButton`、`CatStepper`、`CatBreadcrumb`、`CatCalendar`、`CatAlert`）未改动。
- `CatTabView` 的加号按钮仍为文本 `"+"`。
