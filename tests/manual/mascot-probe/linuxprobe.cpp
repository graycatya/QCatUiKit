// SPDX-License-Identifier: Apache-2.0
#include "linuxprobe.h"
#include "mascotcompanion.h"
#include <QApplication>
#include <QCursor>
#include <QDateTime>
#include <QDir>
#include <QEvent>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QKeyEvent>
#include <QLibrary>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSaveFile>
#include <QScreen>
#include <QSystemTrayIcon>
#include <QUuid>
#include <cstdio>
#ifdef QCAT_PROBE_X11
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#undef KeyPress
#undef FocusIn
#undef FocusOut
#endif

struct LinuxProbe::Native
{
#ifdef QCAT_PROBE_X11
    Display *display = nullptr;
    Window target = 0;
    Window previousFocus = 0;
    QPoint previousCursor;
    QLibrary xtst{"Xtst", 6};
    using Motion = int (*)(Display *, int, int, int, unsigned long);
    using Button = int (*)(Display *, unsigned int, int, unsigned long);
    using Key = int (*)(Display *, unsigned int, int, unsigned long);
    Motion motion = nullptr;
    Button button = nullptr;
    Key key = nullptr;
    bool leftDown = false;
    Window focus() const
    {
        Window window = 0;
        int revert = 0;
        XGetInputFocus(display, &window, &revert);
        return window;
    }
    QPoint position(Window window) const
    {
        int x = 0, y = 0;
        Window child = 0;
        XTranslateCoordinates(display, window, DefaultRootWindow(display), 0, 0, &x, &y, &child);
        return {x, y};
    }
    bool ownedBy(Window window, qint64 pid) const
    {
        Atom type = 0;
        int format = 0;
        unsigned long size = 0, remaining = 0;
        unsigned char *data = nullptr;
        const int result =
            XGetWindowProperty(display, window, XInternAtom(display, "_NET_WM_PID", False), 0, 1,
                               False, XA_CARDINAL, &type, &format, &size, &remaining, &data);
        const bool owned = result == Success && type == XA_CARDINAL && format == 32 && size == 1 &&
                           data && qint64(*reinterpret_cast<unsigned long *>(data)) == pid;
        if (data)
            XFree(data);
        return owned;
    }
    bool above(Window window) const
    {
        const Atom state = XInternAtom(display, "_NET_WM_STATE", False);
        const Atom above = XInternAtom(display, "_NET_WM_STATE_ABOVE", False);
        Atom type = 0;
        int format = 0;
        unsigned long size = 0, remaining = 0;
        unsigned char *data = nullptr;
        bool result = false;
        if (XGetWindowProperty(display, window, state, 0, 64, False, XA_ATOM, &type, &format, &size,
                               &remaining, &data) == Success &&
            type == XA_ATOM && format == 32)
        {
            const auto *atoms = reinterpret_cast<Atom *>(data);
            for (unsigned long i = 0; i < size; ++i)
                result |= atoms[i] == above;
        }
        if (data)
            XFree(data);
        return result;
    }
    void activate(Window window)
    {
        XEvent event{};
        event.xclient.type = ClientMessage;
        event.xclient.window = window;
        event.xclient.message_type = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
        event.xclient.format = 32;
        event.xclient.data.l[0] = 1;
        event.xclient.data.l[1] = CurrentTime;
        XSendEvent(display, DefaultRootWindow(display), False,
                   SubstructureRedirectMask | SubstructureNotifyMask, &event);
        XFlush(display);
    }
    ~Native()
    {
        if (!display)
            return;
        if (leftDown && button)
            button(display, 1, False, CurrentTime);
        if (motion)
            motion(display, DefaultScreen(display), previousCursor.x(), previousCursor.y(),
                   CurrentTime);
        if (previousFocus && previousFocus != PointerRoot)
            activate(previousFocus);
        XFlush(display);
        XCloseDisplay(display);
    }
#endif
};

// Observe native-delivered window events in the independent target process.
class TargetObserver final : public QObject
{
  public:
    QQuickWindow *window = nullptr;
    QString token;
    int clicks = 0, keys = 0;
    void publish()
    {
        QJsonObject state{{"token", token},
                          {"pid", QCoreApplication::applicationPid()},
                          {"qpaPlatform", QGuiApplication::platformName()},
                          {"nativeId", QString::number(window->winId())},
                          {"mouse", clicks},
                          {"keys", keys},
                          {"active", window->isActive()},
                          {"dpr", window->devicePixelRatio()},
                          {"width", window->width()},
                          {"height", window->height()}};
        const QByteArray line = QJsonDocument(state).toJson(QJsonDocument::Compact) + '\n';
        std::fwrite(line.constData(), 1, size_t(line.size()), stdout);
        std::fflush(stdout);
    }
    bool eventFilter(QObject *, QEvent *event) override
    {
        if (event->type() == QEvent::MouseButtonPress)
            ++clicks;
        else if (event->type() == QEvent::KeyPress)
            ++keys;
        else if (event->type() != QEvent::FocusIn && event->type() != QEvent::FocusOut)
            return false;
        QTimer::singleShot(0, this, [this] { publish(); });
        return false;
    }
};

