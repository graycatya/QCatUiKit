# 版本与发布状态

[文档索引](README.md) · [ABI](ABI.md)

版本数字统一定义在根 `.cmake.conf` 的 `QCATUIKIT_REPO_MODULE_VERSION`，由根 `CMakeLists.txt` 传入 `project(QCatUiKit VERSION ...)`。发布状态使用独立的 CMake 缓存变量 `QCATUIKIT_RELEASE_CHANNEL`，只接受 `stable`（正式）和 `experimental`（实验），默认 `experimental`。输入其他值会在配置阶段失败。

| 构建配置 | 完整版本字符串 | 发布状态 |
| --- | --- | --- |
| 默认或 `-DQCATUIKIT_RELEASE_CHANNEL=experimental` | `1.0.0-experimental` | 实验版 |
| `-DQCATUIKIT_RELEASE_CHANNEL=stable` | `1.0.0` | 正式版 |

`CMAKE_BUILD_TYPE=Debug/Release` 决定编译与优化方式，不改变发布状态。正式版必须显式选择 stable。该标识由发行者声明，不是自动质量认证。

```sh
# 实验版：默认配置
cmake -S . -B build/experimental -GNinja \
  -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/gcc_64 \
  -DCMAKE_BUILD_TYPE=Release -DQCATUIKIT_RELEASE_CHANNEL=experimental
cmake --build build/experimental

# 正式版：使用独立目录
cmake -S . -B build/stable -GNinja \
  -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/gcc_64 \
  -DCMAKE_BUILD_TYPE=Release -DQCATUIKIT_RELEASE_CHANNEL=stable
cmake --build build/stable
```

## C++ 查询

```cpp
#include <QCatUiKit/Version.h>

const QString displayVersion = QCatUiKit::versionString(); // "1.0.0-experimental"
const QVersionNumber numericVersion = QCatUiKit::version(); // 1, 0, 0
const auto channel = QCatUiKit::releaseChannel();           // ReleaseChannel::Experimental
const bool experimental = QCatUiKit::isExperimentalRelease();
const bool stable = QCatUiKit::isStableRelease();
const int abi = QCatUiKit::abiVersion();                    // 1
```

所有函数都是共享库导出的非内联实现，读取**实际加载的库**，不读取客户端编译时头文件中的版本常量。因此，替换兼容动态库后，原客户端可以直接查询新的版本与发布状态。`version()` 返回数字版本供比较，不包含发布状态；判断是否正式发行必须另外检查 releaseChannel。

## QML 查询

```qml
import QtQuick
import QCatUiKit

Text {
    text: QCatUiKit.version + (QCatUiKit.isExperimental ? "（实验版）" : "（正式版）")
}
```

`QCatUiKit` 是模块信息的公开只读单例，属性从 C++ 运行时查询接口取得。C++ 实现类为 `QCatUiKitInfo`，定义在私有头 `qcatuikitinfo_p.h` 中，不作为公开类安装。每个 QML 引擎自行创建单例对象。

使用命名空间导入 `import QCatUiKit as UiKit` 时，访问入口为 `UiKit.QCatUiKit.version`。

这里的 `version` 是已加载的 QCatUiKit 共享库版本，与宿主应用的 `Qt.application.version`、Qt 自身版本以及 `import QCatUiKit 1.0` 的导入版本分别管理。标题中需要显示库版本时可写：

```qml
title: qsTr("QCatUiKit %1 · Component Gallery").arg(QCatUiKit.version)
```

属性列表：

| 属性 | 类型 | 含义 |
| --- | --- | --- |
| `version` | string | 完整字符串，实验版包含 `-experimental` 后缀 |
| `versionMajor` / `versionMinor` / `versionPatch` | int | 主／次／补丁版本数字 |
| `releaseChannel` | string | `stable` 或 `experimental` |
| `isStable` / `isExperimental` | bool | 正式／实验标志，互斥 |
| `abiVersion` | int | 动态库 ABI 主版本 |

属性均为 CONSTANT，不允许 QML 写入。库替换需要先退出进程；不支持进程内卸载后热替换。

## CMake 包元数据

`find_package(QCatUiKit 1 REQUIRED)` 后还可读取：

```cmake
message(STATUS "${QCatUiKit_VERSION}")         # 1.0.0（数字版本）
message(STATUS "${QCatUiKit_VERSION_STRING}")  # 1.0.0-experimental
message(STATUS "${QCatUiKit_RELEASE_CHANNEL}") # experimental
message(STATUS "${QCatUiKit_ABI_VERSION}")     # 1
```

CMake 包版本匹配和动态库文件版本保持数字格式；`find_package` 的版本约束不区分正式／实验。需要正式依赖的应用可在配置时检查 `QCatUiKit_RELEASE_CHANNEL STREQUAL "stable"`。这些 CMake 变量描述找到的安装包，运行时身份应通过上述接口检查。

## 发布规则

- 修改版本数字只编辑根 `.cmake.conf` 的 `QCATUIKIT_REPO_MODULE_VERSION`；运行时信息、动态库文件版本和安装包版本由 CMake 同步生成。
- 正式发行选择 stable；开发及实验构建保留 experimental。渠道变更不改变 ABI 版本。
- 新增兼容功能使用次版本，兼容修复使用补丁版本；打破 ABI 的变更提升主版本及 `QCATUIKIT_ABI_VERSION`。
- QML 的 `import QCatUiKit 1.0` 是独立的模块 API 版本，不能用于判断实际加载库的补丁号或发布状态。
- 两种渠道使用相同的库和模块文件名，应分别安装到不同前缀；切换时按完整发行单元部署。

## 验证运行时替换

`tests/auto/cmake/version_consumer` 是独立安装消费者。先按[构建说明](BUILDING.md)分别安装实验版和正式版，再以实验版安装包编译一次：

```sh
cmake -S tests/auto/cmake/version_consumer -B build/version-consumer -GNinja \
  -DCMAKE_PREFIX_PATH="/path/to/experimental;/path/to/Qt" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/version-consumer
```

Linux 使用同一可执行文件切换运行库；按实际安装布局替换 lib：

```sh
LD_LIBRARY_PATH=/path/to/experimental/lib:/path/to/Qt/lib \
  ./build/version-consumer/version-consumer 1.0.0-experimental experimental
# Linux：同一可执行文件，使用正式版库；不重新编译
LD_LIBRARY_PATH=/path/to/stable/lib:/path/to/Qt/lib \
  ./build/version-consumer/version-consumer 1.0.0 stable
```

测试程序验证完整版本与发布状态，不匹配时返回非零状态。常规 `ctest` 中的 version 测试还会将 C++ 和两个 QML 引擎的结果与各自的 CMake 配置比较。

应使用与构建配置一致的 Qt 运行时。若开发机还安装了不同版本的系统 Qt，Linux 测试时需要将对应 Qt 的 lib 目录一并加入 LD_LIBRARY_PATH，避免间接依赖混用；正式应用通过 Qt 部署工具处理运行时依赖。
