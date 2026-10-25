# 构建、安装与应用接入

[文档索引](README.md)

## 环境与选项

需要 CMake 3.24+、C++17 编译器和 Qt 6.8+。Qt 组件为 Core、Gui、Qml、Quick、QuickControls2、ShaderTools，以及匹配版本的 GuiPrivate 开发头（图表及 LED RHI 渲染）。开启 Gallery 时需要 LinguistTools；开启测试时需要 Test、QuickTest。Ninja 可作为单配置或多配置生成器。

Linux 原生 Wayland Dock 的顶层拖动协议探测可选使用 `pkg-config` 和 `wayland-client` 开发头（通常为 `libwayland-dev`）。探测代码运行时动态加载系统 `libwayland-client`，不强制链接该库；缺少开发头时仍能构建，Dock 使用系统移动降级路径，操作差异见 [Dock 原生 Wayland](DOCKING.md#原生-wayland)。无需 Qt Wayland Compositor 开发包。

| CMake 选项 | 默认值 | 用途 |
| --- | --- | --- |
| `QCATUIKIT_BUILD_EXAMPLES` | `ON` | 构建 Gallery 及其内部 QML 模块 |
| `BUILD_TESTING` | `ON` | 构建自动测试并注册 CTest 测试组 |
| `QCATUIKIT_TEST_CHART_GPU` | `OFF` | 注册二维／三维及导出的真实图形窗口测试，需开启测试 |
| `QCATUIKIT_TEST_LED_GPU` | `OFF` | 注册 LED 真实 GPU 测试，需开启测试 |
| `QCATUIKIT_RELEASE_CHANNEL` | `experimental` | `experimental` 或 `stable`，独立于编译模式 |
| `CMAKE_BUILD_TYPE` | 由生成器／调用方决定 | 单配置生成器的 Debug／Release 等模式 |
| `QCATUIKIT_INSTALL_LAYOUT` | `QT` | 跟随所用 Qt 的安装布局；`STANDALONE` 使用 GNUInstallDirs 和 `<libdir>/qml` |
| `QCATUIKIT_INSTALL_QMLDIR` | 跟随安装布局 | 相对安装前缀的 QML 导入根目录；切换布局时用 `-UQCATUIKIT_INSTALL_QMLDIR` 重算默认值 |
| `CMAKE_INSTALL_PREFIX` | 顶层 `QT` 构建默认所用 Qt 前缀 | 安装目标目录；保留显式设置，可用安装命令的 `--prefix` 覆盖 |
| `CMAKE_PREFIX_PATH` | 由调用方提供 | Qt 及已安装依赖的 CMake 包搜索路径 |

构建命令从仓库根目录执行，Qt 路径须替换为本机相应编译器套件。构建输出与源码分离。

## Linux 构建与运行

```sh
cmake -S . -B build/release -GNinja \
  -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/gcc_64 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/release --parallel
./build/release/examples/uikit/gallery/qcat-gallery --language zh_CN
```

单配置构建的 QML 模块位于 `build/release/qml/QCatUiKit`。Gallery 会添加相应的构建目录导入路径。

## Windows 多配置构建

在与 Qt 套件匹配的 MSVC 开发者 PowerShell 中运行：

```powershell
cmake -S . -B build/windows -G "Ninja Multi-Config" -DCMAKE_PREFIX_PATH="C:/Qt/6.x/msvc2022_64"
cmake --build build/windows --config Release --parallel
./build/windows/bin/Release/qcat-gallery.exe --language zh_CN
ctest --test-dir build/windows -C Release --output-on-failure
cmake --build build/windows --config Release --target QCatUiKit_qmllint qcat-gallery_qmllint
```

确保对应 Qt 的 `bin` 位于运行时 PATH。Windows 单配置生成器的程序位于 `<build>/bin/`，多配置生成器位于 `<build>/bin/<config>/`。构建、测试、安装使用同一配置。

## 最小库构建

```sh
cmake -S . -B build/library -GNinja \
  -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/gcc_64 \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=OFF -DQCATUIKIT_BUILD_EXAMPLES=OFF
cmake --build build/library --parallel
```

该配置不需要 LinguistTools、Test 或 QuickTest。库仍包含全部公开组件和图表。交叉编译通过目标平台 Qt 的 CMake toolchain 配置，运行时依赖必须与目标架构匹配。

## 安装

默认按所用 Qt 套件安装，使用与该 Qt 匹配的编译器。在 Windows 的 MSVC 开发者终端中，可一次配置并安装两种编译模式：

```powershell
cmake -S . -B build/qt-module -G "Ninja Multi-Config" `
  -DCMAKE_PREFIX_PATH="C:/Qt/6.x/msvc2022_64" `
  -DQCATUIKIT_BUILD_EXAMPLES=OFF -DBUILD_TESTING=OFF
cmake --build build/qt-module --config Release --parallel
cmake --build build/qt-module --config Debug --parallel
cmake --install build/qt-module --config Release
cmake --install build/qt-module --config Debug
```

全新顶层构建默认将安装前缀设为该 Qt 的 `QT6_INSTALL_PREFIX`。已有构建缓存若保存了其他前缀，需要重新配置 `-DCMAKE_INSTALL_PREFIX=/path/to/Qt/kit`。Windows 的库与插件 Debug 名称带 `d` 后缀，Release 无后缀；CMake 导出文件保存两种配置，QML 插件由 Qt 按当前编译模式选择。非 Windows 平台本次未增加双配置库名，使用各自配置的独立前缀。库使用 Qt 私有渲染头，安装包要求构建时的同一 Qt 版本及兼容工具链。

库、插件及头文件路径使用 Qt 包导出的目标安装布局，包括交叉编译的目标路径，不执行宿主机的 qmake 推测目标路径。QML 插件的相对 RPATH 随安装布局计算。安装到 Qt 套件后，同一套件的应用、QML 工具使用默认路径即可发现模块；发布应用仍需按下文部署依赖，目标机器不会自动拥有开发机的安装目录。

也可以保留 Qt 布局但先安装到独立目录检查：

```sh
cmake --install build/release --prefix "$PWD/stage"
```

多配置构建加 `--config Release`。默认 `QT` 布局中的目录取自 Qt；常见 SDK 如下。设置 `QCATUIKIT_INSTALL_LAYOUT=STANDALONE` 可保留原 GNUInstallDirs 布局，QML 则位于 `<libdir>/qml`。显式覆盖 `QCATUIKIT_INSTALL_QMLDIR` 时，运行时也要使用相应导入路径。

| 安装位置 | 内容 |
| --- | --- |
| `include/QCatUiKit` | 公开头及转发头 |
| `<libdir>`；Windows 的 `bin` | 共享库、链接库 |
| `qml/QCatUiKit`（或 Qt 的 `QT6_INSTALL_QML`） | QML 插件、qmldir、类型信息、QML／JS 文件 |
| `<libdir>/cmake/QCatUiKit`、`<libdir>/cmake/Qt6QCatUiKit` | 独立包导出目标及 Qt6 组件入口 |
| `share/QCatUiKit` | LICENSE、NOTICE 和附带许可证 |

安装目标包含库与 QML 模块；Gallery 从构建目录运行。文档保存在源码仓库。

## 应用接入

安装到所用 Qt 套件后，CMake 只需指定 Qt 的前缀即可：

```cmake
find_package(Qt6 6.8 REQUIRED COMPONENTS Quick QCatUiKit)
target_link_libraries(myapp PRIVATE Qt6::Quick Qt6::QCatUiKit)
```

QML 使用 `import QCatUiKit`，无需 `engine.addImportPath()`。`Qt::QCatUiKit` 也在未禁用无版本目标时提供。QCatUiKit 的 API 版本仍为独立版本，不随 Qt 组件包装入口的版本改变。

使用安装包的最小应用包含下面的 `CMakeLists.txt`、`main.cpp` 和 `Main.qml`。QML 示例可直接使用 [项目 README](../../../README.md) 中的 ApplicationWindow。

```cmake
cmake_minimum_required(VERSION 3.24)
project(UiKitExample LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 6.8 REQUIRED COMPONENTS Quick QuickControls2)
find_package(QCatUiKit 1 REQUIRED)
qt_standard_project_setup(REQUIRES 6.8)
list(APPEND QML_IMPORT_PATH "${QCatUiKit_QML_IMPORT_PATH}")

qt_add_executable(myapp main.cpp)
qt_add_qml_module(myapp
    URI Example
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    NO_IMPORT_SCAN
    IMPORT_PATH "${QCatUiKit_QML_IMPORT_PATH}"
    QML_FILES Main.qml)
target_link_libraries(myapp PRIVATE QCatUiKit::QCatUiKit Qt6::Quick Qt6::QuickControls2)
```

```cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.addImportPath(QCoreApplication::applicationDirPath() + "/qml");
    engine.loadFromModule("Example", "Main");
    if (engine.rootObjects().isEmpty())
        return 1;
    return app.exec();
}
```

从应用目录配置和运行，使用绝对安装路径：

```sh
cmake -S . -B build -GNinja \
  -DCMAKE_PREFIX_PATH="/absolute/stage;/path/to/Qt/6.x/gcc_64" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
QML_IMPORT_PATH=/absolute/stage/qml \
  LD_LIBRARY_PATH=/absolute/stage/lib:/path/to/Qt/6.x/gcc_64/lib \
  ./build/myapp
```

按实际 Qt 布局替换 `lib`、`qml`（`STANDALONE` 为 `<libdir>/qml`）。CMake 包变量 `QCatUiKit_QML_IMPORT_PATH` 给出安装包的 QML 根目录；它不会自动修改独立前缀宿主的运行时导入路径。应用也可以把模块部署到上例的 `<app>/qml/QCatUiKit`。

以上安装包示例通过 NO_IMPORT_SCAN 使用运行时共享插件，并为 qmllint 设置模块搜索路径；完整可复制应用见 [技能模板](../../../skills/qcatuikit-ui/assets/starter)。

源码接入时，在应用的 CMake 中用以下内容替换 `find_package(QCatUiKit ...)`：

```cmake
set(QCATUIKIT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory(path/to/QCatUiKit qcat-uikit)
```

这两个开关作用于共享 CMake 缓存；应用还需要自己的测试时应保留所需的 BUILD_TESTING 设置。链接目标不变，构建目录中的模块位于项目设置的 `QT_QML_OUTPUT_DIRECTORY`；将该路径加入宿主导入路径。

## 部署与排错

将库、QML 插件、qmldir、QML 文件和匹配的 Qt 运行时一起部署。Linux 插件带指向库目录的相对 RPATH；Windows 要使项目和 Qt 的 DLL 可被搜索到；macOS 插件使用相对 loader 路径。应用打包仍需使用目标平台的 Qt 部署工具处理 Qt 模块、平台插件和图像插件。

开发机有多套 Qt 时，CMake 找到正确套件不代表所有间接动态依赖都使用该套件。Linux 的上述运行命令显式设置项目和 Qt 的库路径，避免加载系统 Qt 的间接依赖；部署环境应统一管理运行时搜索路径。

| 现象 | 检查 |
| --- | --- |
| 找不到 Qt6 或 QCatUiKit CMake 包 | CMAKE_PREFIX_PATH 是否指向对应安装前缀 |
| `module "QCatUiKit" is not installed` | QML 导入根目录下是否存在 QCatUiKit/qmldir |
| 模块插件无法加载 | 共享库搜索路径、Qt 版本、架构、编译器与 Debug／Release 是否匹配 |
| 离屏环境无法创建窗口 | 检查平台插件；自动检查可设置 QT_QPA_PLATFORM=offscreen |
| SVG 示例不显示 | 部署 Qt SVG 图像插件；应用图片地址应指向自己的资源 |
| 软件后端效果不同 | 图形特效需在目标硬件后端检查，参见验证说明 |

## 检查安装包

```sh
cmake -S tests/auto/cmake/consumer -B build/consumer -GNinja \
  -DCMAKE_PREFIX_PATH="$PWD/stage;/path/to/Qt/6.x/gcc_64" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/consumer
LD_LIBRARY_PATH="$PWD/stage/lib:/path/to/Qt/6.x/gcc_64/lib" \
  ./build/consumer/consumer
```

其他安装消费者、离屏测试及验证范围见 [验证说明](VALIDATION.md)。公开接口替换检查见 [ABI](ABI.md)。