int LinuxProbe::runTarget(const QString &token)
{
    if (QGuiApplication::platformName() != "xcb" &&
        !QGuiApplication::platformName().startsWith("wayland"))
        return 2;
    QQmlEngine engine;
    QQmlComponent component(&engine, QUrl("qrc:/LinuxTarget.qml"));
    std::unique_ptr<QObject> object(component.create());
    auto *window = qobject_cast<QQuickWindow *>(object.get());
    if (!window)
        return 1;
    window->setPosition(QGuiApplication::primaryScreen()->availableGeometry().topLeft() +
                        QPoint(80, 80));
    TargetObserver observer;
    observer.window = window;
    observer.token = token;
    window->installEventFilter(&observer);
    QObject::connect(window, &QQuickWindow::closing, qApp, &QCoreApplication::quit);
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, &observer, [&observer] { observer.publish(); });
    timer.start(250);
    return qApp->exec();
}

LinuxProbe::LinuxProbe(QQmlEngine *engine, QJsonObject environment)
    : m_native(std::make_unique<Native>()), m_engine(engine), m_report(std::move(environment)),
      m_token(QUuid::createUuid().toString(QUuid::WithoutBraces))
{
    m_report.insert("prototypeAcceptance", "Pending");
    m_report.insert("probePid", QCoreApplication::applicationPid());
    m_report.insert("instanceToken", m_token);
    if (QGuiApplication::platformName().startsWith("wayland"))
    {
        m_companion = std::make_unique<MascotCompanion>(m_token);
        connect(m_companion.get(), &MascotCompanion::changed, this,
                [this]
                {
                    updateCompanionStatus();
                    if (m_companion->ready() && m_pet && m_pet->isVisible())
                        companionCall(
                            "snapshot"); // Reconnect never replays movement or topmost requests.
                });
        connect(
            m_companion.get(), &MascotCompanion::completed, this,
            [this](quint64 id, bool applied, const QString &reason, const QJsonObject &result)
            {
                const auto operation = m_companionOperations.take(id);
                m_report.insert("lastCompanionRequest", QJsonObject{{"id", QString::number(id)},
                                                                    {"operation", operation},
                                                                    {"applied", applied},
                                                                    {"reason", reason},
                                                                    {"result", result}});
                if (applied)
                {
                    m_report.insert("nativeWindow", result);
                    if (operation == "move" || operation == "above")
                    {
                        auto capabilities = m_report.value("capabilities").toObject();
                        capabilities.insert(
                            operation == "move" ? "position" : "alwaysOnTop",
                            QJsonObject{{"status", "Supported"},
                                        {"reason",
                                         "Companion native readback in this prototype run only"}});
                        m_report.insert("capabilities", capabilities);
                    }
                }
                if (m_automatic)
                {
                    if (m_companion->ready())
                        record("companion." + operation, applied,
                               {{"reason", reason}, {"result", result}});
                    else
                        pending("companion." + operation, reason);
                }
                else
                    update();
            });
        updateCompanionStatus();
    }
    m_reportTimer.setSingleShot(true);
    m_reportTimer.setInterval(100);
    connect(&m_reportTimer, &QTimer::timeout, this, &LinuxProbe::refreshReport);
    connect(&m_target, &QProcess::readyReadStandardOutput, this,
            [this]
            {
                m_buffer += m_target.readAllStandardOutput();
                while (m_buffer.contains('\n'))
                {
                    const auto end = m_buffer.indexOf('\n');
                    const auto state = QJsonDocument::fromJson(m_buffer.left(end)).object();
                    m_buffer.remove(0, end + 1);
                    if (state.value("token").toString() == m_token &&
                        state.value("pid").toInteger() == m_target.processId() &&
                        state.value("qpaPlatform").toString() == QGuiApplication::platformName())
                        m_targetState = state;
                }
                update();
            });
    connect(&m_target, &QProcess::errorOccurred, this,
            [this](QProcess::ProcessError)
            {
                m_report.insert("targetError", m_target.errorString());
                update();
            });
    m_runner.setInterval(500);
    connect(
        &m_runner, &QTimer::timeout, this,
        [this]
        {
            if (!m_ready)
            {
                if (m_targetState.isEmpty())
                {
                    if (++m_ticks >= 20)
                    {
                        record("independentTarget", false,
                               {{"reason", "No verified child handshake"},
                                {"stderr", QString::fromUtf8(m_target.readAllStandardError())}});
                        finishChecks();
                    }
                    return;
                }
                m_ready = true;
                record("independentTarget", true, {{"targetPid", m_target.processId()}});
#ifdef QCAT_PROBE_X11
                if (m_native->display)
                {
                    m_native->target = m_targetState.value("nativeId").toString().toULong();
                    if (!m_native->ownedBy(m_native->target, m_target.processId()))
                    {
                        record("nativeTargetOwnership", false);
                        finishChecks();
                        return;
                    }
                    record("nativeTargetOwnership", true);
                    m_native->activate(m_native->target);
                    XRaiseWindow(m_native->display, m_native->target);
                    XSetInputFocus(m_native->display, m_native->target, RevertToParent,
                                   CurrentTime);
                    XFlush(m_native->display);
                }
#endif
                QTimer::singleShot(
                    700, this,
                    [this]
                    {
                        QQmlComponent component(m_engine, QUrl("qrc:/LinuxOverlay.qml"));
                        m_pet = qobject_cast<QQuickWindow *>(component.createWithInitialProperties(
                            {{"probe", QVariant::fromValue(this)}}));
                        if (!m_pet)
                        {
                            record("overlayCreated", false, {{"error", component.errorString()}});
                            finishChecks();
                            return;
                        }
                        m_pet->installEventFilter(this);
                        m_pet->setTitle("QCat mascot probe:" + m_token);
                        applyMask();
                        resetPosition();
                        m_pet->show();
                        if (m_companion)
                            QTimer::singleShot(250, this,
                                               [this]
                                               {
                                                   if (m_companion->ready())
                                                       companionCall("snapshot");
                                               });
                        prepareChecks();
                    });
                return;
            }
            if (!m_pet || !m_automatic)
                return;
            if (!m_companionOperations.isEmpty())
                return;
            if (m_step < int(m_steps.size()))
                m_steps[size_t(m_step++)]();
            else
                finishChecks();
        });
}

