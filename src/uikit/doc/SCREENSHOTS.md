# 文档截图与更新

[文档索引](README.md)

当前界面截图更新于 **2026-10-12**，由本仓库 Gallery 的实际 QML 页面和数据模型生成。环境为 Linux、Qt 6.12.0、Debug，使用 offscreen + software，关闭装饰动画以稳定捕获。品牌标识是设计资源，独立于界面截图。

| 图片 | 状态 |
| --- | --- |
| [中文首页](images/gallery-desktop.png) | 1200 × 900，亮色桌面 |
| [英文首页](images/gallery-desktop-en.png) | 1200 × 900，亮色桌面 |
| [触摸设置](images/gallery-touch.png) | 1200 × 1100，英文暗色触摸 |
| [窄屏对话](images/gallery-mobile.png) | 420 × 3000，中文暗色移动密度，10,000 条消息 |
| [桌面图表](images/gallery-charts-desktop.png) | 1280 × 1600，保留合计 1,000,000 条原始记录的折线概览 |
| [窄屏图表](images/gallery-charts-mobile.png) | 420 × 2400，同样保留 1,000,000 条记录，暗色移动密度 |
| [表格操作](images/gallery-table.png) | 1280 × 1500，100,000 行，显示编辑与压力参数 |
| [对话操作](images/gallery-chat.png) | 1280 × 1700，10,000 条消息，显示状态与压力参数 |
| [消息输入](images/gallery-chatinput.png) | 1280 × 1500，10,000 条提交历史，实际输入框连续提交 |

长页面使用较高的真实窗口展示操作区，没有拼接不同滚动位置。压力测试在达到容量后停止，截图保留真实统计；计时随机器而变，不用于性能排名。

## 重新生成整套截图

构建时开启 Gallery 和测试，然后从仓库根目录执行：

```sh
cmake --build build/release --target tst_gallery
python3 tools/capture_gallery_docs.py --build-dir build/release
```

多配置生成器给构建命令添加 `--config Release`，给截图工具添加 `--config Release`。如需先审阅输出，使用 `--output-dir /tmp/qcat-doc-images`，默认输出位置为 `src/uikit/doc/images`。

工具通过 Gallery 测试程序加载真实页面，设置主题和密度，填充实际模型后捕获。所有场景成功才更新输出文件；不会后期改写图片中的数据或统计。完整捕获通常需要几十秒，机器较慢时会更久。

输出后检查数字、坐标轴、控件位置、裁剪及窄屏可达性，再更新相关文案。单张普通窗口截图也可使用 [Gallery 的 --screenshot 参数](../../../examples/uikit/gallery/README.md)。
