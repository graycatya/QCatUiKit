# Tasks

## 1. 设置页色板

- [x] 1.1 solarized 色板补充 `surfaceSelected`、`surfacePressed`、`focusRing`，且状态色深浅顺序与 `surfaceHover` 一致。
- [x] 1.2 选择 `Custom palette` 时通过 `Theme.overrides` 设置非默认 `disabledOpacity`；切换 Light／Dark 时清空 `overrides`。

## 2. 状态展示

- [x] 2.1 设置页新增「交互状态」面板，显示五个状态色块及 token 名称。
- [x] 2.2 面板提供普通、选中、禁用按钮和复选、开关示例，用 `objectName` 暴露色块供测试定位。
- [x] 2.3 色块颜色绑定 `Theme.tokens`，在明暗切换和自定义色板下同步更新。

## 3. 测试

- [x] 3.1 `tst_gallery.qml::test_stateTokenDemo` 断言五个色块颜色等于对应 token。
- [x] 3.2 `test_customThemeScheme` 断言自定义状态色、`disabledOpacity`，以及切回内置色板后的默认值。
- [x] 3.3 测试清理阶段重置 `Theme.overrides`，避免状态泄漏到其他用例。

## 4. 文档与验证

- [x] 4.1 `THEMING.md` 的 `colorPalette` 示例补齐新 token，并说明 `disabledOpacity` 使用 `overrides`。
- [x] 4.2 运行 `qcat-gallery_lupdate` 并补齐 `zh_CN` 译文。
- [x] 4.3 构建 Gallery，运行 `tst_gallery` 与 `qml_contracts`。
- [x] 4.4 `qmllint` 修改文件、`python tools/check_docs.py`、`git diff --check`、`openspec validate add-gallery-state-tokens-demo --strict --no-interactive` 通过。
- [x] 4.5 在 `validation.md` 记录命令、结果、视觉检查与未覆盖平台。