LinuxProbe::~LinuxProbe()
{
    m_runner.stop();
    m_target.disconnect(this);
    if (m_companion)
        m_companion->disconnect(this);
    if (m_tray)
        m_tray->hide();
    if (m_pet)
        delete m_pet;
    m_pet = nullptr;
    m_target.terminate();
    if (!m_target.waitForFinished(1500))
    {
        m_target.kill();
        m_target.waitForFinished(1500);
    }
}

QString LinuxProbe::report() const
{
    return m_reportText;
}

void LinuxProbe::setDiagnosticsVisible(bool visible)
{
    m_diagnosticsVisible = visible;
    m_reportTimer.stop();
    if (visible)
        refreshReport();
}

void LinuxProbe::refreshReport()
{
    if (!m_diagnosticsVisible)
        return;
    m_reportText = QString::fromUtf8(QJsonDocument(m_report).toJson());
    ++m_reportRefreshes;
    emit reportChanged();
}

bool LinuxProbe::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_pet &&
        (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseMove ||
         event->type() == QEvent::MouseButtonRelease))
    {
        m_pointerPosition = static_cast<QMouseEvent *>(event)->globalPosition().toPoint();
        if (event->type() == QEvent::MouseMove && m_dragging)
            bodyMoved();
    }
    return QObject::eventFilter(watched, event);
}

