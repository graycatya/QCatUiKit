# 本地验证记录

日期：2026-10-08。环境：Linux x86_64、GCC 15.2、Qt 6.12.0、CMake + Ninja。QML 测试与截图使用 offscreen 平台和 Qt Quick software 后端。

| 检查 | 结果 |
| --- | --- |
| Debug 完整构建：共享库、QML 插件、gallery、测试 | 通过 |
| Release 最小构建：BUILD_TESTING=OFF、QCATUIKIT_BUILD_EXAMPLES=OFF | 通过 |
| CTest：models、qml_contracts、gallery_smoke | 3/3 通过 |
| C++ 行为测试 | 5 个测试函数通过，覆盖模型协议、越界、通知、主题及缓存失效 |
| Qt Quick Test | 38 项通过（包含初始化与清理）；其中 27 项为所有公开 QML 组件的导入与创建 |
| QML 行为 | 鼠标、键盘、模拟触摸、加载状态、enabled 绑定恢复、IPv4、分页、日期、Escape、局部主题与析构 |
| QCatUiKit_qmllint / qcat-gallery_qmllint | 无警告；.qmllint.ini 将 MaxWarnings 设为 0 |
| Debug 安装包由 Qt 自带 qmltestrunner 加载 | 38 项通过，无源码树或 build 导入路径 |
| Release 安装包由 Qt 自带 qmltestrunner 加载 | 38 项通过 |
| find_package 独立 C++ 消费者 | 从安装包成功配置、编译、链接及执行 |
| 不重编译消费者的共享库替换 | 通过，实际加载候选安装目录中的 libQCatUiKit.so.1 |
| QCatUiKit 导出符号 | 基线 91 个，候选 91 个，删除 0 个 |
| 视觉检查 | 桌面亮色、触摸暗色、390px 移动布局已渲染并查看 |

## 初次迁移的 ABI 实验

先安装不含主题缓存的基线，用其公开头和库编译 `tests/consumer`。随后只在 ThemePalette 的 Impl 增加 QVariantMap 缓存与失效逻辑，重新构建安装候选库。原消费者在候选库优先的 LD_LIBRARY_PATH 下正常运行，dladdr 输出确认加载的是候选库。公开对象大小在客户端为 24 / 24 / 24 字节；导出符号集合没有删除。此结果是对当前消费者与此次私有实现变化的验证，不替代未来发布时的完整 ABI 审计。

复现入口：`tests/check_abi_replacement.sh`；平台边界见 [ABI](ABI.md)。

## 截图

- [桌面亮色](images/gallery-desktop.png)
- [触摸暗色](images/gallery-touch.png)
- [移动宽度](images/gallery-mobile.png)

截图命令：

```sh
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software \
  ./build/examples/gallery/qcat-gallery --screenshot /tmp/gallery.png
# 可附加 --dark --density touch，或 --density mobile --width 390
```

## 尚未执行的环境验证

没有在 Windows、macOS、Android、iOS 或实体嵌入式设备上构建运行，也没有在 Qt 6.8 上运行本轮测试。Qt 6.8 是项目声明的最低版本，本次实际执行版本为 6.12.0。触摸使用 Qt Quick Test 合成事件；实际输入法、屏幕阅读器、GPU 上的 MultiEffect、设备 DPI、功耗与帧率需要目标设备验收。软件后端截图不能证明 GPU 特效效果。

迁移关系图用于旧模型导航，有 4 条悬空端点和 3 组同端点关系合并；关键结论已核对源码。详细说明见 [架构文档](ARCHITECTURE.md)。

## 版本接口新增验证

新增 C++ Version.h 与 QML LibraryInfo 后，分别构建 experimental（Debug）和 stable（Release），结果：

- 实验构建 CTest 4/4 通过，正式构建 3/3 通过（未构建 gallery）。
- version 测试将 C++ 函数和两个 QML 引擎中的只读单例与 CMake 预期配置对比，均通过。
- 两种安装包分别由 Qt qmltestrunner 加载，各 39 项通过。
- qmllint 无警告；非法渠道 preview 在 CMake 配置阶段被拒绝。
- 独立消费者仅用实验安装包编译一次。在固定 Qt 6.12 运行时下，先加载实验库得到 `1.0.0-experimental experimental ABI 1`，再切换到正式库得到 `1.0.0 stable ABI 1`，消费者未重新编译。
- 安装包的 VERSION_STRING、RELEASE_CHANNEL 和 ABI_VERSION 元数据正确。
- 旧 ABI 消费者仍可运行；QCatUiKit 导出符号从 91 增至 97，无删除。

开发机同时存在系统 Qt 6.10 与独立 Qt 6.12；独立消费者初次未指定运行时路径时出现 Qt 间接依赖混用，使用与构建一致的 Qt lib 路径后通过。版本管理文档的复现命令已包含该路径。
