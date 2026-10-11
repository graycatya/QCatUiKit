# QCatUiKit Gallery

展示库按 16 个分类组织 101 个公开 QML 类型。首页提供常用组件入口，搜索支持组件类型名和当前语言的标题、说明；详情页包含可操作的预览、重置按钮和可复制的 QML 示例。窄窗口通过侧边抽屉导航。

顶部可切换中文和 English、亮色和暗色。设置页提供主题色、桌面／触摸／移动密度和减少动效。语言默认跟随系统：中文系统使用简体中文，其余使用英文；选择仅在当前运行期间生效。

参考组件的 78 项逐项映射及 QML 使用差异见 [组件对照](../../../src/uikit/doc/REFERENCE_COMPONENTS.md)。

## 运行

构建 gallery 需要 Qt 6.8+ 的 LinguistTools。翻译会随默认构建编译为 QM 并嵌入程序资源。

```sh
cmake --build build --target qcat-gallery
./build/examples/uikit/gallery/qcat-gallery --language zh_CN
./build/examples/uikit/gallery/qcat-gallery --language en --page calendar
./build/examples/uikit/gallery/qcat-gallery --dark --density touch --page settings
```

Windows 单配置构建使用 `build/bin/qcat-gallery.exe`，多配置构建在 `build/bin/<config>/`。省略 `--language` 即跟随系统语言。

`--page` 支持 `home`、`all`、`settings`，以及 `GalleryCatalog.qml` 中的组件 ID，如 `button`、`busy`、`calendar`、`chat`、`table`。`--width 390 --density mobile` 可检查窄屏布局。`--smoke-test` 在加载后自动退出；`--screenshot <path>` 渲染截图后退出。

## 翻译维护

英文源码使用 `qsTr()`。两个完整的 Qt Linguist 翻译目录位于 `translations/qcatgallery_en.ts` 和 `translations/qcatgallery_zh_CN.ts`；同时收集 gallery 与 QCatUiKit 控件的文本，覆盖分页、聊天状态和无障碍提示。

```sh
cmake --build build --target qcat-gallery_lupdate
# 用 Qt Linguist 补全两个 TS 文件；保留 %1、%2 等占位符。
cmake --build build --target qcat-gallery_lrelease
cmake --build build --target qcat-gallery
```

[Qt 的 qt_add_translations 文档](https://doc.qt.io/qt-6/qtlinguist-cmake-qt-add-translations.html)说明了翻译提取、编译和资源嵌入流程。

`GalleryLanguage` 负责安装翻译、更新默认 QLocale 并重新计算 QML 翻译绑定。日历使用显式 locale 绑定，示例表格和欢迎消息也会更新语言；用户输入、消息、所选日期和当前组件保持不变。组件类型名与 QML 代码中的 API 标识保持英文。

Qt 自带的 qtbase 与 qtdeclarative 中文翻译在运行时从 Qt 的翻译目录加载（如果已部署）。项目自己的 QM 已嵌入，不需要另放在可执行文件旁。图片示例使用随程序嵌入的 SVG，运行环境需包含 Qt 的 SVG 图片插件。软件渲染下视觉效果页显示原始图片，阴影效果应在 GPU 后端验收。

## 品牌资源

侧栏、首页和窄屏标题栏统一使用完整灰猫与组件角标。标识保留原配色，不随主题色切换而重新着色。`GalleryLogo.qml` 统一处理显示方式，资源来自 [项目 Logo 目录](../../../src/uikit/doc/images/brand/qcat-uikit/README.md)。

16–512 px PNG 随 gallery 嵌入；应用窗口图标使用多尺寸 PNG，Windows exe 图标通过独立的 RC 资源嵌入原始 ICO。运行和构建不依赖 CatCommStudio 仓库。
## 源码入口

- `Main.qml`：窗口、导航状态、搜索和响应式布局。
- `GalleryCatalog.qml`：组件分类、说明与示例代码。
- `GalleryLogo.qml`：品牌标识显示。
- `GalleryPreview.qml`：各组件的可操作示例。
- `GalleryHome.qml`、`GalleryComponentPage.qml`、`GallerySettingsPage.qml`：页面。
- `gallerylanguage.h/.cpp`：运行时语言切换。

`qcat-gallery-ui` 是供程序和 gallery 测试复用的内部静态 QML 模块。关闭 `QCATUIKIT_BUILD_EXAMPLES` 后不构建该模块，也不需要 LinguistTools。

自动测试在 `tests/auto/gallery`，覆盖两种语言下所有组件页面、搜索、输入与消息保留、日历 locale，以及弹出层和移动导航的语言切换。
示例代码均通过动态创建检查。图片示例引用 gallery 内嵌的演示资源；用于其他应用时请替换为自己的图片资源地址。

## 图表演示

图表分类包含 CatChart、CatChartAxis、CatChartSeries 三个页面。预览选择器提供原测试项目的 15 个演示：折线、面积、分组柱状、堆叠柱状、散点、气泡、饼图、环形、雷达、热力图、柱状与折线混合、时间轴、对数轴、正负堆叠、散点与趋势线。折线演示每条序列保留 100,000 点，可以启动／暂停实时追加；数据序列页可调整缓冲容量。滚轮缩放、拖动平移、图例切换以及键盘操作均在预览中可用。

`--page chart --chart-demo 7` 可直接打开环形图，`--page chart-axis` 默认打开时间轴。`--chart-demo` 接受 0–14，需配合上述图表页面或 `chart-series`。继续支持 `--language`、`--dark`、`--width`、`--density` 和 `--screenshot`。

演示数据及其翻译位于 `gallerychartdata.cpp`，属于 gallery 应用，不进入库的公开 API。语言切换保留实时数据、暂停状态、序列显隐与手动范围；重置示例才重新生成数据。