void LinuxProbe::start(const QString &output)
{
    m_automatic = !output.isEmpty();
    m_output =
        m_automatic ? output : QDir::temp().filePath("qcat-linux-probe-" + m_token + ".json");
    m_report.insert("outputPath", m_output);
    if (m_companion)
        m_companion->start();
#ifdef QCAT_PROBE_X11
    if (QGuiApplication::platformName() == "xcb")
    {
        m_native->display = XOpenDisplay(nullptr);
        if (m_native->display)
        {
            m_native->previousFocus = m_native->focus();
            m_native->previousCursor = QCursor::pos();
            m_native->motion =
                reinterpret_cast<Native::Motion>(m_native->xtst.resolve("XTestFakeMotionEvent"));
            m_native->button =
                reinterpret_cast<Native::Button>(m_native->xtst.resolve("XTestFakeButtonEvent"));
            m_native->key =
                reinterpret_cast<Native::Key>(m_native->xtst.resolve("XTestFakeKeyEvent"));
        }
    }
#endif
    m_menu = std::make_unique<QMenu>();
    auto action = [this](const QString &text, auto callback)
    {
        connect(m_menu->addAction(text), &QAction::triggered, this,
                [this, callback, text]
                {
                    ++m_menuActions;
                    m_report.insert("lastMenuAction", text);
                    callback();
                    update();
                });
    };
    action(tr("Show"), [this] { setPetVisible(true); });
    action(tr("Hide"), [this] { setPetVisible(false); });
    action(tr("Toggle passthrough"), [this] { setPassthrough(!m_passthrough); });
    action(tr("Exit request"), [this] { ++m_exitRequests; });
    QPixmap pixmap(32, 32);
    pixmap.fill(QColor("#e6a64b"));
    m_tray = std::make_unique<QSystemTrayIcon>(QIcon(pixmap));
    m_tray->setToolTip(tr("QCat Linux mascot probe"));
    m_tray->setContextMenu(m_menu.get());
    m_report.insert("trayServiceAvailable", QSystemTrayIcon::isSystemTrayAvailable());
    m_tray->show();
    m_target.start(QCoreApplication::applicationFilePath(), {"--linux-target", m_token});
    m_runner.start();
    update();
}

void LinuxProbe::applyMask()
{
    if (!m_pet)
        return;
    QRegion region(QRect(120, 110, 180, 170), QRegion::Ellipse);
    if (m_bubble)
        region += QRect(60, 20, 300, 60);
    m_pet->setMask(region);
}
void LinuxProbe::setPassthrough(bool enabled)
{
    m_passthrough = enabled;
    if (m_pet)
    {
        m_pet->setFlag(Qt::WindowTransparentForInput, enabled);
        applyMask();
    }
    update();
}
void LinuxProbe::setTopmost(bool enabled)
{
    m_topmost = enabled;
    if (m_companion)
    {
        companionCall("above", {{"enabled", enabled}});
        update();
        return;
    }
    if (m_pet)
        m_pet->setFlag(Qt::WindowStaysOnTopHint, enabled);
    update();
}
void LinuxProbe::setBubble(bool enabled)
{
    m_bubble = enabled;
    if (m_pet)
    {
        m_pet->setProperty("bubbleVisible", enabled);
        applyMask();
    }
    update();
}
void LinuxProbe::setPetVisible(bool enabled)
{
    if (m_pet)
        m_pet->setVisible(enabled);
    update();
}
void LinuxProbe::resetPosition()
{
    if (!m_pet)
        return;
#ifdef QCAT_PROBE_X11
    if (m_native->display && m_native->target)
        m_origin = m_native->position(m_native->target) + QPoint(80, 80);
    else
#endif
        m_origin =
            QGuiApplication::primaryScreen()->availableGeometry().topLeft() + QPoint(160, 160);
    m_pet->setPosition(m_origin);
    if (m_companion && m_pet->isVisible())
        companionCall("move", {{"x", m_origin.x()}, {"y", m_origin.y()}});
    update();
}

void LinuxProbe::updateCompanionStatus()
{
    const auto status = m_companion->status();
    m_report.insert("companion", status);
    m_report.insert("companionConnection", status.value("state"));
    m_report.insert("companionVersion", status.value("version"));
    auto capabilities = m_report.value("capabilities").toObject();
    for (const QString &name : {QString("position"), QString("alwaysOnTop")})
        capabilities.insert(
            name, QJsonObject{{"status", m_companion->ready() ? "Unknown" : "Unavailable"},
                              {"reason", status.value("reason")}});
    capabilities.insert(
        "smoothMovement",
        QJsonObject{{"status", "Unavailable"},
                    {"reason", "Smooth movement is not implemented by this primitive probe"}});
    m_report.insert("capabilities", capabilities);
    if (!m_companion->ready())
        m_report.remove("nativeWindow");
    update();
}

void LinuxProbe::companionCall(const QString &operation, const QJsonObject &arguments)
{
    if (m_companion)
        m_companionOperations.insert(m_companion->call(operation, arguments), operation);
}

