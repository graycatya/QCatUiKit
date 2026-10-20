# QCatUiKit agent guidance

This repository provides a Qt 6 QML component library and its Gallery.

For requests to build or refine a UI with QCatUiKit, read
[skills/qcatuikit-ui/SKILL.md](skills/qcatuikit-ui/SKILL.md). It covers public
components, themes, models, 2D/3D charts, LED and motion workflows,
integration, and a runnable starter.
Apply it to relevant UI work, not unrelated maintenance tasks.

Use [the documentation index](src/uikit/doc/README.md) for current APIs and
[CONTRIBUTING.md](CONTRIBUTING.md) for repository conventions. Preserve existing
application architecture and user changes; Gallery-only helpers are not library APIs.

For documentation changes, run `python tools/check_docs.py`. Keep repeatable
validation instructions in `src/uikit/doc/VALIDATION.md` and dated execution
evidence in `src/uikit/doc/VALIDATION_HISTORY.md`.


For maintenance and new components, use [the maintenance map](src/uikit/doc/MAINTENANCE.md)
to select relevant files and checks. Limit searches to source file types and scoped
directories; exclude assets and build output from broad text reads.
