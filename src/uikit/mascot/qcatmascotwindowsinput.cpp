// SPDX-License-Identifier: Apache-2.0
#include "qcatmascotwindowsinput_p.h"
#include <QCoreApplication>
#include <QList>
#include <QMouseEvent>
#include <QRectF>
#include <QWheelEvent>
#include <cstddef>
#include <windowsx.h>

namespace QCatUiKit
{
namespace
{
constexpr auto inputClass = L"QCatUiKitMascotInput";
}
MascotWindowsInput::MascotWindowsInput(QQuickWindow *window) : m_window(window) {}
MascotWindowsInput::~MascotWindowsInput()
{
    if (m_surface)
        DestroyWindow(m_surface);
}
QString MascotWindowsInput::errorString() const
{
    return QStringLiteral("Windows input surface request failed (Win32 error %1)").arg(m_error);
}
bool MascotWindowsInput::sync(const QRegion &region, bool enabled)
{
    if (!m_window || !m_window->isVisible() || !enabled || region.isEmpty())
    {
        if (m_surface)
        {
            if (GetCapture() == m_surface)
                ReleaseCapture();
            ShowWindow(m_surface, SW_HIDE);
        }
        return true;
    }
    const HWND target = reinterpret_cast<HWND>(m_window->winId());
    if (m_surface && target != m_target)
        DestroyWindow(m_surface);
    m_target = target;
    if (!m_surface)
    {
        WNDCLASSW wc{};
        wc.lpfnWndProc = windowProc;
        wc.style = CS_DBLCLKS;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = inputClass;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        if (!RegisterClassW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        {
            m_error = GetLastError();
            return false;
        }
        m_surface = CreateWindowExW(WS_EX_NOREDIRECTIONBITMAP | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
                                    inputClass, L"QCatUiKit mascot input", WS_POPUP, 0, 0, 1, 1,
                                    target, nullptr, wc.hInstance, this);
        if (!m_surface)
        {
            m_error = GetLastError();
            return false;
        }
    }
    RECT bounds{};
    if (!GetWindowRect(target, &bounds))
    {
        m_error = GetLastError();
        return false;
    }
    // Build one native region from declared logical geometry, not rendered alpha.
    QList<RECT> rectangles;
    RECT regionBounds{};
    const qreal dpr = m_window->devicePixelRatio();
    QRegion physicalRegion;
    for (const QRect &rect : region)
    {
        physicalRegion +=
            QRectF(rect.x() * dpr, rect.y() * dpr, rect.width() * dpr, rect.height() * dpr)
                .toAlignedRect();
    }
    physicalRegion &= QRegion(0, 0, bounds.right - bounds.left, bounds.bottom - bounds.top);
    if (physicalRegion.isEmpty())
    {
        if (GetCapture() == m_surface)
            ReleaseCapture();
        ShowWindow(m_surface, SW_HIDE);
        return true;
    }
    for (const QRect &pixels : physicalRegion)
    {
        const RECT native{pixels.left(), pixels.top(), pixels.right() + 1, pixels.bottom() + 1};
        if (rectangles.isEmpty())
            regionBounds = native;
        else
            UnionRect(&regionBounds, &regionBounds, &native);
        rectangles.append(native);
    }
    QByteArray data(int(offsetof(RGNDATA, Buffer) + rectangles.size() * sizeof(RECT)), '\0');
    auto *native = reinterpret_cast<RGNDATA *>(data.data());
    native->rdh = {sizeof(RGNDATAHEADER), RDH_RECTANGLES, DWORD(rectangles.size()),
                   DWORD(rectangles.size() * sizeof(RECT)), regionBounds};
    memcpy(native->Buffer, rectangles.constData(), size_t(rectangles.size()) * sizeof(RECT));
    HRGN mask = ExtCreateRegion(nullptr, DWORD(data.size()), native);
    if (!mask || !SetWindowRgn(m_surface, mask, FALSE))
    {
        m_error = GetLastError();
        if (mask)
            DeleteObject(mask);
        ShowWindow(m_surface, SW_HIDE);
        return false;
    }
    // Owned windows remain above their owner. Keep this surface immediately
    // above its own visual, never independently above another application's UI.
    const HWND preceding = GetWindow(target, GW_HWNDPREV);
    const UINT flags = SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_SHOWWINDOW |
                       (preceding == m_surface ? SWP_NOZORDER : 0);
    if (!SetWindowPos(m_surface, preceding ? preceding : HWND_TOP, bounds.left, bounds.top,
                      bounds.right - bounds.left, bounds.bottom - bounds.top, flags))
    {
        m_error = GetLastError();
        ShowWindow(m_surface, SW_HIDE);
        return false;
    }
    return true;
}
LRESULT CALLBACK MascotWindowsInput::windowProc(HWND window, UINT message, WPARAM wp, LPARAM lp)
{
    auto *self = reinterpret_cast<MascotWindowsInput *>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE)
    {
        self = static_cast<MascotWindowsInput *>(
            reinterpret_cast<CREATESTRUCTW *>(lp)->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    if (message == WM_NCDESTROY)
    {
        if (self)
            self->m_surface = nullptr;
        SetWindowLongPtrW(window, GWLP_USERDATA, 0);
    }
    if (message == WM_MOUSEACTIVATE)
        return MA_NOACTIVATE;
    if (message == WM_ERASEBKGND)
        return 1;
    if (message == WM_PAINT)
    {
        PAINTSTRUCT paint{};
        BeginPaint(window, &paint);
        EndPaint(window, &paint);
        return 0;
    }
    if (self && message == WM_CAPTURECHANGED && self->m_buttons)
    {
        self->m_buttons = {};
        if (self->m_window)
        {
            QEvent ungrab(QEvent::UngrabMouse);
            QCoreApplication::sendEvent(self->m_window, &ungrab);
        }
        return 0;
    }
    if (self && self->m_window && self->m_target &&
        ((message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) || message == WM_MOUSELEAVE))
    {
        if (message == WM_MOUSEMOVE)
        {
            TRACKMOUSEEVENT tracking{sizeof(TRACKMOUSEEVENT), TME_LEAVE, window, 0};
            TrackMouseEvent(&tracking);
        }
        // The visual is intentionally transparent to native mouse input, so
        // translate our own real native messages into its Qt input coordinates.
        // Never forward or synthesize events into another application's window.
        const auto receiver = self->m_window;
        POINT native{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
        const bool wheel = message == WM_MOUSEWHEEL || message == WM_MOUSEHWHEEL;
        if (wheel)
            ScreenToClient(window, &native);
        const QPointF local(native.x / receiver->devicePixelRatio(),
                            native.y / receiver->devicePixelRatio());
        const auto global = receiver->mapToGlobal(local);
        Qt::KeyboardModifiers modifiers;
        if (GetKeyState(VK_SHIFT) < 0)
            modifiers |= Qt::ShiftModifier;
        if (GetKeyState(VK_CONTROL) < 0)
            modifiers |= Qt::ControlModifier;
        if (GetKeyState(VK_MENU) < 0)
            modifiers |= Qt::AltModifier;
        if (message == WM_MOUSELEAVE)
        {
            QEvent leave(QEvent::Leave);
            QCoreApplication::sendEvent(receiver, &leave);
            return 0;
        }
        if (wheel)
        {
            const int delta = GET_WHEEL_DELTA_WPARAM(wp);
            QWheelEvent event(local, global, {},
                              message == WM_MOUSEWHEEL ? QPoint(0, delta) : QPoint(-delta, 0),
                              self->m_buttons, modifiers, Qt::NoScrollPhase, false);
            QCoreApplication::sendEvent(receiver, &event);
            return 0;
        }
        Qt::MouseButton button = Qt::NoButton;
        QEvent::Type type = QEvent::MouseMove;
        switch (message)
        {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
            button = Qt::LeftButton;
            break;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDBLCLK:
            button = Qt::RightButton;
            break;
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MBUTTONDBLCLK:
            button = Qt::MiddleButton;
            break;
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        case WM_XBUTTONDBLCLK:
            button = GET_XBUTTON_WPARAM(wp) == XBUTTON1 ? Qt::BackButton : Qt::ForwardButton;
            break;
        default:
            break;
        }
        if (message == WM_LBUTTONUP || message == WM_RBUTTONUP || message == WM_MBUTTONUP ||
            message == WM_XBUTTONUP)
        {
            type = QEvent::MouseButtonRelease;
            self->m_buttons &= ~Qt::MouseButtons(button);
        }
        else if (button != Qt::NoButton)
        {
            const bool doubleClick = message == WM_LBUTTONDBLCLK || message == WM_RBUTTONDBLCLK ||
                                     message == WM_MBUTTONDBLCLK || message == WM_XBUTTONDBLCLK;
            type = doubleClick ? QEvent::MouseButtonDblClick : QEvent::MouseButtonPress;
            self->m_buttons |= button;
            SetCapture(window);
        }
        const auto buttons = self->m_buttons;
        QMouseEvent event(type, local, global, button, buttons, modifiers);
        event.setTimestamp(ulong(GetMessageTime()));
        QCoreApplication::sendEvent(receiver, &event);
        if (type == QEvent::MouseButtonRelease && !buttons && GetCapture() == window)
            ReleaseCapture();
        return 0;
    }
    return DefWindowProcW(window, message, wp, lp);
}
} // namespace QCatUiKit
