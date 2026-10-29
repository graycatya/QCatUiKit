// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QPointer>
#include <QQuickWindow>
#include <QRegion>
#include <qt_windows.h>

namespace QCatUiKit
{
// Private input-only owned window. Never paints or masks the Qt visual window.
class MascotWindowsInput
{
  public:
    explicit MascotWindowsInput(QQuickWindow *window);
    ~MascotWindowsInput();
    bool sync(const QRegion &region, bool enabled);
    QString errorString() const;

  private:
    static LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wp, LPARAM lp);
    QPointer<QQuickWindow> m_window;
    HWND m_surface = nullptr;
    HWND m_target = nullptr;
    DWORD m_error = 0;
    Qt::MouseButtons m_buttons;
};
} // namespace QCatUiKit