void LinuxProbe::snapshotCompanion()
{
    companionCall("snapshot");
}
void LinuxProbe::bodyPressed()
{
    ++m_bodyClicks;
    m_dragging = true;
    m_dragCursor = m_pointerPosition;
    m_dragPosition = m_pet->position();
    m_dragClock.start();
    m_lastDragEventMs = -1;
    m_maxDragEventGapMs = 0;
    if (QGuiApplication::platformName().startsWith("wayland"))
        m_report.insert("systemMoveAccepted", m_pet->startSystemMove());
    update();
}
void LinuxProbe::bodyMoved()
{
    if (!m_dragging || QGuiApplication::platformName() != "xcb")
        return;
    const QPoint delta = m_pointerPosition - m_dragCursor;
    if (!delta.isNull())
    {
        const qint64 elapsed = m_dragClock.elapsed();
        if (m_lastDragEventMs >= 0)
            m_maxDragEventGapMs = qMax(m_maxDragEventGapMs, elapsed - m_lastDragEventMs);
        m_lastDragEventMs = elapsed;
        ++m_drags;
        m_pet->setPosition(m_dragPosition + delta);
    }
    update();
}
void LinuxProbe::bodyReleased()
{
    m_dragging = false;
    update();
}
void LinuxProbe::bubbleActivated()
{
    ++m_bubbleClicks;
    m_pet->setFlag(Qt::WindowDoesNotAcceptFocus, false);
    m_pet->requestActivate();
    update();
}

