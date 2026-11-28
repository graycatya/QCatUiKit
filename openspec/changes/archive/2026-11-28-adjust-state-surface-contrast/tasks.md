# Tasks

## 1. 默认值

- [x] 1.1 Light：`surfaceHover` `#e9f2ff`、`surfaceSelected` `#dbe8ff`、`surfacePressed` `#c6dcff`。
- [x] 1.2 Dark：`surfaceHover` `#333333`、`surfaceSelected` `#2b3a52`、`surfacePressed` `#3f5470`。

## 2. 测试

- [x] 2.1 更新 `tst_models.cpp` 的 Light/Dark 默认值断言。
- [x] 2.2 增加相对亮度计算，断言每个状态与 `surface` 的差 ≥ 0.015，并校验 Light 递减、Dark 递增。
- [x] 2.3 更新 `tst_gallery.qml` 中切回 Light 后的 `surfaceSelected` 断言。

## 3. 文档与验证

- [x] 3.1 `THEMING.md` 说明三级默认值满足可辨识下限，自定义色板需自行保证。
- [x] 3.2 构建并运行 `models`、`gallery_contracts`、`qml_contracts`。
- [x] 3.3 `python tools/check_docs.py`、`git diff --check`、`openspec validate adjust-state-surface-contrast --strict --no-interactive` 通过。
- [x] 3.4 在 `validation.md` 记录命令、结果、视觉检查与未覆盖平台。
