# 构建、安装与应用接入

[文档索引](README.md)

## 环境与选项

需要 CMake 3.24+、C++17 编译器和 Qt 6.8+。Qt 组件为 Core、Gui、Qml、Quick、QuickControls2、ShaderTools，以及匹配版本的 GuiPrivate 开发头（图表及 LED RHI 渲染）。开启 Gallery 时需要 LinguistTools；开启测试时需要 Test、QuickTest。Ninja 可作为单配置或多配置生成器。

Linux 原生 Wayland Dock 的顶层拖动协议探测可选使用 `pkg-config` 和 `wayland-client` 开发头（通常为 `libwayland-dev`）。探测代码运行时动态加载系统 `libwayland-client`，不强制链接该库；缺少开发头时仍能构建，Dock 使用系统移动降级路径，操作差异见 [Dock 原生 Wayland](DOCKING.md#原生-wayland)。无需 Qt Wayland Compositor 开发包。

| CMake 选项 | 默认值 | 用途 |
| --- | --- | --- |
| `QCATUIKIT_BUILD_EXAMPLES` | `ON` | 构建 Gallery 及其内部 QML 模块 |
| `QCATUIKIT_ENABLE_THORVG` | `ON` | 自动准备并构建三种矢量 UI 组件的实验 QRhi 后端 |
| `QCATUIKIT_THORVG_SOURCE_DIR` | 空 | 开发用源码覆盖；默认自动下载固定源码并应用仓库内 QRhi 实现 |
| `QCATUIKIT_BUILD_THORVG` | 跟随 ENABLE_THORVG | 通过 Meson 构建静态库；仅 CPU 模式使用 v1.1.3 |
| `BUILD_TESTING` | `ON` | 构建自动测试并注册 CTest 测试组 |
| `QCATUIKIT_BUILD_MANUAL_TESTS` | `OFF` | 构建 tests/manual 下的交互验证程序，需开启测试 |
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

## Qt Creator 自动配置与 ThorVG

clone 后直接用 Qt Creator 打开根目录 `CMakeLists.txt`，选择匹配的 Qt 6 MSVC 套件并配置 Debug 或 Release，然后构建。默认启用 SVG、Lottie 和画布后端，无需准备相邻 ThorVG 仓库、安装 Meson 或设置本机源码路径。首次配置需要联网；下载均校验固定 SHA-256，并使用文件锁防止不同构建目录同时解压。

| 路径选项 | 默认目录 | 内容 |
| --- | --- | --- |
| `QCATUIKIT_THIRDPARTY_DIR` | `<source>/3rdparty` | 校验并准备好的依赖源码 |
| `QCATUIKIT_THIRDPARTY_DOWNLOAD_DIR` | `<source>/3rdparty-download` | 可复用的原始压缩包 |
| `QCATUIKIT_BUILD_TOOLS_DIR` | `<source>/3rdparty-tools` | Meson 1.9.2 与按需下载的 Python |

这些目录不进入 Git，可在首次配置时修改为共享缓存目录。Windows x64 优先使用已有 Python 3.7+；没有时自动下载独立 Python 3.12.10，不修改系统或运行 pip。其他主机需要 Python 3.7+。Ninja 使用 Qt Creator 套件提供的工具。可用 `QCATUIKIT_PYTHON_EXECUTABLE`、`QCATUIKIT_MESON_EXECUTABLE`、`QCATUIKIT_NINJA_EXECUTABLE` 覆盖工具；`QCATUIKIT_BOOTSTRAP_PYTHON=ON` 可验证独立 Python 路径。

CPU-only 和默认 QRhi 构建均固定使用官方 `v1.1.3` 源码及相同 SHA-256 校验。QRhi 是本项目维护的适配实现：在该官方版本上应用 `adapters/qcatthorvgqrhi/overlay`，不要求官方提供 QRhi，保持指定的基础版本。三个 Meson 覆盖入口以 v1.1.3 为基础，仅加入 QRhi 接入，保留上游版本号与其他选项。来源见 [实现记录](../../../adapters/qcatthorvgqrhi/README.md)。适配画布类型为 `QCatUiKit::ThorvgQrhiCanvas`，自有实现按 QCatUiKit 命名，适配入口仅为 `qcatthorvgqrhicanvas.h`。源码目录标识包括补丁内容指纹；文件列表及内容变化都会触发重新配置，更新实现会生成新的依赖目录。可用 `QCATUIKIT_THORVG_SOURCE_DIR` 显式选择自维护源码，该目录必须提供当前适配头 `inc/qcatthorvgqrhicanvas.h`。

配置阶段准备源码和工具，编译阶段通过 `ExternalProject` 调用 Meson 构建和安装。源码和压缩包可在 Debug／Release 间共用；产物分别保存在 `<build>/third_party/thorvg-<identity>/meson/<config>` 和 `install/<config>`，MSVC 使用 `/MDd`／`/MD`。启用 CPU、QRhi 和 SVG／Lottie／TTF 加载器，关闭额外工具及测试；静态库名为 `libthorvg-1.a`，私有链接进 QCatUiKit，不单独安装到 Qt。

离线时可复用上述三个目录，或预先放入与 CMake 中 URL、文件名、SHA-256 一致的压缩包；首次离线构建前仍须准备完整缓存。修改本地源码后，正常构建会调用 Meson 增量编译。交叉编译必须提供 `QCATUIKIT_THORVG_CROSS_FILE`，不自动转换 CMake toolchain；本次验证范围为 Windows x64 MSVC。

禁用矢量渲染使用 `-DQCATUIKIT_ENABLE_THORVG=OFF -DQCATUIKIT_BUILD_THORVG=OFF`。组件类型仍保留，报告 `backendAvailable=false`。仅验证官方 v1.1.3 的 CPU 依赖使用 `-DQCATUIKIT_ENABLE_THORVG=OFF -DQCATUIKIT_BUILD_THORVG=ON`。额外 Meson 参数可通过 `QCATUIKIT_THORVG_MESON_OPTIONS` 传入，但不得改变静态库类型、安装目录或库名。组件行为及限制见 [矢量图形与动画](VECTOR.md)。

MSVC 编译器输出经 `cmake/msvc_compiler_launcher.py` 统一为 UTF-8 和固定 ASCII include 前缀，避免中文工具链在 Qt Creator 无控制台环境下把本地编码写入 Ninja 规则，同时保留真实编译返回码。GuiPrivate 的版本绑定警告属于 Qt 私有接口的正常提示；应使用构建时匹配的 Qt 套件。

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

### Qt Creator 的 Ninja 规则与类型信息

如果 CMake 在 `ninja -t recompact` 阶段提示 `unknown build rule`，先核对同一构建目录的 `build.ninja` 与 `CMakeFiles/rules.ninja` 是否一致。配置曾中断、生成文件来自不同轮次时可能出现规则引用缺失；不能仅凭该错误认定某个 C++ 测试无法编译。避免 IDE 和命令行同时配置同一个构建目录。

在对应编译器环境重新配置现有目录，然后构建类型注册目标及应用：

```sh
cmake -S . -B <build>
cmake --build <build> --target QCatUiKit_qmltyperegistration qcat-gallery-ui_qmltyperegistration
cmake --build <build> --parallel
```

Gallery 关闭时省略它的类型注册目标。`.qmltypes` 由 Qt 工具生成，首次构建前 IDE 可能报告尚不存在；应检查 `<build>/qml/QCatUiKit/QCatUiKit.qmltypes` 及 Gallery 子目录中的 `qcat-gallery-ui.qmltypes`，不要手写空文件或复制其他构建配置的产物。构建成功后重新加载项目，让 Qt Creator 刷新 QML 类型信息。GuiPrivate 的 Qt 版本绑定提示与 Ninja 规则错误是不同事项。

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

## 角色播放开发工具

启用 `BUILD_TESTING=ON` 和 `QCATUIKIT_BUILD_MANUAL_TESTS=ON` 时构建 `qcat-mascot-player`，用于查看角色清单、动作与精灵／Lottie 播放。它是普通预览窗口，桌面悬浮集成仍待实现。当前接口和独立安装消费者用法见 [角色包与动画播放](MASCOTPET.md) 与 [工具说明](../../../tests/manual/mascot-player/README.md)。
