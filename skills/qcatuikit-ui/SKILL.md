---
name: qcatuikit-ui
description: Build or refine Qt Quick/QML interfaces with QCatUiKit, including components, themes, models, 2D/3D charts, LED workflows, attitude/tilt input, and application integration. Use for QCatUiKit UI work, not unrelated web, Qt Widgets, or general C++ maintenance.
---

# QCatUiKit UI

Deliver working QML that uses QCatUiKit's public types and the application's existing behavior. Keep the user's visual direction, data flow, and chosen integration method.

## Find the API and integration

In a QCatUiKit checkout, use `src/uikit/doc/README.md` as the documentation map. This skill lives at `skills/qcatuikit-ui`; the checkout root is two directories above it. If the skill was copied elsewhere, locate the dependency checkout or installed QML type information from the application's CMake configuration instead of assuming that relative path.

Read only what the task needs:

- `src/uikit/doc/COMPONENTS.md`: components grouped by UI purpose.
- `src/uikit/doc/THEMING.md`: ThemePalette properties, tokens, and density.
- `src/uikit/doc/MODELS.md`: TableModel, ChatModel, TreeModel and their view contracts.
- `src/uikit/doc/CHARTS.md`, `CHARTS3D.md`, `CHARTSTATISTICS.md`, `CHARTEXTENSIONS.md`: the requested chart family, data ownership, limits, interaction and export.
- `src/uikit/doc/SLIDERS.md`: scalar/range editing and chart viewport synchronization.
- `src/uikit/doc/DOCKING.md`: panel registration, tabs/splits, workspace outer edges, floating windows and layout persistence.
- `src/uikit/doc/LEDWORKFLOW.md`, `LEDPREVIEW.md`: layout, Deck/Storage/Mixer, multi-channel output and GPU preview.
- `src/uikit/doc/ATTITUDE.md`: aircraft display and independent application-supplied tilt input.
- `src/uikit/doc/BUILDING.md`: CMake integration, QML import paths and runtime deployment.

Confirm unfamiliar properties in `src/uikit/qml/Cat*.qml`, public `src/uikit/<module>/qcat*.h`, or the installed `QCatUiKit.qmltypes`. Gallery snippets are examples; `GalleryChartData`, `GalleryDataDemo`, `GalleryIcon`, `GalleryLedDemo` and `QCatUiKit.Gallery` are not public application dependencies.

## Compose the interface

Use `import QCatUiKit`, `QtQuick`, and `QtQuick.Layouts` as needed. Prefer the matching Cat control for its themed visual and interaction behavior. Use normal Qt Quick layout and rendering primitives for custom content.

- Bind colors and dimensions to `Theme.tokens`, or explicitly pass a local `ThemePalette` through each application-owned themed child. Change palette properties, not the read-only tokens map.
- Select `ThemePalette.Desktop`, `Touch`, or `Mobile` explicitly. Window width does not select density. Do not force fixed control heights that clip text or shrink touch targets.
- Use `Layout.*` for layout-managed children; use anchors for children outside layouts. Do not make both systems own the same item's size or position.
- Give wrapping labels a constrained width. Switch multi-column forms to a single column when their actual contents no longer fit. Bound long lists/tables and let their views scroll instead of creating a delegate for every record.
- Wire actions to observable state or application signals. Show the relevant empty, invalid, pending, failed, or disabled state; do not display controls that merely look functional.
- Preserve Qt keyboard and focus behavior. Use `qsTr()` for user-facing text and accessible names for icon-only or unlabeled inputs. Use app-owned icons/assets; Gallery resources are not deployed with the library.

For task-specific composition, consult [UI patterns](references/ui-patterns.md). This reference distinguishes lookalike controls and covers forms, tables, conversation and slider edits. Read [chart composition](references/charts.md) only for chart work, or the relevant section of [LED and motion integration](references/led-and-motion.md) for those workflows. Keep library API details in the linked topic documents; dated validation records describe past runs, not current API limits.

## Start or extend an application

For an existing app, preserve its CMake target, navigation, localization and model ownership. For a new installed-package consumer, copy and adapt [assets/starter](assets/starter), which contains a complete CMake/C++/QML application with a responsive records form and table. Replace its sample content and actions with the requested product behavior.

The starter expects Qt 6.8+, CMake 3.24+, C++17 and an installed QCatUiKit package. Configure with both installation prefixes in CMAKE_PREFIX_PATH. Its `--qml-import-path` option supplies the runtime module root during development; packaged apps can use an adjacent qml directory. See [build and verification](references/build-and-verify.md) for concrete commands.

Do not add Qt Charts, Graphs, WebEngine, a Widgets hierarchy, or a new library API just to compose an ordinary page. If public controls cannot meet a real requirement, explain the gap and implement a scoped app-level composition or the requested library change.

## Verify the result

Build the changed application and run its QML lint target when available. Exercise the actual action and state transitions affected by the task. Inspect a rendered view at the target size and a relevant narrow layout, including the density or theme the user requested. Correct binding loops, layout conflicts, missing modules and clipped inputs before delivery.

Use existing meaningful tests for behavioral changes; a cosmetic adjustment does not need a new test that repeats the implementation. For large data, retain data in a model, yield between batches, and distinguish data-operation time from rendered FPS. Gallery's million-record, 100,000-row and 10,000-message presets are demo settings, not universal library limits.

Report which files to integrate, how to run the UI, what was verified and any actual unmet requirement. Do not claim a platform or hardware backend was tested from a software screenshot.
