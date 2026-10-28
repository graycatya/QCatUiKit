// SPDX-License-Identifier: Apache-2.0
#include "windowsprobe.h"
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMenu>
#include <QPainter>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSaveFile>
#include <QScreen>
#include <QSysInfo>
#include <QUuid>
#include <cstdio>
#include <windows.h>

namespace
{
HWND hwnd(quintptr value)
{
    return reinterpret_cast<HWND>(value);
}
quintptr id(HWND value)
{
    return reinterpret_cast<quintptr>(value);
}
QString handle(quintptr value)
{
    return QString::number(value, 16);
}
int counter(HWND w, const wchar_t *name)
{
    return int(reinterpret_cast<quintptr>(GetPropW(w, name)));
}
void increment(HWND w, const wchar_t *name)
{
    SetPropW(w, name, reinterpret_cast<HANDLE>(quintptr(counter(w, name) + 1)));
    InvalidateRect(w, nullptr, TRUE);
}
LRESULT CALLBACK targetProc(HWND w, UINT message, WPARAM wp, LPARAM lp)
{
    switch (message)
    {
    case WM_LBUTTONDOWN:
        increment(w, L"QCatProbeClicks");
        SetFocus(w);
        return 0;
    case WM_SETFOCUS:
        increment(w, L"QCatProbeFocusIn");
        return 0;
    case WM_KILLFOCUS:
        increment(w, L"QCatProbeFocusOut");
        return 0;
    case WM_KEYDOWN:
        increment(w, L"QCatProbeKeys");
        return 0;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(w, &ps);
        RECT r;
        GetClientRect(w, &r);
        HBRUSH brush = CreateSolidBrush(RGB(29, 71, 99));
        FillRect(dc, &r, brush);
        DeleteObject(brush);
        SetTextColor(dc, RGB(255, 255, 255));
        SetBkMode(dc, TRANSPARENT);
        const QString text =
            QStringLiteral("Independent native target process %1\nClicks: %2   Keys: "
                           "%3\nTransparent areas must deliver real input here.")
                .arg(GetCurrentProcessId())
                .arg(counter(w, L"QCatProbeClicks"))
                .arg(counter(w, L"QCatProbeKeys"));
        r.left += 16;
        r.top += 16;
        DrawTextW(dc, reinterpret_cast<LPCWSTR>(text.utf16()), -1, &r, DT_LEFT | DT_TOP);
        EndPaint(w, &ps);
        return 0;
    }
    case WM_CLOSE:
        DestroyWindow(w);
        return 0;
    case WM_DESTROY:
        QCoreApplication::quit();
        return 0;
    default:
        return DefWindowProcW(w, message, wp, lp);
    }
}
bool screenshot(HWND target, const QString &path)
{
    RECT r;
    if (!GetClientRect(target, &r))
        return false;
    POINT origin{0, 0};
    if (!ClientToScreen(target, &origin))
        return false;
    const int width = r.right, height = r.bottom;
    HDC dc = GetDC(nullptr), memory = CreateCompatibleDC(dc);
    HBITMAP bitmap = CreateCompatibleBitmap(dc, width, height);
    HGDIOBJ previous = SelectObject(memory, bitmap);
    const bool copied =
        BitBlt(memory, 0, 0, width, height, dc, origin.x, origin.y, SRCCOPY | CAPTUREBLT);
    SelectObject(memory, previous);
    QImage image(width, height, QImage::Format_RGB32);
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = -height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    const bool read = GetDIBits(memory, bitmap, 0, height, image.bits(), &info, DIB_RGB_COLORS);
    DeleteObject(bitmap);
    DeleteDC(memory);
    ReleaseDC(nullptr, dc);
    return copied && read && image.save(path);
}
} // namespace

int WindowsProbe::runTarget(const QString &token)
{
    WNDCLASSW wc{};
    wc.lpfnWndProc = targetProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"QCatMascotIndependentTarget";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    if (!RegisterClassW(&wc))
        return 2;
    RECT work;
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);
    const qreal dpr = QGuiApplication::primaryScreen()->devicePixelRatio();
    HWND w = CreateWindowExW(0, wc.lpszClassName, reinterpret_cast<LPCWSTR>(token.utf16()),
                             WS_OVERLAPPEDWINDOW, work.left + qRound(60 * dpr),
                             work.top + qRound(60 * dpr), qRound(800 * dpr), qRound(620 * dpr),
                             nullptr, nullptr, wc.hInstance, nullptr);
    if (!w)
        return 3;
    ShowWindow(w, SW_SHOW);
    UpdateWindow(w);
    return QCoreApplication::exec();
}

