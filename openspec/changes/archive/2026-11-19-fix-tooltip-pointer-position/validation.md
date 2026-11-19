# CatToolTip 鼠标定位验证

日期：2026-11-13。环境：Windows 11、Qt 6.12.0 MSVC Debug。

变更：默认提示跟随父项内的鼠标位置，右偏移 12、下偏移 16 逻辑像素；底部空间不足时翻到上方，保留 Popup 的窗口边缘约束。无鼠标悬停时回退到父项下方；应用仍可覆盖 x/y。

- 定向运行 `tst_qml -input tests/auto/qml/data/tst_tooltip.qml -import <build>/qml`：5 passed，0 failed。覆盖移动跟随、右侧／底部边缘、按钮输入和移出隐藏。
- `cmake --build <build> --target QCatUiKit_qmllint qcat-gallery_qmllint`：通过。
- `python tools/check_docs.py`：通过。
- `git diff --check`：通过。
- 完整 Debug 构建：通过。初次构建与测试并行导致 Windows 文件占用，结束该轮测试后重新构建成功。
- 最终 `ctest --test-dir <build> -R '^(qml_contracts|mascot_contracts|gallery_smoke)$' --output-on-failure -j 2`：QML 契约和 Gallery 启动通过。首轮全套测试发现的浮动窗口销毁时跟踪对象为空警告已修复，此轮 QML 契约验证通过；首轮 Gallery 长测试已主动终止以释放构建文件，因此不记为全套通过。
- 额外桌宠回归：34 passed、2 failed、13 skipped；`lottieRendering` 和 `largePackageKeepsEventLoopResponsive` 超过各自 30000ms 限制，实际分别需要约 89350ms 和 47050ms。未修改桌宠代码或超时标准；不宣称整套测试通过。

真实桌面明暗主题、窄窗口视觉检查以及其他平台尚未执行。自动测试使用 offscreen/software。

## 2026-11-14：普通桌面悬停交互

按用户要求将持续跟随改为停稳后延迟显示、显示时固定位置。显示前移动重置 delay；点击（包含等待期间）、Esc、移出和窗口失活关闭；点击／Esc／超时关闭后同次悬停不重开。输入观察采用被动 PointHandler，保留按钮与拖动输入。`hoverInteractionEnabled=false` 用于应用管理的拖动数值提示，库内 SliderHandleImpl 已设置，保留其显式定位与显隐。

验证环境仍为 Windows 11、Qt 6.12.0 MSVC Debug，offscreen/software：

- 完整 Debug 构建通过。
- 提示定向回归：10 passed，0 failed；滑块回归：16 passed，0 failed。首次整组测试暴露的拖动干扰已修复。
- 库／Gallery qmllint、文档检查及 diff 空白检查通过。
- 480×260 离屏明暗主题截图已检查，提示可读且右侧未越界；临时视觉探针 4 passed。未验证真实桌面窗口失活、多屏或其他平台。
- 最终 `ctest --test-dir <build> -R '^(qml_contracts|gallery_smoke)$' --output-on-failure -j 2`：2/2 通过，包含完整 QML 契约组和 Gallery 启动检查。本次未重跑桌宠回归。
