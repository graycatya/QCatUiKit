# 贡献约定

源码与公开头放在 `src/uikit`；C++ 文件使用小写名称，私有头以 `_p.h` 结尾。QML 文件保留类型名称。示例位于 `examples/uikit`，自动测试位于 `tests/auto`，模块文档位于 `src/uikit/doc`。新增公开头时同步维护模块 CMake 中的小写头、兼容头和无扩展名转发头列表。

提交实现前先明确组件的公开属性、状态、交互与兼容范围。新增能力更新 COMPONENTS，迁移变化更新 MIGRATION，ABI 相关改动更新 ABI。

本地检查：构建库与 gallery；ctest；QCatUiKit_qmllint 与 qcat-gallery_qmllint；安装后独立消费者；相关明暗／密度视觉验证。测试针对用户能观察到的行为，避免绑定内部对象树。ABI 变更保留旧消费者二进制执行替换检查。

新源文件使用 `SPDX-License-Identifier: Apache-2.0`。引入代码或资源保留其来源和许可；不添加 Qt 5、QrenCodeQml 或 lottie 依赖。
