# ABI 与动态库替换

## 支持范围

QCatUiKit 的 ABI 承诺覆盖同一主版本内导出的 **C++ 公开接口**。发布库采用 `VERSION 1.x.y` 和 `SOVERSION 1`，Windows 使用相同 DLL 名称。公开状态类的成员布局是 Qt 基类加一个 `const std::unique_ptr<Impl>`；Impl 定义只存在于 `.cpp`，构造和析构在共享库中执行。导出宏同时支持 Windows import/export 与 ELF 隐藏默认符号可见性。

替换前提：相同 CPU 架构、平台、编译器 ABI、C++ 运行时、兼容的 Qt 动态运行时和一致的构建模式。Pimpl 不承诺跨编译器、跨 Qt 主版本、Windows Debug/Release 混用或进程运行中卸载热替换。先关闭应用，再替换安装包中的库和 QML 模块文件。

## 兼容纪律

- 同一主版本保留类名、命名空间、基类、公开符号、函数签名、对象大小及既有虚函数布局。
- 不给公开对象追加数据成员，不把访问 Impl 的函数改为头文件内联实现。
- 不改变已发布枚举数值、模型角色数值和名称、信号参数及现有属性语义。
- 可在 Impl 内改变存储、算法或缓存；新非虚函数和新 QML 能力仍需逐项审核。
- QML 控件维护属性／信号／方法的源码和行为兼容性。QML 不具有与 C++ 对象内存布局相同的 ABI 概念。
- `SurfaceImpl` 为模块内部类型；控件视觉树、内部 id、内部子对象和 CMake 构建目录不是公开 API。
- QML 编译缓存与 Qt 版本有关。部署共享库、插件、qmldir、qmltypes 和匹配资源为同一发行单元。
- 打破兼容的变更提升主版本与 SOVERSION；`SameMajorVersion` 的包版本规则不是自动 ABI 检查器。

## 实际替换检查（Linux）

`tests/auto/cmake/check_abi_replacement.sh BASELINE_PREFIX CANDIDATE_PREFIX QT_PREFIX [WORK_DIR]`：

1. 只用基线安装包的公开头文件与库编译独立 consumer。
2. 保存该可执行文件，不用候选头文件重新编译。
3. 以候选库目录优先的 `LD_LIBRARY_PATH` 运行原可执行文件。
4. 检查进程打印的实际库路径属于候选目录，并验证主题与两种模型行为。

此检查证明所测 consumer 的二进制替换路径。发布时还应对完整导出符号、类型信息和既有 QML 契约做差异检查，不能仅凭一个 smoke consumer 声称所有历史客户端兼容。当前项目没有已发布的 QCatUiKit 基线，首次迁移不承诺与旧 QCatGrayQuick 的 C++ ABI 或 QML API 兼容。

版本查询函数位于 `qcatuikitversion.h`（兼容路径 `Version.h`），均在共享库内实现。QCatUiKit 是公开的只读 QML 单例，C++ 实现类 QCatUiKitInfo 为私有实现且不导出；旧 QML 名称 LibraryInfo 保留兼容。渠道从 experimental 改为 stable 不改变 SONAME，ABI 仍为同一版本；渠道和数字版本都由运行时查询返回。参见 [版本管理](VERSIONING.md)。

新增公开图表类 `ChartItem`／`ChartAxis`／`ChartSeries` 沿用 Qt 基类 + `const std::unique_ptr<Impl>` 的对象布局，不暴露缓存和数据容器。`ChartSeries::Sample` 是公开的值类型，其字段及枚举数值纳入后续兼容约定。图表公共头继承 QQuickItem，因此 Qt Quick 是共享库的公开链接依赖；CMake 包同步传递该依赖。旧 QtNativeCharts 测试版不纳入本库 ABI 承诺。独立安装消费者位于 `tests/auto/cmake/charts-consumer`。