WindowsProbe::WindowsProbe(QQmlEngine *engine, QJsonObject environment)
    : m_engine(engine), m_report(std::move(environment)), m_token(QUuid::createUuid().toString())
{
    m_previousForeground = foreground();
    POINT cursor;
    GetCursorPos(&cursor);
    m_previousCursor = {cursor.x, cursor.y};
    m_report.insert("prototype", "Windows native window prototype");
    m_report.insert("kernelVersion", QSysInfo::kernelVersion());
    m_report.insert("buildAbi", QSysInfo::buildAbi());
#ifdef _MSC_FULL_VER
    m_report.insert("msvcFullVersion", int(_MSC_FULL_VER));
#endif
    m_report.insert("prototypeAcceptance", "Pending");
    m_report.insert("checks", QJsonArray());
    m_report.insert(
        "evidenceMethod",
        "SendInput to own overlay and separate native target process; no QtTest synthetic clicks");
    QCoreApplication::instance()->installNativeEventFilter(this);
    m_menu = std::make_unique<QMenu>();
    m_menu->addAction(tr("Show / hide pet"), this,
                      [this]
                      {
                          ++m_trayActions;
                          setPetVisible(!m_pet->isVisible());
                      });
    m_menu->addAction(tr("Toggle input passthrough"), this,
                      [this]
                      {
                          ++m_trayActions;
                          setPassthrough(!m_passthrough);
                      });
    m_menu->addAction(tr("Exit requested (host stays alive)"), this,
                      [this]
                      {
                          ++m_trayActions;
                          ++m_exitRequests;
                          emit changed();
                      });
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setBrush(QColor("#e6a64b"));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(2, 2, 28, 28);
    painter.end();
    m_tray.setIcon(QIcon(pixmap));
    m_tray.setToolTip(tr("QCat Windows mascot prototype"));
    // Open our own QMenu so an explicit tray interaction can own keyboard focus.
    connect(&m_tray, &QSystemTrayIcon::activated, this,
            [this](QSystemTrayIcon::ActivationReason reason)
            {
                auto events = m_report.value("trayActivationReasons").toArray();
                events.append(int(reason));
                m_report.insert("trayActivationReasons", events);
                if (reason == QSystemTrayIcon::Context)
                {
                    m_menu->popup(QCursor::pos());
                    m_menu->activateWindow();
                    QApplication::setActiveWindow(m_menu.get());
                    m_menu->setFocus(Qt::PopupFocusReason);
                    m_menu->setActiveAction(m_menu->actions().first());
                    SetForegroundWindow(reinterpret_cast<HWND>(m_menu->winId()));
                    SetFocus(reinterpret_cast<HWND>(m_menu->winId()));
                }
                if (reason == QSystemTrayIcon::DoubleClick)
                {
                    ++m_trayActions;
                    setPetVisible(!m_pet->isVisible());
                }
            });
    m_runner.setInterval(250);
    m_observer.setInterval(500);
    connect(&m_observer, &QTimer::timeout, this,
            [this]
            {
                if (!m_runner.isActive() && !m_behind)
                    m_behind = id(FindWindowW(L"QCatMascotIndependentTarget",
                                              reinterpret_cast<LPCWSTR>(m_token.utf16())));
                emit changed();
            });
    connect(&m_runner, &QTimer::timeout, this,
            [this]
            {
                if (!m_behind)
                {
                    m_behind = id(FindWindowW(L"QCatMascotIndependentTarget",
                                              reinterpret_cast<LPCWSTR>(m_token.utf16())));
                    if (!m_behind && ++m_ticks < 40)
                        return;
                    if (!m_behind)
                    {
                        record("independentTarget", false);
                        finishChecks();
                        return;
                    }
                    DWORD pid = 0;
                    GetWindowThreadProcessId(hwnd(m_behind), &pid);
                    record("independentTarget",
                           pid == DWORD(m_target.processId()) && pid != GetCurrentProcessId(),
                           {{"pid", int(pid)}});
                    SetWindowPos(hwnd(m_behind), HWND_TOP, 0, 0, 0, 0,
                                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                    resetPosition();
                }
                if (m_step < int(m_steps.size()))
                    m_steps[m_step++]();
                else
                    finishChecks();
            });
}
WindowsProbe::~WindowsProbe()
{
    m_runner.stop();
    m_observer.stop();
    m_tray.hide();
    m_menu->close();
    QCoreApplication::instance()->removeNativeEventFilter(this);
    delete m_pet;
    if (m_target.state() != QProcess::NotRunning)
    {
        if (m_behind)
            PostMessageW(hwnd(m_behind), WM_CLOSE, 0, 0);
        if (!m_target.waitForFinished(2000))
        {
            m_target.kill();
            m_target.waitForFinished(1000);
        }
    }
}
QString WindowsProbe::report() const
{
    auto report = m_report;
    report.insert("bodyClicks", m_bodyClicks);
    report.insert("bubbleClicks", m_bubbleClicks);
    report.insert("trayActions", m_trayActions);
    report.insert("exitRequests", m_exitRequests);
    report.insert("foregroundHwnd", handle(foreground()));
    report.insert("targetClicks", targetCounter(L"QCatProbeClicks"));
    report.insert("targetKeys", targetCounter(L"QCatProbeKeys"));
    report.insert("trayMenuVisible", m_menu->isVisible());
    if (m_pet)
    {
        report.insert("graphicsApi", int(m_pet->rendererInterface()->graphicsApi()));
        if (m_pet->rendererInterface()->graphicsApi() == QSGRendererInterface::Direct3D11)
            report.insert("graphicsApiName", "Direct3D11");
    }
    return QString::fromUtf8(QJsonDocument(report).toJson());
}
void WindowsProbe::start()
{
    if (m_pet)
        return;
    QQmlComponent component(m_engine, QUrl("qrc:/WindowsOverlay.qml"));
    m_pet = qobject_cast<QQuickWindow *>(
        component.createWithInitialProperties({{"probe", QVariant::fromValue(this)}}));
    if (!m_pet)
    {
        record("overlayCreation", false, {{"error", component.errorString()}});
        return;
    }
    m_pet->setFlag(Qt::WindowDoesNotAcceptFocus, true);
    m_native = quintptr(m_pet->winId());
    applyMask();
    resetPosition();
    m_target.start(QCoreApplication::applicationFilePath(), {"--windows-target", m_token});
    m_tray.show();
    setPetVisible(true);
    m_report.insert("trayServiceAvailable", QSystemTrayIcon::isSystemTrayAvailable());
    m_observer.start();
    emit changed();
}
void WindowsProbe::applyMask()
{
    if (!m_pet)
        return;
    QRegion region(QRect(120, 110, 180, 170));
    if (m_bubble)
        region += QRect(60, 20, 300, 60);
    m_pet->setMask(region);
}
void WindowsProbe::setPassthrough(bool enabled)
{
    if (!m_pet)
        return;
    m_passthrough = enabled;
    m_pet->setFlag(Qt::WindowTransparentForInput, enabled);
    m_native = quintptr(m_pet->winId());
    emit changed();
}
void WindowsProbe::setTopmost(bool enabled)
{
    if (!m_pet)
        return;
    m_topmost = enabled;
    SetWindowPos(reinterpret_cast<HWND>(m_pet->winId()), enabled ? HWND_TOPMOST : HWND_NOTOPMOST, 0,
                 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    emit changed();
}
void WindowsProbe::setBubble(bool visible)
{
    m_bubble = visible;
    if (m_pet)
    {
        m_pet->setProperty("bubbleVisible", visible);
        applyMask();
    }
    emit changed();
}
void WindowsProbe::setPetVisible(bool visible)
{
    if (!m_pet)
        return;
    if (visible)
    {
        m_pet->setFlag(Qt::WindowDoesNotAcceptFocus, true);
        m_pet->show();
        m_native = quintptr(m_pet->winId());
        ShowWindow(reinterpret_cast<HWND>(m_pet->winId()), SW_SHOWNOACTIVATE);
        setTopmost(m_topmost);
    }
    else
        m_pet->hide();
    emit changed();
}
void WindowsProbe::resetPosition()
{
    if (m_pet)
        m_pet->setPosition(QGuiApplication::primaryScreen()->availableGeometry().topLeft() +
                           QPoint(160, 150));
}
void WindowsProbe::bodyPressed()
{
    ++m_bodyClicks;
    m_dragging = true;
    m_dragCursor = QCursor::pos();
    m_dragPosition = m_pet->position();
    emit changed();
}
void WindowsProbe::bodyMoved()
{
    if (m_dragging && m_pet)
        m_pet->setPosition(m_dragPosition + QCursor::pos() - m_dragCursor);
}
void WindowsProbe::bodyReleased()
{
    m_dragging = false;
}
void WindowsProbe::focusBubble()
{
    if (!m_pet)
        return;
    m_pet->setFlag(Qt::WindowDoesNotAcceptFocus, false);
    m_native = quintptr(m_pet->winId());
    m_pet->requestActivate();
}
void WindowsProbe::bubbleActivated()
{
    ++m_bubbleClicks;
    emit changed();
}
bool WindowsProbe::nativeEventFilter(const QByteArray &, void *message, qintptr *result)
{
    const auto *msg = static_cast<MSG *>(message);
    if (m_native && msg->hwnd == hwnd(m_native) && msg->message == WM_MOUSEACTIVATE &&
        (GetWindowLongPtrW(msg->hwnd, GWL_EXSTYLE) & WS_EX_NOACTIVATE))
    {
        *result = MA_NOACTIVATE;
        return true;
    }
    return false;
}
QPoint WindowsProbe::physicalPoint(const QPoint &local) const
{
    POINT point{0, 0};
    ClientToScreen(reinterpret_cast<HWND>(m_pet->winId()), &point);
    const auto dpr = m_pet->devicePixelRatio();
    return {point.x + qRound(local.x() * dpr), point.y + qRound(local.y() * dpr)};
}
int WindowsProbe::targetCounter(const wchar_t *name) const
{
    return m_behind ? counter(hwnd(m_behind), name) : 0;
}
quintptr WindowsProbe::foreground() const
{
    return id(GetForegroundWindow());
}
bool WindowsProbe::mouse(const QPoint &point, unsigned long flags, bool shellIcon)
{
    POINT p{point.x(), point.y()};
    DWORD pid = 0;
    GetWindowThreadProcessId(WindowFromPoint(p), &pid);
    if (!shellIcon && pid != GetCurrentProcessId() && pid != DWORD(m_target.processId()))
    {
        record("inputGuard", false, {{"reason", "Point is not over a test-owned window"}});
        m_runner.stop();
        QTimer::singleShot(0, this, &WindowsProbe::finishChecks);
        return false;
    }
    INPUT input{};
    input.type = INPUT_MOUSE;
    input.mi.dx = MulDiv(point.x() - GetSystemMetrics(SM_XVIRTUALSCREEN), 65535,
                         GetSystemMetrics(SM_CXVIRTUALSCREEN) - 1);
    input.mi.dy = MulDiv(point.y() - GetSystemMetrics(SM_YVIRTUALSCREEN), 65535,
                         GetSystemMetrics(SM_CYVIRTUALSCREEN) - 1);
    input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_MOVE | flags;
    const bool ok = SendInput(1, &input, sizeof(input)) == 1;
    if (ok)
    {
        if (flags & MOUSEEVENTF_LEFTDOWN)
            m_leftDown = true;
        if (flags & MOUSEEVENTF_LEFTUP)
            m_leftDown = false;
        if (flags & MOUSEEVENTF_RIGHTDOWN)
            m_rightDown = true;
        if (flags & MOUSEEVENTF_RIGHTUP)
            m_rightDown = false;
    }
    if (!ok)
    {
        m_inputAvailable = false;
        record("SendInput", false, {{"win32Error", int(GetLastError())}});
        m_runner.stop();
        QTimer::singleShot(0, this, &WindowsProbe::finishChecks);
    }
    return ok;
}
bool WindowsProbe::key(unsigned short code)
{
    DWORD pid = 0;
    GetWindowThreadProcessId(GetForegroundWindow(), &pid);
    if (pid != GetCurrentProcessId() && pid != DWORD(m_target.processId()))
    {
        record("keyboardGuard", false, {{"reason", "Foreground is not test-owned"}});
        m_runner.stop();
        QTimer::singleShot(0, this, &WindowsProbe::finishChecks);
        return false;
    }
    INPUT inputs[2]{};
    inputs[0].type = inputs[1].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = inputs[1].ki.wVk = code;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
    return SendInput(2, inputs, sizeof(INPUT)) == 2;
}
void WindowsProbe::record(const QString &name, bool passed, const QJsonObject &detail)
{
    auto checks = m_report.value("checks").toArray();
    checks.append(QJsonObject{{"name", name},
                              {"result", passed ? "Passed" : "Failed"},
                              {"detail", detail},
                              {"foregroundHwnd", handle(foreground())},
                              {"targetHwnd", handle(m_behind)},
                              {"targetClicks", targetCounter(L"QCatProbeClicks")},
                              {"targetKeys", targetCounter(L"QCatProbeKeys")},
                              {"bodyClicks", m_bodyClicks},
                              {"bubbleClicks", m_bubbleClicks}});
    m_report.insert("checks", checks);
    m_allPassed &= passed;
    emit changed();
}
void WindowsProbe::runChecks(const QString &path)
{
    m_output = path;
    start();
    if (!m_pet)
    {
        finishChecks();
        return;
    }
    const auto click = [this](QPoint local)
    {
        const auto p = physicalPoint(local);
        mouse(p, MOUSEEVENTF_LEFTDOWN);
        mouse(p, MOUSEEVENTF_LEFTUP);
    };
    m_steps = {
        [this, click]
        {
            setPetVisible(false);
            click(QPoint(-30, 200));
        },
        [this]
        {
            m_savedFocus = foreground();
            m_targetKeys = targetCounter(L"QCatProbeKeys");
            setPetVisible(true);
        },
        [this]
        {
            record("showDoesNotActivate", m_savedFocus == m_behind && foreground() == m_behind,
                   {{"before", handle(m_savedFocus)}, {"after", handle(foreground())}});
            m_report.insert("graphicsApi", int(m_pet->rendererInterface()->graphicsApi()));
            m_report.insert("windowDpr", m_pet->devicePixelRatio());
            m_targetClicks = targetCounter(L"QCatProbeClicks");
        },
        [click] { click({20, 200}); },
        [this]
        {
            record("blankCrossProcessClick",
                   targetCounter(L"QCatProbeClicks") == m_targetClicks + 1 && m_bodyClicks == 0);
        },
        [click] { click({210, 190}); },
        [this]
        {
            record("bodyReceivesClickWithoutFocus",
                   m_bodyClicks == 1 && foreground() == m_behind &&
                       targetCounter(L"QCatProbeClicks") == m_targetClicks + 1);
            key('A');
        },
        [this]
        {
            record("typingStaysInBehindProcess",
                   targetCounter(L"QCatProbeKeys") == m_targetKeys + 1 && foreground() == m_behind);
            const auto p = physicalPoint({210, 190});
            HDC dc = GetDC(nullptr);
            const auto body = GetPixel(dc, p.x(), p.y());
            const auto blankPoint = physicalPoint({20, 200});
            const auto blank = GetPixel(dc, blankPoint.x(), blankPoint.y());
            const auto referencePoint = physicalPoint({-30, 200});
            const auto reference = GetPixel(dc, referencePoint.x(), referencePoint.y());
            ReleaseDC(nullptr, dc);
            record("transparentComposite",
                   blank == reference && body != blank && blank != CLR_INVALID,
                   {{"referenceRgb", int(reference)},
                    {"blankRgb", int(blank)},
                    {"bodyRgb", int(body)}});
            setPassthrough(true);
            m_targetClicks = targetCounter(L"QCatProbeClicks");
        },
        [click] { click({210, 190}); },
        [this, click]
        {
            record("wholeWindowBodyPassthrough",
                   targetCounter(L"QCatProbeClicks") == m_targetClicks + 1 && m_bodyClicks == 1);
            click({210, 50});
        },
        [this]
        {
            record("wholeWindowBubblePassthrough",
                   targetCounter(L"QCatProbeClicks") == m_targetClicks + 2 && m_bubbleClicks == 0);
            setPassthrough(false);
            setBubble(false);
            m_targetClicks = targetCounter(L"QCatProbeClicks");
        },
        [click] { click({210, 50}); },
        [this]
        {
            record("dynamicBubbleInputRegion",
                   targetCounter(L"QCatProbeClicks") == m_targetClicks + 1);
            setBubble(true);
        },
        [this]
        {
            m_savedPosition = m_pet->position();
            mouse(physicalPoint({210, 190}), MOUSEEVENTF_LEFTDOWN);
        },
        [this] { mouse(physicalPoint({240, 210}), 0); },
        [this]
        {
            POINT p;
            GetCursorPos(&p);
            mouse({p.x, p.y}, MOUSEEVENTF_LEFTUP);
        },
        [this]
        {
            RECT r;
            GetWindowRect(reinterpret_cast<HWND>(m_pet->winId()), &r);
            record("nativeDrag",
                   (m_pet->position() - m_savedPosition - QPoint(30, 20)).manhattanLength() <= 3,
                   {{"logicalDeltaX", m_pet->x() - m_savedPosition.x()},
                    {"logicalDeltaY", m_pet->y() - m_savedPosition.y()},
                    {"physicalX", int(r.left)},
                    {"physicalY", int(r.top)}});
            resetPosition();
        },
        [this]
        {
            const auto expected =
                QGuiApplication::primaryScreen()->availableGeometry().topLeft() + QPoint(160, 150);
            RECT physical{};
            const bool read = GetWindowRect(reinterpret_cast<HWND>(m_pet->winId()), &physical);
            const qreal dpr = m_pet->devicePixelRatio();
            record("externalPosition",
                   read && (m_pet->position() - expected).manhattanLength() <= 2 &&
                       qAbs(physical.left - qRound(expected.x() * dpr)) <= 2 &&
                       qAbs(physical.top - qRound(expected.y() * dpr)) <= 2,
                   {{"physicalX", int(physical.left)}, {"physicalY", int(physical.top)}});
            setTopmost(false);
        },
        [this]
        {
            record("topmostOff",
                   !(GetWindowLongPtrW(reinterpret_cast<HWND>(m_pet->winId()), GWL_EXSTYLE) &
                     WS_EX_TOPMOST));
            setTopmost(true);
        },
        [this, click]
        {
            record("topmostOn",
                   bool(GetWindowLongPtrW(reinterpret_cast<HWND>(m_pet->winId()), GWL_EXSTYLE) &
                        WS_EX_TOPMOST));
            click({140, 50});
        },
        [this]
        {
            record("explicitBubbleFocus",
                   foreground() == id(reinterpret_cast<HWND>(m_pet->winId())) &&
                       m_bubbleClicks == 1);
            key(VK_TAB);
        },
        [this] { key(VK_SPACE); },
        [this]
        {
            record(
                "bubbleKeyboardActivation", m_bubbleClicks >= 2,
                {{"activeFocusItem",
                  m_pet->activeFocusItem() ? m_pet->activeFocusItem()->objectName() : QString()}});
            const QString shot =
                QFileInfo(m_output).absolutePath() + "/windows-mascot-prototype.png";
            record("screenshot", screenshot(hwnd(m_behind), shot), {{"path", shot}});
            setPetVisible(false);
        },
        [this]
        {
            record("hostRestoresHiddenPet", !m_pet->isVisible());
            setPetVisible(true);
        },
        [this]
        {
            record("hostRestoreVisible", m_pet->isVisible());
            const QRect geometry = m_tray.geometry();
            m_report.insert("trayGeometry", QJsonArray{geometry.x(), geometry.y(), geometry.width(),
                                                       geometry.height()});
            record("trayRegistered", QSystemTrayIcon::isSystemTrayAvailable() &&
                                         m_tray.isVisible() && !geometry.isEmpty());
            if (geometry.isEmpty())
            {
                finishChecks();
                return;
            }
            if (!geometry.isEmpty())
            {
                const auto trayShot = QGuiApplication::primaryScreen()->grabWindow(
                    0, geometry.x(), geometry.y(), geometry.width(), geometry.height());
                trayShot.save(QFileInfo(m_output).absolutePath() + "/windows-mascot-tray.png");
                const qreal dpr = QGuiApplication::primaryScreen()->devicePixelRatio();
                m_trayPoint = geometry.center() * dpr;
                mouse(m_trayPoint, 0, true);
            }
        },
        // Qt may return the overflow chevron for an icon hidden by the shell.
        // A single left click expands it (a visible icon's Trigger has no action here).
        [this] { mouse(m_trayPoint, MOUSEEVENTF_LEFTDOWN, true); },
        [this] { mouse(m_trayPoint, MOUSEEVENTF_LEFTUP, true); },
        [this]
        {
            const QRect geometry = m_tray.geometry();
            if (geometry.isEmpty())
            {
                record("expandedTrayIcon", false);
                finishChecks();
                return;
            }
            m_report.insert(
                "expandedTrayGeometry",
                QJsonArray{geometry.x(), geometry.y(), geometry.width(), geometry.height()});
            const auto shot = QGuiApplication::primaryScreen()->grabWindow(
                0, geometry.x(), geometry.y(), geometry.width(), geometry.height());
            shot.save(QFileInfo(m_output).absolutePath() + "/windows-mascot-tray-expanded.png");
            m_trayPoint = geometry.center() * QGuiApplication::primaryScreen()->devicePixelRatio();
            mouse(m_trayPoint, 0, true);
        },
        [this] { mouse(m_trayPoint, MOUSEEVENTF_RIGHTDOWN, true); },
        [this] { mouse(m_trayPoint, MOUSEEVENTF_RIGHTUP, true); },
        [this]
        {
            record("nativeTrayContextOpened", m_menu->isVisible(),
                   {{"activationReasons", m_report.value("trayActivationReasons")}});
            key(VK_HOME);
        },
        [this] { key(VK_RETURN); },
        [this]
        {
            record("nativeTrayMenuShowHide", m_trayActions >= 1 && !m_pet->isVisible());
            setPetVisible(true);
            m_menu->actions()[1]->trigger();
            record("trayPassthroughAction", m_passthrough);
            m_menu->actions()[1]->trigger();
            m_menu->actions()[2]->trigger();
            record("trayExitIsHostRequest", m_exitRequests == 1 && QCoreApplication::instance());
        }};
    m_runner.start();
}
void WindowsProbe::finishChecks()
{
    if (m_finished)
        return;
    m_finished = true;
    m_runner.stop();
    m_observer.stop();
    if (m_leftDown || m_rightDown)
    {
        INPUT input{};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags =
            (m_leftDown ? MOUSEEVENTF_LEFTUP : 0) | (m_rightDown ? MOUSEEVENTF_RIGHTUP : 0);
        SendInput(1, &input, sizeof(input));
        m_leftDown = m_rightDown = false;
    }
    m_report.insert("prototypeAcceptance", m_allPassed        ? "Passed"
                                           : m_inputAvailable ? "Failed"
                                                              : "Pending");
    m_report.insert("acceptance", "Pending"); // Full platform matrix is a separate task.
    m_report.insert("timestampUtc", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    m_report.insert("targetProcessId", qint64(m_target.processId()));
    m_report.insert("probeProcessId", int(GetCurrentProcessId()));
    m_report.insert("systemInputAvailable", m_inputAvailable);
    m_report.insert("scope", "Single primary-screen Windows prototype; mixed DPI, full bubbles and "
                             "production adapter not accepted");
    if (m_allPassed)
    {
        auto capabilities = m_report.value("capabilities").toObject();
        for (const auto &name : {"transparency", "inputRegions", "wholeWindowPassthrough", "drag",
                                 "position", "alwaysOnTop", "focus", "tray"})
            capabilities.insert(
                name,
                QJsonObject{{"status", "Supported"},
                            {"reason", "Passed the recorded single-primary-screen Windows "
                                       "prototype; full platform acceptance remains Pending"}});
        m_report.insert("capabilities", capabilities);
    }
    QByteArray json = report().toUtf8();
    if (!m_output.isEmpty())
    {
        QSaveFile file(m_output);
        if (!file.open(QIODevice::WriteOnly) || file.write(json) != json.size() || !file.commit())
        {
            m_allPassed = false;
            m_report.insert("prototypeAcceptance", "Failed");
            m_report.insert("reportWriteError", file.errorString());
            json = report().toUtf8();
        }
    }
    std::fwrite(json.constData(), 1, size_t(json.size()), stdout);
    SetCursorPos(m_previousCursor.x(), m_previousCursor.y());
    if (IsWindow(hwnd(m_previousForeground)))
        SetForegroundWindow(hwnd(m_previousForeground));
    emit finished(m_allPassed);
}
