# 架构与源码组织

[文档索引](README.md)

## 模块组成

```text
应用 QML / C++
  ├─ QCatUiKit QML 模块
  │    ├─ Cat 组件：Qt Quick Controls / Templates 与声明式组合
  │    ├─ Theme / ThemePalette：主题配置 → 语义 Token → 控件
  │    ├─ TableModel / ChatModel / TreeModel：数据模型
  │    └─ CatChart / CatChartAxis / CatChartSeries：图表
  └─ QCatUiKit 共享库
       ├─ 主题、表格、消息的公开 C++ 类
       ├─ ChartItem / ChartAxis / ChartSeries 与渲染实现
       ├─ 运行时版本查询
       └─ 私有实现、容器、采样和布局算法

Gallery 应用
  ├─ 导航、搜索、设置、组件目录与示例
  ├─ 中英文翻译管理
  └─ 图表／数据演示控制器与持续压力测试
```

`qt_add_qml_module` 生成类型注册、插件、资源及工具类型信息。库的 QML URI 为 `QCatUiKit`，资源前缀为 `/qt/qml`。Gallery 使用内部静态模块 `QCatUiKit.Gallery`，由可执行程序和 Gallery 测试复用。

## 目录

| 路径 | 职责 |
| --- | --- |
| `.cmake.conf` | 基础版本数字 |
| 根 `CMakeLists.txt` | 依赖、构建选项、发布渠道和包导出 |
| `cmake/` | 安装包配置模板 |
| `src/uikit/` | 共享库 C++ 公开头、私有头与实现 |
| `src/uikit/qml/` | Cat 组件、Theme、内部视觉实现与变换辅助代码 |
| `src/uikit/doc/` | 当前功能、接入、维护和验证文档 |
| `examples/uikit/gallery/` | Gallery 页面、控制器、翻译及演示资源 |
| `tests/auto/models/` | 主题、消息与表格模型协议 |
| `tests/auto/charts/` | 图表、坐标和序列数据 |
| `tests/auto/version/` | C++／QML 版本身份 |
| `tests/auto/qml/` | 控件行为、视觉状态、动画时序 |
| `tests/auto/gallery/` | 页面、示例、翻译和演示数据控制器 |
| `tests/auto/cmake/` | 安装消费者与动态库替换检查 |
| `LICENSES/` | 所包含代码的附带许可证 |

C++ 文件采用小写名称，私有头以 `_p.h` 结尾且不安装。QML 文件名保持类型名称，例如 `CatButton.qml`。

## 公开接口与安装

C++ 公开状态类采用 Qt 基类加 `const std::unique_ptr<Impl>` 的布局，状态与算法位于实现文件。构造、析构和导出函数在共享库中执行；兼容边界见 [ABI](ABI.md)。

真实公开头位于 `src/uikit`。配置阶段生成构建目录的转发头；安装包提供真实头和相对路径转发头，例如：

- `<QCatUiKit/qcatthemepalette.h>`
- `<QCatUiKit/ThemePalette>`
- `<QCatUiKit/ThemePalette.h>`

CMake 统一链接 `QCatUiKit::QCatUiKit`。QML 公开名称由组件文件、注册宏和模块 CMake 定义。TreeModel 与版本信息单例的 C++ 实现为私有；SurfaceImpl 是内部 QML 类型，应用不依赖其对象树或 id。

## 状态、线程与视图

主题配置生成只读 Token map，控件通过绑定更新。Theme 按 QML 引擎创建；局部 ThemePalette 由调用方持有并显式传入。组合组件把主题继续传给其内部控件。

模型只管理数据与标准 Qt 模型通知，不决定光标、列宽或窗口大小。TableModel 使用 QAbstractTableModel，ChatModel 使用 QAbstractListModel，TreeModel 提供只读层次数据。视图负责布局、选择、滚动与 delegate。

所有 QObject API 在所属线程调用。工作线程准备批次，通过 queued connection 交给模型或图表对象；不要并发读写容器，也不要跨数据修改保留内部样本引用。

图表将数据和样式更新合并到 polish 阶段，在 GUI 线程准备布局及图元，由渲染线程创建 SceneGraph 节点。software 后端通过图像绘制回退；具体数据与性能边界见 [图表](CHARTS.md)。

## Gallery 边界

GalleryChartData 保存演示源数据并生成采样／聚合预览；GalleryDataDemo 持有实际表格和消息模型，管理批次、容量及统计。两者都属于应用层，安装库不包含这些演示 API。

页面销毁、离开或隐藏时停止持续测试。运行统计反映数据操作；绘制发生在后续 Qt Quick 渲染阶段。示例容量不能直接推导库在其他应用或设备上的性能上限。

## 维护入口

新增组件同步维护模块 CMake、组件目录、GalleryCatalog、交互预览、翻译及相关检查。布局使用逻辑像素和主题 Token，输入优先采用 Qt Controls 的焦点、键盘、禁用与可访问性协议。详见 [贡献指南](../../../CONTRIBUTING.md) 和 [验证说明](VALIDATION.md)。
