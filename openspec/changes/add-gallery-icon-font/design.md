# Design

## 字体放在 Gallery，不放库

字体放库会带来两个后果：随库分发的安装体积增加约 600 KB，且图标字体的许可会约束整个库的分发方式。Remix Icon License v1.0 允许在 UI kit 中把图标作为次要组件使用，但禁止把图标字体当作独立产品分发。库本体不需要图标字体也能工作，因此字体只随 Gallery 示例分发，并在 NOTICE 中声明它不是库资源。

## 加载方式

两处加载，覆盖不同宿主：

- `main.cpp` 在创建引擎前调用 `QFontDatabase::addApplicationFont`，应用首帧就有字形，不会闪占位方框。
- `GalleryIcons` 单例内保留 `FontLoader`。自动测试直接加载 `QCatUiKit.Gallery` 模块，不经过应用入口；没有这一层，测试里的图标控件会退回默认字体。

`family` 在 `FontLoader` 就绪时取 `loader.name`，否则回退到字面量 `"remixicon"`。`main.cpp` 注册的族名与之一致。

## 码位必须核对，不能按字形名推断

字体 `post` 表的字形名不可靠：`close-line`、`check-line`、`add-line` 三名缺失，`close-fill`、`check-fill` 各自对应两个码位且指向不同粗细的字形。按名字自动生成映射会取到填充版或错误的图标。

因此先渲染全部候选码位为对照图，逐个目视确认后取：左折角 `ea64`、右折角 `ea6e`、细叉 `eb99`、细勾 `eb7b`、下折角 `ea4e`、细加号 `ea13`。

## 单例而非散落码点

`GalleryIcons` 用语义名（`chevronLeft`、`close`、`check`）暴露码位。调用处不出现字符字面量，也不出现 `\uea64` 这类魔法值；换字体时只改一个文件。

## 尺寸与内边距

原按钮设 `implicitWidth: 28`、`implicitHeight: 28`，但沿用 `CatButton` 默认的 12 像素水平内边距，可用内容区只有 4 像素；同时 28 与相邻复选框的 32 不等高，`Flow` 按顶部对齐后不在一条线上。

改为宽高都取 `Theme.tokens.controlHeight`、`padding: 0`、图标 `iconSize: 16`，与复选框同高，字形完整落在内容区内。

## 不做的部分

库内部的系统字形（`CatMenuItem` 的勾选与箭头、`CatMultiSelectComboBox` 与 `CatDropDownButton` 的下折角、`CatStepper` 的勾、`CatBreadcrumb` 与 `CatCalendar` 的箭头、`CatAlert` 的叉号）不在本次范围。它们要么改为绘制图形（与 `CatTitleBar`、页签关闭符号的既有做法一致），要么随库引入字体；这取决于是否接受库增加字体依赖，需要单独决策。
