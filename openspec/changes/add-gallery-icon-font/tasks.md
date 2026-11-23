# Tasks

## 1. 资源与加载

- [x] 1.1 将 `assets/remixicon.ttf` 加入 Gallery QML 模块的 `RESOURCES`。
- [x] 1.2 `main.cpp` 在创建引擎前同步调用 `QFontDatabase::addApplicationFont`，失败时输出警告。
- [x] 1.3 新增 `GalleryIcons` 单例：集中声明字体族与语义化码位，并保留 `FontLoader` 供不经过应用入口的宿主使用。
- [x] 1.4 在 Gallery 的 QML 模块中注册该单例（`QT_QML_SINGLETON_TYPE`）。

## 2. 示例修复

- [x] 2.1 表格列移动按钮改为图标按钮，`◀`／`▶` 字符不再使用。
- [x] 2.2 按钮宽高改为 `Theme.tokens.controlHeight`，`padding: 0`，图标 `iconSize: 16`，与相邻复选框同高且字形不溢出。
- [x] 2.3 图标颜色跟随主题：可用为 `tokens.text`，禁用为 `tokens.textDisabled`。

## 3. 第三方资源登记

- [x] 3.1 新增 `LICENSES/RemixIcon-1.0.txt`（上游原文）。
- [x] 3.2 `NOTICE` 增加来源、字体版本 4.9、许可证与"仅随 Gallery 分发、不是库资源"的说明。
- [x] 3.3 `examples/uikit/gallery/README.md` 说明字体用途与许可位置。

## 4. 测试

- [x] 4.1 `tst_data.qml::test_tableColumnMoveIcons`：字体 `FontLoader` 为 Ready；首列左禁用／右可用，末列相反；移动后边界按新顺序重算。
- [x] 4.2 同一用例断言按钮尺寸为 `controlHeight`、图标字体族为 `remixicon`、禁用与可用颜色分别取主题 Token。
- [x] 4.3 码位不按字形名推断：渲染候选码位对照图并逐项目视确认后写入单例。

## 5. 集成检查

- [x] 5.1 Release 构建通过；`ctest` 21/21 通过。
- [x] 5.2 `qcat-gallery_qmllint`、`python tools/check_docs.py`、`git diff --check`、`openspec validate add-gallery-icon-font --strict --no-interactive` 通过。
- [x] 5.3 `QCatUiKit_qmllint` 目标失败，但失败点为未改动的 `src/uikit/qml/CatTableView.qml`，单独运行该文件可复现，与本次改动无关；已记入 validation.md。

## Workflow follow-up

- 归档时把 R06 同步进 `gallery` 主规格。
- 库内部系统字形（菜单勾选与箭头、下拉折角、步骤勾、面包屑箭头、警告叉号）另立项：改为绘制图形或接受随库字体依赖，两者取舍见 design.md。