void LinuxProbe::update()
{
    m_report.insert("target", m_targetState);
    m_report.insert("bodyClicks", m_bodyClicks);
    m_report.insert("bubbleClicks", m_bubbleClicks);
    m_report.insert("dragEvents", m_drags);
    m_report.insert("diagnosticsRefreshes", m_reportRefreshes);
    m_report.insert("passthroughRequested", m_passthrough);
    m_report.insert("topmostRequested", m_topmost);
    m_report.insert("menuActions", m_menuActions);
    m_report.insert("exitRequests", m_exitRequests);
    if (m_pet)
    {
        m_report.insert("petVisible", m_pet->isVisible());
        m_report.insert("graphicsApi", int(m_pet->rendererInterface()->graphicsApi()));
        m_report.insert("qtReportedPosition", QJsonArray{m_pet->x(), m_pet->y()});
    }
    if (m_diagnosticsVisible && !m_reportTimer.isActive())
        m_reportTimer.start();
    emit changed();
}
void LinuxProbe::save()
{
    update();
    m_report.insert("recordedAtUtc", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    m_report.remove("writeError");
    QSaveFile file(m_output);
    const auto bytes = QJsonDocument(m_report).toJson();
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
        m_report.insert("writeError", file.errorString());
    emit changed();
}
void LinuxProbe::record(const QString &name, bool passed, const QJsonObject &detail)
{
    auto checks = m_report.value("checks").toArray();
    checks.append(QJsonObject{{"name", name},
                              {"status", passed ? "Passed" : "Failed"},
                              {"detail", detail},
                              {"target", m_targetState},
                              {"bodyClicks", m_bodyClicks},
                              {"bubbleClicks", m_bubbleClicks}});
    m_report.insert("checks", checks);
    m_failed |= !passed;
    save();
}
void LinuxProbe::pending(const QString &name, const QString &reason)
{
    auto checks = m_report.value("checks").toArray();
    checks.append(QJsonObject{{"name", name}, {"status", "Pending"}, {"reason", reason}});
    m_report.insert("checks", checks);
    save();
}
int LinuxProbe::targetClicks() const
{
    return m_targetState.value("mouse").toInt();
}
int LinuxProbe::targetKeys() const
{
    return m_targetState.value("keys").toInt();
}
QPoint LinuxProbe::nativePosition() const
{
#ifdef QCAT_PROBE_X11
    if (m_native->display && m_pet)
        return m_native->position(m_pet->winId());
#endif
    return {};
}
bool LinuxProbe::targetFocused() const
{
#ifdef QCAT_PROBE_X11
    if (m_native->display)
        return m_native->focus() == m_native->target;
#endif
    return false;
}
bool LinuxProbe::petFocused() const
{
#ifdef QCAT_PROBE_X11
    if (m_native->display)
        return m_native->focus() == m_pet->winId();
#endif
    return false;
}
bool LinuxProbe::mouse(const QPoint &local, int action)
{
#ifdef QCAT_PROBE_X11
    if (m_native->display && m_native->motion && m_native->button && m_pet &&
        m_pet->devicePixelRatio() == 1 && m_targetState.value("dpr").toDouble() == 1)
    {
        const QPoint point = nativePosition() + local;
        const QRect targetRect(
            m_native->position(m_native->target),
            QSize(m_targetState.value("width").toInt(), m_targetState.value("height").toInt()));
        if (!targetRect.adjusted(2, 2, -2, -2).contains(point))
            return false;
        m_native->motion(m_native->display, DefaultScreen(m_native->display), point.x(), point.y(),
                         CurrentTime);
        XSync(m_native->display, False);
        // XWayland applies pointer motion asynchronously; inspect ownership on a later turn.
        QTimer::singleShot(120, this,
                           [this, point, action]
                           {
                               Window root = 0, child = 0;
                               int rx = 0, ry = 0, wx = 0, wy = 0;
                               unsigned int mask = 0;
                               XQueryPointer(m_native->display,
                                             DefaultRootWindow(m_native->display), &root, &child,
                                             &rx, &ry, &wx, &wy, &mask);
                               if ((child != m_native->target && child != m_pet->winId()) ||
                                   QPoint(rx, ry) != point)
                               {
                                   record("inputOwnership", false,
                                          {{"child", QString::number(child)},
                                           {"targetId", QString::number(m_native->target)},
                                           {"petId", QString::number(m_pet->winId())},
                                           {"actualPointer", QJsonArray{rx, ry}},
                                           {"requestedPointer", QJsonArray{point.x(), point.y()}}});
                                   return;
                               }
                               if (action != 3)
                               {
                                   if (action != 2)
                                   {
                                       m_native->button(m_native->display, 1, True, CurrentTime);
                                       m_native->leftDown = true;
                                   }
                                   if (action != 1)
                                   {
                                       m_native->button(m_native->display, 1, False, CurrentTime);
                                       m_native->leftDown = false;
                                   }
                               }
                               XFlush(m_native->display);
                           });
        return true;
    }
#endif
    return false;
}
bool LinuxProbe::key(unsigned long symbol)
{
#ifdef QCAT_PROBE_X11
    if (m_native->display && m_native->key && (targetFocused() || petFocused()))
    {
        const auto code = XKeysymToKeycode(m_native->display, symbol);
        if (!code)
            return false;
        m_native->key(m_native->display, code, True, CurrentTime);
        m_native->key(m_native->display, code, False, CurrentTime);
        XFlush(m_native->display);
        return true;
    }
#endif
    return false;
}

void LinuxProbe::prepareChecks()
{
    if (!m_automatic)
        return;
    m_steps.push_back(
        [this]
        {
            const auto image = m_pet->grabWindow();
            bool alpha = !image.isNull() && image.hasAlphaChannel();
            if (alpha)
                alpha = image.pixelColor(20, 160).alpha() == 0 &&
                        image.pixelColor(200, 180).alpha() == 255;
            record("renderedAlpha", alpha,
                   {{"scope", "Qt render target, not desktop composition"}});
            if (!image.isNull())
                m_report.insert("overlayImageSaved", image.save(m_output + ".png"));
#ifdef QCAT_PROBE_X11
            if (m_native->display)
            {
                const QPoint origin = nativePosition();
                const QImage desktop =
                    m_pet->screen()->grabWindow(0, origin.x(), origin.y(), 420, 320).toImage();
                const QColor background("#265d83");
                // A valid blue reference is required: some XWayland servers cannot capture the
                // desktop.
                if (desktop.size() == QSize(420, 320) &&
                    desktop.pixelColor(20, 300).rgb() == background.rgb())
                {
                    const bool transparent =
                        desktop.pixelColor(20, 160).rgb() == background.rgb() &&
                        desktop.pixelColor(200, 180).rgb() == QColor("#e6a64b").rgb();
                    record("desktopComposition", transparent);
                    m_report.insert("desktopImageSaved", desktop.save(m_output + ".desktop.png"));
                }
                else
                    pending("desktopComposition",
                            "Desktop capture has no usable independent background reference");
            }
#endif
            record("trayServiceDiagnostic", m_report.value("trayServiceAvailable").isBool(),
                   {{"available", m_report.value("trayServiceAvailable")},
                    {"scope", "service query only, menu input still Pending"}});
        });
    if (QGuiApplication::platformName().startsWith("wayland"))
    {
        updateCompanionStatus();
        m_steps.push_back(
            [this]
            {
                pending("nativeWaylandInput",
                        "No XTest substitution; native input still requires manual evidence");
                if (m_companion->ready())
                    companionCall("move", {{"x", m_origin.x()}, {"y", m_origin.y()}});
                else
                    pending("nativePositionAndTopmost",
                            m_companion->status().value("reason").toString());
            });
        m_steps.push_back(
            [this]
            {
                if (m_companion->ready())
                    setTopmost(false);
            });
        m_steps.push_back(
            [this]
            {
                if (m_companion->ready())
                    setTopmost(true);
            });
        m_steps.push_back(
            [this]
            {
                if (m_companion->ready())
                    companionCall("snapshot");
                setPassthrough(true);
            });
        m_steps.push_back(
            [this]
            {
                setPassthrough(false);
                setBubble(false);
            });
        m_steps.push_back(
            [this]
            {
                setBubble(true);
                setPetVisible(false);
            });
        m_steps.push_back(
            [this]
            {
                record("hide", !m_pet->isVisible());
                setPetVisible(true);
            });
        m_steps.push_back([this] { record("hostRestore", m_pet->isVisible()); });
        return;
    }
#ifdef QCAT_PROBE_X11
    if (m_native->display && m_native->motion && m_native->button && m_native->key &&
        m_pet->devicePixelRatio() == 1 && m_targetState.value("dpr").toDouble() == 1)
    {
        m_steps.push_back(
            [this]
            {
                record("passiveShowFocus", targetFocused());
                const QPoint point = nativePosition();
                record("nativePosition", point == m_origin,
                       {{"actual", QJsonArray{point.x(), point.y()}},
                        {"requested", QJsonArray{m_origin.x(), m_origin.y()}}});
                record("topmostOn", m_native->above(m_pet->winId()));
                record("blankInputSent", mouse({20, 160}));
            });
        m_steps.push_back(
            [this]
            {
                record("blankToBehind", targetClicks() == 1 && m_bodyClicks == 0);
                record("ellipseCornerInputSent", mouse({125, 115}));
            });
        m_steps.push_back(
            [this]
            {
                record("ellipseCornerToBehind", targetClicks() == 2 && m_bodyClicks == 0);
                record("bodyInputSent", mouse({200, 180}));
            });
        m_steps.push_back(
            [this]
            {
                record("bodyReceivesWithoutFocus",
                       m_bodyClicks == 1 && targetClicks() == 2 && targetFocused());
                record("behindKeySent", key(XK_a));
            });
        m_steps.push_back(
            [this]
            {
                record("behindKeyboard", targetKeys() == 1);
                setPassthrough(true);
            });
        m_steps.push_back([this] { record("passBodyInputSent", mouse({200, 180})); });
        m_steps.push_back(
            [this]
            {
                record("passthroughBody", targetClicks() == 3 && m_bodyClicks == 1);
                record("passBubbleInputSent", mouse({100, 50}));
            });
        m_steps.push_back(
            [this]
            {
                record("passthroughBubble", targetClicks() == 4 && m_bubbleClicks == 0);
                setPassthrough(false);
                setBubble(false);
            });
        m_steps.push_back([this] { record("hiddenBubbleInputSent", mouse({100, 50})); });
        m_steps.push_back(
            [this]
            {
                record("hiddenBubbleToBehind", targetClicks() == 5 && m_bubbleClicks == 0);
                setBubble(true);
            });
        m_steps.push_back([this] { record("activeBubbleInputSent", mouse({100, 50})); });
        m_steps.push_back(
            [this]
            {
                record("bubbleReceivesAndFocuses",
                       m_bubbleClicks == 1 && targetClicks() == 5 && petFocused());
                record("bubbleTabSent", key(XK_Tab));
            });
        m_steps.push_back([this] { record("bubbleSpaceSent", key(XK_space)); });
        m_steps.push_back(
            [this]
            {
                record("bubbleKeyboard", m_bubbleClicks == 2 && targetKeys() == 1);
                record("dragPressSent", mouse({200, 180}, 1));
            });
        m_steps.push_back([this] { record("dragMotionSent", mouse({230, 200}, 3)); });
        m_steps.push_back([this] { record("dragReleaseSent", mouse({200, 180}, 2)); });
        m_steps.push_back(
            [this]
            {
                const QPoint point = nativePosition();
                record("nativeDrag", point == m_origin + QPoint(30, 20) && m_drags > 0,
                       {{"actual", QJsonArray{point.x(), point.y()}}, {"dragEvents", m_drags}});
                resetPosition();
            });
        m_steps.push_back([this]
                          { record("externalPositionReset", nativePosition() == m_origin); });
        m_steps.push_back([this] { record("denseDragPressSent", mouse({230, 200}, 1)); });
        m_steps.push_back([this] { runDenseDrag(); });
        m_steps.push_back(
            [this]
            {
                resetPosition();
                setTopmost(false);
            });
        m_steps.push_back(
            [this]
            {
                record("topmostOff", !m_native->above(m_pet->winId()));
                setTopmost(true);
            });
        m_steps.push_back(
            [this]
            {
                record("topmostRestored", m_native->above(m_pet->winId()));
                setPetVisible(false);
            });
        m_steps.push_back(
            [this]
            {
                record("hide", !m_pet->isVisible());
                setPetVisible(true);
            });
        m_steps.push_back([this] { record("hostRestore", m_pet->isVisible()); });
        return;
    }
#endif
    m_steps.push_back(
        [this]
        {
            pending(
                "xcbNativeInput",
                "X11/XTest unavailable or DPR differs from single-screen DPR 1 prototype scope");
        });
}

void LinuxProbe::finishChecks()
{
    m_runner.stop();
    const auto checks = m_report.value("checks").toArray();
    auto passed = [&checks](const QString &name)
    {
        for (const auto &check : checks)
            if (check.toObject().value("name").toString() == name)
                return check.toObject().value("status").toString() == "Passed";
        return false;
    };
    auto capabilities = m_report.value("capabilities").toObject();
    const QList<QPair<QString, QStringList>> evidence = {
        {"inputRegions",
         {"blankToBehind", "ellipseCornerToBehind", "bodyReceivesWithoutFocus",
          "hiddenBubbleToBehind"}},
        {"wholeWindowPassthrough", {"passthroughBody", "passthroughBubble"}},
        {"focus",
         {"passiveShowFocus", "bodyReceivesWithoutFocus", "behindKeyboard", "bubbleKeyboard"}},
        {"drag", {"nativeDrag"}},
        {"position", {"nativePosition", "externalPositionReset"}},
        {"alwaysOnTop", {"topmostOn", "topmostOff", "topmostRestored"}},
        {"transparency", {"renderedAlpha", "desktopComposition"}}};
    for (const auto &entry : evidence)
    {
        bool complete = true;
        for (const auto &check : entry.second)
            complete &= passed(check);
        if (complete)
            capabilities.insert(
                entry.first,
                QJsonObject{{"status", "Supported"},
                            {"reason", "Observed in this single-screen DPR 1 prototype run only; "
                                       "full acceptance Pending"}});
    }
    m_report.insert("capabilities", capabilities);
    bool compositionRecorded = false;
    for (const auto &check : checks)
        compositionRecorded |= check.toObject().value("name").toString() == "desktopComposition";
    if (!compositionRecorded)
        pending("desktopComposition",
                "Render-target alpha is not compositor transparency evidence");
    pending("trayMenuInput",
            "Service availability and menu callbacks do not prove actual tray interaction");
    m_report.insert("prototypeAcceptance", m_failed ? "Failed" : "Pending");
    save();
    if (m_automatic)
        emit finished(m_failed || m_report.contains("writeError") ? 1 : 2);
}

void LinuxProbe::runDenseDrag()
{
#ifdef QCAT_PROBE_X11
    m_runner.stop();
    if (!m_dragging || m_diagnosticsVisible)
    {
        record("minimizedContinuousDrag", false,
               {{"reason", "Drag or minimized baseline missing"}});
        m_runner.start();
        return;
    }
    const int refreshBaseline = m_reportRefreshes;
    const int dragBaseline = m_drags;
    auto *timer = new QTimer(this);
    timer->setInterval(16);
    auto count = std::make_shared<int>(0);
    connect(timer, &QTimer::timeout, this,
            [this, timer, count, refreshBaseline, dragBaseline]
            {
                if (++*count <= 40)
                {
                    const QPoint point = m_dragCursor + QPoint(*count, *count / 2);
                    m_native->motion(m_native->display, DefaultScreen(m_native->display), point.x(),
                                     point.y(), CurrentTime);
                    XFlush(m_native->display);
                    return;
                }
                timer->stop();
                timer->deleteLater();
                QTimer::singleShot(
                    250, this,
                    [this, refreshBaseline, dragBaseline]
                    {
                        m_native->button(m_native->display, 1, False, CurrentTime);
                        m_native->leftDown = false;
                        XFlush(m_native->display);
                        QTimer::singleShot(
                            150, this,
                            [this, refreshBaseline, dragBaseline]
                            {
                                const QPoint position = nativePosition();
                                record("minimizedContinuousDrag",
                                       position == m_origin + QPoint(40, 20) &&
                                           m_drags > dragBaseline &&
                                           m_reportRefreshes == refreshBaseline,
                                       {{"actual", QJsonArray{position.x(), position.y()}},
                                        {"requestedMotionEvents", 40},
                                        {"receivedMotionEvents", m_drags - dragBaseline},
                                        {"maxEventGapMs", m_maxDragEventGapMs},
                                        {"diagnosticRefreshesDuringDrag",
                                         m_reportRefreshes - refreshBaseline}});
                                m_runner.start();
                            });
                    });
            });
    timer->start();
#endif
}
