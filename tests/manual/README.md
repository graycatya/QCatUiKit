# 手动验证程序

与自动化断言分开维护，用于人工观察原生窗口、角色素材与平台能力。默认不构建，开启测试后可单独启用：

```sh
cmake -S . -B <build-dir> -DBUILD_TESTING=ON -DQCATUIKIT_BUILD_MANUAL_TESTS=ON
cmake --build <build-dir> --target qcat-mascot-probe qcat-mascot-player
```

- `mascot-probe`：环境及能力探测；`--self-test` 是无窗口的分类回归检查。
- [mascot-player](mascot-player/README.md)：CatGray 精灵与完整 Lottie 角色包的交互预览。

两者也可从各自目录独立配置。播放器使用已安装的 QCatUiKit；探测器仅需要 Qt Quick。自动化角色断言位于 `tests/auto/mascot`，不依赖启用手动程序构建。平台矩阵验收仍需单独执行并记录证据。
