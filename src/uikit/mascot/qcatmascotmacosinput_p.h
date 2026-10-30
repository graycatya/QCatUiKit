// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QQuickWindow>
#include <QRegion>
#include <memory>

namespace QCatUiKit
{
// Private transparent AppKit input surfaces; never changes the visual mask.
class MascotMacosInput
{
  public:
    explicit MascotMacosInput(QQuickWindow *window);
    ~MascotMacosInput();
    bool sync(const QRegion &region, bool enabled);
    QString errorString() const;
    void requestKeyboardFocus();

  private:
    struct Private;
    std::shared_ptr<Private> d;
};
} // namespace QCatUiKit
