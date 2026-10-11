# QCatUiKit

<img src="src/uikit/doc/images/brand/qcat-uikit/png/qcat-uikit-256.png" alt="QCatUiKit Logo" width="96" height="96">

面向 Qt 6 的 QML 组件库。由 QCatGrayQuick 的能力重新设计，提供统一主题、桌面／嵌入式触摸／移动密度，以及采用 Pimpl 的 C++ 共享库。

- Qt **6.8+**、CMake **3.24+**、C++ **17**；只支持 Qt 6。
- 使用 `qt_add_qml_module` 生成 QML 注册、插件、类型信息和资源；`import QCatUiKit`。
- `ThemePalette`、`ChatModel`、`TableModel` 的公开对象只包含私有实现指针；共享库 `SOVERSION 1`。
- 101 个公开 QML 组件（含 3 个原生图表类型），统一颜色、尺寸、焦点、交互状态与减少动效设置。
- 覆盖 Fluent-Qt 的全部 78 种非 Charts 组件类型；[逐项对照与使用差异](src/uikit/doc/REFERENCE_COMPONENTS.md)。
- 中英文 gallery（16 个分类）：分类导航、组件搜索、交互预览、QML 示例和运行时语言切换。
- 图表支持 10 种类型、混合序列、实时环形缓冲、缩放／平移；使用 Qt Quick SceneGraph，无 Qt Charts／Graphs／WebEngine 依赖。[图表使用与迁移](src/uikit/doc/CHARTS.md)。
- 不包含 QCatGrayQuickQrenCodeQml、lottie、Qt5Compat 或第三方图形特效库。
- Apache License 2.0；迁入代码保留原 MIT 许可与归属，存放在 `LICENSES/`。

![桌面示例](src/uikit/doc/images/gallery-desktop.png)

[暗色触摸布局](src/uikit/doc/images/gallery-touch.png) · [移动宽度布局](src/uikit/doc/images/gallery-mobile.png)

## 构建与运行

源码按 Qt 模块习惯组织：`src/uikit` 包含小写命名的 C++ 公开头与实现、`qml/` 和 `doc/`；示例位于 `examples/uikit`，自动测试位于 `tests/auto`。基础版本号集中在 `.cmake.conf`，私有头使用 `_p.h` 后缀。目录与命名约定详见 [架构文档](src/uikit/doc/ARCHITECTURE.md)。

```sh
cmake -S . -B build -GNinja \
  -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/gcc_64 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
cmake --build build --target QCatUiKit_qmllint qcat-gallery_qmllint
./build/examples/uikit/gallery/qcat-gallery
```

Windows 多配置构建的可执行文件位于 `build/bin/<config>/`。

需要 Qt Core、Gui、Qml、Quick、QuickControls2；gallery 还需要 LinguistTools（文件浏览示例使用 QtQuick.Dialogs），测试还需要 Test、QuickTest。最小部署可传入 `-DBUILD_TESTING=OFF -DQCATUIKIT_BUILD_EXAMPLES=OFF`。交叉编译使用目标平台 Qt 的 CMake toolchain；不要把桌面 Qt 的动态库复制到其他架构。

## 组件展示库

顶部可切换中文 / English 和亮暗主题；默认跟随系统语言。16 个分类覆盖所有组件，每个组件都有交互预览和可复制的 QML 示例。设置页提供主题色、交互密度与减少动效。语言切换保留当前组件、输入内容、聊天草稿和所选日期。

启动可传入 `--language zh_CN` 或 `--language en`；`--page calendar` 等参数直接打开组件。运行参数、源码入口与 TS 翻译维护流程见 [Gallery 说明](examples/uikit/gallery/README.md)。

## 使用

```cmake
add_subdirectory(QCatUiKit)
target_link_libraries(myapp PRIVATE QCatUiKit::QCatUiKit)
```

```qml
import QtQuick
import QtQuick.Controls.Basic
import QCatUiKit

ApplicationWindow {
    width: 600; height: 400; visible: true
    color: Theme.tokens.background
    Component.onCompleted: Theme.density = ThemePalette.Touch

    CatButton {
        anchors.centerIn: parent
        text: qsTr("开始")
        primary: true
        onClicked: console.log("activated")
    }
}
```

组件不会自动根据操作系统猜测交互密度。桌面触摸屏应显式选择 `Touch`；移动窗口可选择 `Mobile`。布局使用逻辑像素，缩放交给 Qt 高 DPI 系统。

## 版本查询

默认构建为实验版 `1.0.0-experimental`；正式构建传入 `-DQCATUIKIT_RELEASE_CHANNEL=stable`。

```cpp
#include <QCatUiKit/Version.h>
auto version = QCatUiKit::versionString();
bool experimental = QCatUiKit::isExperimentalRelease();
```

C++ 可使用 `<QCatUiKit/ThemePalette>` 或 `<QCatUiKit/qcatthemepalette.h>` 等 Qt 风格头路径；原有 `<QCatUiKit/ThemePalette.h>`、`<QCatUiKit/Version.h>` 等路径继续兼容。

QML 使用 `QCatUiKit.version`、`QCatUiKit.releaseChannel`、`QCatUiKit.isStable` 和 `QCatUiKit.isExperimental`。旧名称 `LibraryInfo` 继续兼容；新增代码使用 `QCatUiKit`。查询返回实际加载的共享库信息；发布状态与 Debug/Release 编译模式独立。详见 [版本管理](src/uikit/doc/VERSIONING.md)。

## 安装与复用

```sh
cmake --install build --prefix "$PWD/stage"
cmake -S tests/auto/cmake/consumer -B build-consumer \
  -DCMAKE_PREFIX_PATH="$PWD/stage;/path/to/Qt/6.x/gcc_64"
cmake --build build-consumer
./build-consumer/consumer
```

安装后应用使用 `find_package(QCatUiKit 1 REQUIRED)` 与 `QCatUiKit::QCatUiKit`。纯 QML 宿主需要把 `<prefix>/<libdir>/qml` 加到 QML 导入路径。Linux 的模块插件包含指向安装目录中共享库的相对 RPATH；Windows 部署时还需让 `<prefix>/bin` 位于 DLL 搜索路径。应用打包仍应使用目标平台的 Qt 部署工具处理 Qt 插件与运行时依赖。

## 文档

- [图表接口、数据约定与迁移](src/uikit/doc/CHARTS.md)
- [组件与公开行为](src/uikit/doc/COMPONENTS.md)
- [主题架构](src/uikit/doc/THEMING.md)
- [迁移对照与行为变化](src/uikit/doc/MIGRATION.md)
- [版本号与正式／实验发布状态](src/uikit/doc/VERSIONING.md)
- [ABI 边界及替换验证](src/uikit/doc/ABI.md)
- [实现架构与维护约定](src/uikit/doc/ARCHITECTURE.md)
- [本地验证记录](src/uikit/doc/VALIDATION.md)

参考文章 [AI 与开源贡献](https://ant.design/docs/blog/ai-open-source-contribution-cn) 主要用于公开契约、回归测试及审查方法；主题分层参考 [Ant Design 定制主题](https://ant.design/docs/react/customize-theme-cn/)。本库采用 Qt 的对象、焦点、模型和声明式绑定机制实现这些原则。
