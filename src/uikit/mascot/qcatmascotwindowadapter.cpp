// SPDX-License-Identifier: Apache-2.0
#include "qcatmascotwindowadapter_p.h"
#include <QGuiApplication>
#include <QLibrary>
#include <QMouseEvent>
#include <QPolygon>
#include <QScreen>
#include <QStyleHints>
#include <QtGui/qguiapplication_platform.h>
#include <cmath>
#include <utility>
#if defined(QCATUIKIT_HAS_XSHAPE) && QT_CONFIG(xcb)
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#undef Status
#undef None
#undef Bool
#undef Expose
#endif

namespace QCatUiKit
{
namespace
{
#if defined(QCATUIKIT_HAS_XSHAPE) && QT_CONFIG(xcb)
// Load the optional Xext runtime; building the library needs no Xext headers.
struct ShapeApi
{
    QLibrary library{"Xext", 6};
    using Query = int (*)(Display *, int *, int *);
    using Combine = void (*)(Display *, Window, int, int, int, XRectangle *, int, int, int);
    Query query = reinterpret_cast<Query>(library.resolve("XShapeQueryExtension"));
    Query version = reinterpret_cast<Query>(library.resolve("XShapeQueryVersion"));
    Combine combine = reinterpret_cast<Combine>(library.resolve("XShapeCombineRectangles"));
    bool available(Display *display) const
    {
        int event = 0, error = 0, major = 0, minor = 0;
        return display && query && version && combine && query(display, &event, &error) &&
               version(display, &major, &minor) && (major > 1 || (major == 1 && minor >= 1));
    }
};
ShapeApi &shapeApi()
{
    static ShapeApi api;
    return api;
}
#endif
QString screenKey(QScreen *s)
{
    return s->name() + QLatin1Char('|') + s->manufacturer() + QLatin1Char('|') + s->serialNumber();
}
bool finite(const QPointF &p)
{
    return std::isfinite(p.x()) && std::isfinite(p.y()) && std::abs(p.x()) < 1e7 &&
           std::abs(p.y()) < 1e7;
}
} // namespace
QRect MascotWindowAdapter::availableGeometry() const
{
    auto *screen = findScreen({});
    return screen ? screen->availableGeometry() : QRect(0, 0, 800, 600);
}
void MascotWindowAdapter::setCharacterOffset(const QPoint &offset)
{
    if (offset == m_characterOffset)
        return;
    const auto delta = m_characterOffset - offset;
    if (m_window && canPosition() && !m_layoutTimer.isActive())
        m_layoutAnchor = m_window->position() + m_characterOffset;
    m_characterOffset = offset;
    if (m_window && canPosition())
    {
        m_from += delta;
        m_to += delta;
        if (m_move.isEmpty())
        {
            m_window->setPosition(m_layoutAnchor - m_characterOffset);
            // Layout and native resize acknowledgements can arrive in different
            // event turns. Keep one origin through the whole transition.
            m_layoutTimer.start();
        }
        else
        {
            m_layoutTimer.stop();
            m_window->setPosition(m_window->position() + delta);
        }
    }
    applyMask();
    emit characterOffsetChanged();
    // Layout offsets are written while QML evaluates sprite geometry. Notify
    // placement on the next turn to avoid re-entering the same bindings.
    QMetaObject::invokeMethod(
        this, [this] { emit characterPositionChanged(); }, Qt::QueuedConnection);
}
MascotWindowAdapter::MascotWindowAdapter(QObject *parent) : QObject(parent)
{
    m_timer.setInterval(16);
    m_layoutTimer.setSingleShot(true);
    m_layoutTimer.setInterval(50);
    connect(&m_layoutTimer, &QTimer::timeout, this,
            [this]
            {
                if (m_window && canPosition())
                    m_window->setPosition(m_layoutAnchor - m_characterOffset);
            });
    connect(&m_timer, &QTimer::timeout, this,
            [this]
            {
                if (!m_window)
                {
                    finishMove("Cancelled");
                    return;
                }
                const qreal t = qMin(1., qreal(m_clock.elapsed()) / m_duration);
                m_window->setPosition(m_from + ((m_to - m_from) * t));
                if (t >= 1)
                    finishMove(m_window->position() == m_to ? "Completed" : "Failed");
            });
    connect(this, &MascotWindowAdapter::changed, this, &MascotWindowAdapter::refresh);
    const auto screenChanged = [this]
    {
        cancelMove();
        syncGeometry();
        if (m_window && canPosition() && m_constrain)
            m_window->setPosition(bounded(findScreen({}), m_window->position()));
        emit screenGeometryChanged();
        emit changed();
    };
    connect(qGuiApp, &QGuiApplication::screenRemoved, this, screenChanged);
    connect(qGuiApp, &QGuiApplication::screenAdded, this,
            [this](QScreen *screen)
            {
                connect(screen, &QScreen::availableGeometryChanged, this,
                        [this]
                        {
                            cancelMove();
                            syncGeometry();
                            emit screenGeometryChanged();
                        });
                emit screenGeometryChanged();
                emit changed();
            });
    for (auto *screen : QGuiApplication::screens())
        connect(screen, &QScreen::availableGeometryChanged, this, screenChanged);
}
MascotWindowAdapter::~MascotWindowAdapter()
{
    if (m_controller && m_dragging)
        m_controller->endDrag();
}
void MascotWindowAdapter::setWindow(QQuickWindow *w)
{
    if (m_window == w)
        return;
    cancelMove();
    if (m_window)
    {
        m_window->removeEventFilter(this);
        disconnect(m_window, nullptr, this, nullptr);
    }
    m_window = w;
    if (w)
    {
        w->setTransientParent(nullptr);
        connect(w, &QWindow::transientParentChanged, this,
                [this]
                {
                    if (m_window && m_window->transientParent())
                        m_window->setTransientParent(nullptr);
                });
        w->installEventFilter(this);
        connect(w, &QWindow::xChanged, this, &MascotWindowAdapter::characterPositionChanged,
                Qt::QueuedConnection);
        connect(w, &QWindow::yChanged, this, &MascotWindowAdapter::characterPositionChanged,
                Qt::QueuedConnection);
        connect(w, &QWindow::widthChanged, this, &MascotWindowAdapter::applyMask);
        connect(w, &QWindow::heightChanged, this, &MascotWindowAdapter::applyMask);
        connect(w, &QWindow::screenChanged, this,
                [this]
                {
                    cancelMove();
                    syncGeometry();
                    emit screenGeometryChanged();
                    emit changed();
                });
        connect(w, &QWindow::visibleChanged, this,
                [this]
                {
                    if (!m_window->isVisible())
                    {
                        cancelMove();
                        m_pressed = false;
                        if (m_dragging && m_controller)
                            m_controller->endDrag();
                        m_dragging = false;
                    }
                });
    }
    emit screenGeometryChanged();
    emit changed();
}
void MascotWindowAdapter::setController(MascotPetController *c)
{
    if (m_controller == c)
        return;
    if (m_controller)
        disconnect(m_controller, nullptr, this, nullptr);
    m_controller = c;
    if (c)
    {
        connect(c, &MascotPetController::regionsChanged, this, &MascotWindowAdapter::applyMask);
        connect(c, &MascotPetController::petScaleChanged, this, &MascotWindowAdapter::syncGeometry);
        connect(c, &MascotPetController::anchorsChanged, this, &MascotWindowAdapter::syncGeometry);
        connect(c, &MascotPetController::packageChanged, this, &MascotWindowAdapter::syncGeometry);
        connect(c, &MascotPetController::draggingChanged, this,
                [this]
                {
                    if (m_controller && !m_controller->dragging())
                    {
                        m_dragging = false;
                        m_pressed = false;
                    }
                });
        connect(c, &MascotPetController::packageSourceChanged, this,
                &MascotWindowAdapter::cancelMove);
    }
    emit changed();
}
bool MascotWindowAdapter::canPosition() const
{
    const auto p = QGuiApplication::platformName();
    return p == "xcb" || p == "windows" || p == "cocoa";
}
void MascotWindowAdapter::setInputTransparent(bool value)
{
    if (m_transparent == value)
        return;
    m_transparent = value;
    emit changed();
}
void MascotWindowAdapter::setAlwaysOnTop(bool value)
{
    if (m_above == value)
        return;
    if (value && !canPosition())
    {
        emit platformError("AlwaysOnTopUnavailable",
                           "This session does not provide direct stacking control");
        return;
    }
    m_above = value;
    emit changed();
}
QVariantMap MascotWindowAdapter::capabilities() const
{
    const bool headless = QGuiApplication::platformName() == "offscreen" ||
                          QGuiApplication::platformName() == "minimal";
    QVariantMap result;
    for (const auto &name : {"position", "smoothMovement", "alwaysOnTop", "inputRegions",
                             "inputTransparent", "systemDrag", "skipTaskbar"})
    {
        const bool positioning = QString(name) == "position" || QString(name) == "smoothMovement" ||
                                 QString(name) == "alwaysOnTop" || QString(name) == "skipTaskbar";
        bool unavailable = headless || (positioning && !canPosition());
#if !defined(QCATUIKIT_HAS_XSHAPE)
        if (QString(name) == "skipTaskbar" && QGuiApplication::platformName() == "xcb")
            unavailable = true;
#endif
        if (QString(name) == "inputRegions")
        {
            const auto qpa = QGuiApplication::platformName();
            unavailable = !qpa.startsWith("wayland");
#if defined(QCATUIKIT_HAS_XSHAPE) && QT_CONFIG(xcb)
            if (qpa == "xcb")
            {
                auto *native = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
                unavailable = !native || !shapeApi().available(native->display());
            }
#endif
        }
        result.insert(
            name, QVariantMap{{"status", unavailable ? "Unavailable" : "Unknown"},
                              {"reason",
                               unavailable ? "Not provided directly by this QPA session"
                                           : "Qt request available; desktop acceptance required"}});
    }
    return result;
}
QVariantList MascotWindowAdapter::screens() const
{
    QVariantList result;
    for (auto *s : QGuiApplication::screens())
        result.append(QVariantMap{{"id", screenKey(s)},
                                  {"name", s->name()},
                                  {"availableGeometry", s->availableGeometry()},
                                  {"devicePixelRatio", s->devicePixelRatio()}});
    return result;
}
QScreen *MascotWindowAdapter::findScreen(const QString &id) const
{
    for (auto *s : QGuiApplication::screens())
        if (screenKey(s) == id)
            return s;
    if (m_window && m_window->screen())
        return m_window->screen();
    return QGuiApplication::primaryScreen();
}
QPoint MascotWindowAdapter::bounded(QScreen *s, const QPointF &point) const
{
    if (!s || !m_window || !m_constrain)
        return point.toPoint();
    const auto a = s->availableGeometry();
    return QPoint(
        qBound(a.left(), qRound(point.x()), qMax(a.left(), a.right() + 1 - m_window->width())),
        qBound(a.top(), qRound(point.y()), qMax(a.top(), a.bottom() + 1 - m_window->height())));
}
bool MascotWindowAdapter::setPosition(const QString &screenId, const QPointF &point)
{
    if (m_controller && m_controller->dragging())
    {
        emit platformError("InteractionBusy", "Position request rejected during pointer drag");
        return false;
    }
    m_layoutTimer.stop();
    const auto generation = ++m_serial;
    cancelMove();
    if (generation != m_serial)
        return false;
    if (!m_window || !finite(point) || !canPosition())
    {
        emit platformError("PositionUnavailable",
                           "Position is invalid or unavailable in this session");
        return false;
    }
    auto *s = findScreen(screenId);
    if (!s)
        return false;
    const auto target = bounded(s, s->availableGeometry().topLeft() + point - m_characterOffset);
    m_window->setScreen(s);
    m_window->setPosition(target);
    return m_window->position() == target;
}
QString MascotWindowAdapter::moveTo(const QString &screenId, const QPointF &point, int durationMs)
{
    m_layoutTimer.stop();
    const quint64 generation = ++m_serial;
    const QString id = QString::number(generation);
    if (m_controller && m_controller->dragging())
    {
        QTimer::singleShot(0, this,
                           [this, id]
                           {
                               emit platformError("InteractionBusy",
                                                  "Move request rejected during pointer drag");
                               emit moveEnded(id, "Failed");
                           });
        return id;
    }
    cancelMove();
    if (generation != m_serial)
    {
        QTimer::singleShot(0, this, [this, id] { emit moveEnded(id, "Cancelled"); });
        return id;
    }
    if (!m_window || !m_window->isVisible() || !finite(point) || durationMs < 0 ||
        durationMs > 3600000 || !canPosition() || !findScreen(screenId))
    {
        QTimer::singleShot(0, this,
                           [this, id]
                           {
                               emit platformError("MoveUnavailable",
                                                  "Invalid move or unavailable positioning");
                               emit moveEnded(id, "Failed");
                           });
        return id;
    }
    auto *s = findScreen(screenId);
    m_window->setScreen(s);
    m_move = id;
    m_from = m_window->position();
    m_to = bounded(s, s->availableGeometry().topLeft() + point - m_characterOffset);
    m_duration = qMax(1, durationMs);
    m_clock.start();
    m_timer.start();
    return id;
}
void MascotWindowAdapter::finishMove(const QString &reason)
{
    m_timer.stop();
    const auto id = std::exchange(m_move, {});
    if (!id.isEmpty())
        emit moveEnded(id, reason);
}
void MascotWindowAdapter::cancelMove()
{
    finishMove("Cancelled");
}
bool MascotWindowAdapter::requestKeyboardFocus()
{
    if (!m_window || !m_window->isVisible() || m_transparent || m_bubbleRect.isEmpty())
        return false;
    m_window->setFlag(Qt::WindowDoesNotAcceptFocus, false);
    m_window->requestActivate();
    return true;
}
void MascotWindowAdapter::releaseKeyboardFocus()
{
    if (m_window)
        m_window->setFlag(Qt::WindowDoesNotAcceptFocus, true);
}
void MascotWindowAdapter::applyWindowPolicy()
{
#if defined(QCATUIKIT_HAS_XSHAPE) && QT_CONFIG(xcb)
    if (!m_window || QGuiApplication::platformName() != "xcb")
        return;
    auto *native = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    if (!native)
        return;
    Display *display = native->display();
    const auto window = m_window->winId();
    const Atom state = XInternAtom(display, "_NET_WM_STATE", false);
    const Atom skipTaskbar = XInternAtom(display, "_NET_WM_STATE_SKIP_TASKBAR", false);
    const Atom skipPager = XInternAtom(display, "_NET_WM_STATE_SKIP_PAGER", false);
    if (!m_window->isVisible())
    {
        Atom type = 0;
        int format = 0;
        unsigned long count = 0, remaining = 0;
        unsigned char *data = nullptr;
        QList<Atom> states;
        if (XGetWindowProperty(display, window, state, 0, 1024, false, XA_ATOM, &type, &format,
                               &count, &remaining, &data) == Success &&
            data && type == XA_ATOM && format == 32)
            for (unsigned long i = 0; i < count; ++i)
                states.append(reinterpret_cast<Atom *>(data)[i]);
        if (data)
            XFree(data);
        for (Atom atom : {skipTaskbar, skipPager})
            if (!states.contains(atom))
                states.append(atom);
        XChangeProperty(display, window, state, XA_ATOM, 32, PropModeReplace,
                        reinterpret_cast<const unsigned char *>(states.constData()), states.size());
    }
    else
    {
        XEvent event{};
        event.xclient.type = ClientMessage;
        event.xclient.window = window;
        event.xclient.message_type = state;
        event.xclient.format = 32;
        event.xclient.data.l[0] = 1; // EWMH add state
        event.xclient.data.l[1] = skipTaskbar;
        event.xclient.data.l[2] = skipPager;
        event.xclient.data.l[3] = 1; // normal application request
        XSendEvent(display, DefaultRootWindow(display), false,
                   SubstructureRedirectMask | SubstructureNotifyMask, &event);
    }
    XFlush(display);
#endif
}
void MascotWindowAdapter::refresh()
{
    if (!m_window)
        return;
    if (m_transparent || m_bubbleRect.isEmpty() || !m_window->isVisible())
        releaseKeyboardFocus();
    m_window->setFlag(Qt::WindowTransparentForInput, m_transparent);
    m_window->setFlag(Qt::WindowStaysOnTopHint, m_above);
    applyWindowPolicy();
    if (m_transparent && m_dragging && m_controller)
        m_controller->endDrag();
    if (m_transparent)
    {
        m_dragging = false;
        m_pressed = false;
    }
    applyMask();
}
void MascotWindowAdapter::syncGeometry()
{
    if (m_geometryPending)
        return;
    m_geometryPending = true;
    QTimer::singleShot(
        0, this,
        [this]
        {
            m_geometryPending = false;
            if (!m_window || !m_controller || m_controller->status() != MascotPetController::Ready)
                return;
            const auto size = m_controller->logicalSize();
            const auto available = availableGeometry();
            if (m_constrain && size.width() > 0 && size.height() > 0)
            {
                const qreal maximum = qMin(qreal(available.width()) / size.width(),
                                           qreal(available.height()) / size.height());
                if (maximum > 0 && m_controller->petScale() > maximum)
                {
                    m_controller->setPetScale(maximum);
                    emit platformError("ScaleCorrected",
                                       "Character scale reduced to fit the available screen");
                }
            }
            const auto foot = m_controller->footAnchor() * m_controller->petScale();
            if (m_haveFoot && canPosition())
                m_window->setPosition(m_window->position() + (m_previousFoot - foot).toPoint());
            m_previousFoot = foot;
            m_haveFoot = true;
            applyMask();
        });
}
void MascotWindowAdapter::applyMask()
{
    if (!m_window || !m_controller || QGuiApplication::platformName() == "offscreen" ||
        QGuiApplication::platformName() == "minimal")
        return;
    QRegion input;
    if (!m_regionsEnabled)
        input = QRegion(0, 0, m_window->width(), m_window->height());
    for (const auto &v : m_regionsEnabled ? m_controller->interactionRegions() : QVariantList{})
    {
        const auto r = v.toMap();
        if (!r.value("clickable").toBool() && !r.value("draggable").toBool())
            continue;
        if (r.value("shape").toString() == "polygon")
        {
            QPolygon polygon;
            for (const auto &p : r.value("points").toList())
            {
                const auto xy = p.toList();
                polygon << QPoint(qRound(xy[0].toDouble()), qRound(xy[1].toDouble()));
            }
            input += QRegion(polygon);
        }
        else
        {
            const auto xy = r.value("rect").toList();
            const QRect rect =
                QRectF(xy[0].toDouble(), xy[1].toDouble(), xy[2].toDouble(), xy[3].toDouble())
                    .toAlignedRect();
            input += QRegion(rect, r.value("shape").toString() == "ellipse" ? QRegion::Ellipse
                                                                            : QRegion::Rectangle);
        }
    }
    if (m_regionsEnabled)
        input.translate(m_characterOffset);
    input += QRegion(m_bubbleRect);
    if (QGuiApplication::platformName() == "xcb")
    {
#if defined(QCATUIKIT_HAS_XSHAPE) && QT_CONFIG(xcb)
        auto *native = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
        if (!native || !shapeApi().available(native->display()))
            return;
        QList<XRectangle> rectangles;
        if (!m_transparent)
        {
            const qreal scale = m_window->devicePixelRatio();
            for (const auto &rect : input)
            {
                const QRect pixels = QRectF(rect.x() * scale, rect.y() * scale,
                                            rect.width() * scale, rect.height() * scale)
                                         .toAlignedRect();
                rectangles.append(XRectangle{short(qBound(-32768, pixels.x(), 32767)),
                                             short(qBound(-32768, pixels.y(), 32767)),
                                             ushort(qBound(0, pixels.width(), 65535)),
                                             ushort(qBound(0, pixels.height(), 65535))});
            }
        }
        shapeApi().combine(native->display(), m_window->winId(), 2 /* ShapeInput */, 0, 0,
                           rectangles.data(), int(rectangles.size()), 0 /* ShapeSet */, Unsorted);
        XFlush(native->display());
#endif
        return;
    }
    // Qt Wayland maps this to wl_surface input_region. On xcb/Windows a
    // QWindow mask also clips output, so it must not be used as an input mask.
    if (QGuiApplication::platformName().startsWith("wayland"))
        m_window->setMask(input.isEmpty() ? QRegion(-2, -2, 1, 1) : input);
}
bool MascotWindowAdapter::eventFilter(QObject *object, QEvent *event)
{
    if (object != m_window || !m_controller)
        return false;
    if (event->type() == QEvent::Hide || event->type() == QEvent::WindowDeactivate)
        releaseKeyboardFocus();
    if (event->type() == QEvent::Expose)
    {
        applyWindowPolicy();
        applyMask();
    }
    if (event->type() == QEvent::MouseButtonPress)
    {
        auto *e = static_cast<QMouseEvent *>(event);
        if (e->button() == Qt::LeftButton && !m_transparent && m_controller->dragEnabled() &&
            !m_controller->hitTest(e->position() - m_characterOffset, true).isEmpty())
        {
            m_pressed = true;
            m_pressGlobal = e->globalPosition();
            m_pressLocal = e->position();
            m_pressWindow = m_window->position();
        }
    }
    else if (event->type() == QEvent::MouseMove && m_pressed)
    {
        auto *e = static_cast<QMouseEvent *>(event);
        if (!m_dragging && (e->globalPosition() - m_pressGlobal).manhattanLength() >=
                               QGuiApplication::styleHints()->startDragDistance())
        {
            const bool wasMoving = !m_move.isEmpty();
            m_dragging = m_controller->beginDrag();
            if (m_dragging)
            {
                // Freeze the trajectory at the threshold, rather than jumping
                // back to the window position captured on pointer press.
                if (wasMoving)
                {
                    m_pressWindow = m_window->position();
                    m_pressGlobal = e->globalPosition();
                }
                cancelMove();
                m_layoutTimer.stop();
            }
            if (m_dragging && !canPosition() && !m_window->startSystemMove())
            {
                m_controller->endDrag();
                m_dragging = false;
                m_pressed = false;
                emit platformError("DragUnavailable", "System drag request rejected");
            }
        }
        if (m_dragging && canPosition())
        {
            const auto p = m_pressWindow + (e->globalPosition() - m_pressGlobal).toPoint();
            auto *s = QGuiApplication::screenAt(e->globalPosition().toPoint());
            m_window->setPosition(bounded(s ? s : findScreen({}), p));
        }
        // Continue delivery so Qt Quick can cancel tap gestures at the drag threshold.
    }
    else if (event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::UngrabMouse)
    {
        m_pressed = false;
        if (m_dragging)
        {
            m_dragging = false;
            m_controller->endDrag();
            return false;
        }
    }
    return false;
}
QVariantMap MascotWindowAdapter::exportState() const
{
    if (!m_window || !m_controller)
        return {};
    auto *s = findScreen({});
    return {{"version", 1},
            {"screenId", s ? screenKey(s) : QString()},
            {"position", m_window->position() + m_characterOffset -
                             (s ? s->availableGeometry().topLeft() : QPoint())},
            {"petScale", m_controller->petScale()},
            {"facing", m_controller->facing()},
            {"alwaysOnTop", m_above},
            {"inputTransparent", m_transparent},
            {"inputRegionsEnabled", m_regionsEnabled}};
}
QVariantMap MascotWindowAdapter::restoreState(const QVariantMap &state)
{
    const auto numeric = [](const QVariant &v)
    {
        const auto t = v.metaType().id();
        return t == QMetaType::Double || t == QMetaType::Int || t == QMetaType::UInt ||
               t == QMetaType::LongLong || t == QMetaType::ULongLong;
    };
    const double scale = state.value("petScale").toDouble();
    const auto facing = state.value("facing").toString();
    const auto point = state.value("position").toPointF();
    if (!m_window || !m_controller || m_controller->status() != MascotPetController::Ready ||
        m_controller->dragging() || !numeric(state.value("version")) ||
        state.value("version").toDouble() != 1 || !numeric(state.value("petScale")) ||
        state.value("screenId").metaType().id() != QMetaType::QString ||
        !state.value("position").canConvert<QPointF>() || !finite(point) || !std::isfinite(scale) ||
        scale <= 0 || (facing != "left" && facing != "right") ||
        state.value("alwaysOnTop").metaType().id() != QMetaType::Bool ||
        state.value("inputTransparent").metaType().id() != QMetaType::Bool ||
        (state.contains("inputRegionsEnabled") &&
         state.value("inputRegionsEnabled").metaType().id() != QMetaType::Bool) ||
        !canPosition())
        return {{"ok", false}, {"reason", "InvalidStateOrPositionUnavailable"}};
    // Validate facing without partially applying scale/window state.
    m_controller->setFacing(facing);
    if (m_controller->facing() != facing)
        return {{"ok", false}, {"reason", "FacingUnavailable"}};
    m_controller->setPetScale(scale);
    m_previousFoot = m_controller->footAnchor() * m_controller->petScale();
    m_haveFoot = true;
    m_above = state.value("alwaysOnTop").toBool();
    m_transparent = state.value("inputTransparent").toBool();
    m_regionsEnabled = state.value("inputRegionsEnabled", false).toBool();
    bool corrected = true;
    for (auto *s : QGuiApplication::screens())
        if (screenKey(s) == state.value("screenId").toString())
            corrected = false;
    setPosition(state.value("screenId").toString(), point);
    emit changed();
    return {{"ok", true}, {"corrected", corrected}};
}
} // namespace QCatUiKit
