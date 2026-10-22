// SPDX-License-Identifier: Apache-2.0
#include "qcatdocknativedrag_p.h"
#include <QDataStream>
#include <QDrag>
#include <QDropEvent>
#include <QGuiApplication>
#include <QIODevice>
#include <QLibrary>
#include <QMimeData>
#include <QPainter>
#include <QQuickItem>
#include <QQuickWindow>
#include <QUuid>

#if defined(Q_OS_LINUX) && defined(QCATUIKIT_HAS_WAYLAND_CLIENT)
#include <QtGui/qguiapplication_platform.h>
#if QT_CONFIG(wayland)
#include <wayland-client.h>
#endif
#endif

namespace QCatUiKit
{
namespace
{
constexpr auto panelMime = "application/x-qcatuikit-dock-panel";

bool hasToplevelDrag()
{
#if defined(Q_OS_LINUX) && defined(QCATUIKIT_HAS_WAYLAND_CLIENT)
#if QT_CONFIG(wayland)
    auto *native = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    if (!native)
        return false;
    // Resolve the transport only in a Wayland process. An XCB-only deployment
    // must not gain a mandatory libwayland-client loader dependency.
    QLibrary library(QStringLiteral("wayland-client"), 0);
#define RESOLVE_WAYLAND(name)                                                                      \
    auto name##Fn = reinterpret_cast<decltype(&name)>(library.resolve(#name))
    RESOLVE_WAYLAND(wl_display_create_queue);
    RESOLVE_WAYLAND(wl_proxy_create_wrapper);
    RESOLVE_WAYLAND(wl_proxy_set_queue);
    RESOLVE_WAYLAND(wl_proxy_wrapper_destroy);
    RESOLVE_WAYLAND(wl_proxy_marshal_flags);
    RESOLVE_WAYLAND(wl_proxy_get_version);
    RESOLVE_WAYLAND(wl_proxy_add_listener);
    RESOLVE_WAYLAND(wl_proxy_destroy);
    RESOLVE_WAYLAND(wl_display_roundtrip_queue);
    RESOLVE_WAYLAND(wl_event_queue_destroy);
#undef RESOLVE_WAYLAND
    const auto *registryInterface =
        reinterpret_cast<const wl_interface *>(library.resolve("wl_registry_interface"));
    if (!wl_display_create_queueFn || !wl_proxy_create_wrapperFn || !wl_proxy_set_queueFn ||
        !wl_proxy_wrapper_destroyFn || !wl_proxy_marshal_flagsFn || !wl_proxy_get_versionFn ||
        !wl_proxy_add_listenerFn || !wl_proxy_destroyFn || !wl_display_roundtrip_queueFn ||
        !wl_event_queue_destroyFn || !registryInterface)
        return false;
    auto *display = native->display();
    auto *queue = wl_display_create_queueFn(display);
    auto *wrapper = static_cast<wl_proxy *>(wl_proxy_create_wrapperFn(display));
    if (!queue || !wrapper)
    {
        if (wrapper)
            wl_proxy_wrapper_destroyFn(wrapper);
        if (queue)
            wl_event_queue_destroyFn(queue);
        return false;
    }
    wl_proxy_set_queueFn(wrapper, queue);
    auto *registry = wl_proxy_marshal_flagsFn(wrapper, WL_DISPLAY_GET_REGISTRY, registryInterface,
                                              wl_proxy_get_versionFn(wrapper), 0, nullptr);
    wl_proxy_wrapper_destroyFn(wrapper);
    if (!registry)
    {
        wl_event_queue_destroyFn(queue);
        return false;
    }
    bool found = false;
    static const wl_registry_listener listener = {
        [](void *data, wl_registry *, uint32_t, const char *interface, uint32_t)
        {
            if (qstrcmp(interface, "xdg_toplevel_drag_manager_v1") == 0)
                *static_cast<bool *>(data) = true;
        },
        [](void *, wl_registry *, uint32_t) {}};
    wl_proxy_add_listenerFn(
        registry, reinterpret_cast<void (**)(void)>(const_cast<wl_registry_listener *>(&listener)),
        &found);
    const bool ok = wl_display_roundtrip_queueFn(display, queue) >= 0;
    wl_proxy_destroyFn(registry);
    wl_event_queue_destroyFn(queue);
    return ok && found;
#endif
#endif
    return false;
}
} // namespace

DockNativeDrag::DockNativeDrag(QObject *parent)
    : QObject(parent),
      m_enabled(QGuiApplication::platformName().startsWith(QLatin1String("wayland"))),
      m_toplevelDragSupported(m_enabled && hasToplevelDrag()),
      m_token(QUuid::createUuid().toByteArray())
{
}

DockNativeDrag::~DockNativeDrag()
{
    cancel();
}

bool DockNativeDrag::enabled() const
{
    return m_enabled;
}
bool DockNativeDrag::toplevelDragSupported() const
{
    return m_toplevelDragSupported;
}
bool DockNativeDrag::active() const
{
    return m_active;
}

void DockNativeDrag::watchWindow(QQuickWindow *window, const QString &id)
{
    if (!m_enabled || !window)
        return;
    if (id.isEmpty())
    {
        for (auto it = m_windows.begin(); it != m_windows.end();)
        {
            if (it.value().isEmpty() && it.key() != window)
            {
                it.key()->removeEventFilter(this);
                it = m_windows.erase(it);
            }
            else
                ++it;
        }
    }
    if (!m_windows.contains(window))
    {
        window->installEventFilter(this);
        connect(window, &QObject::destroyed, this, [this, window] { m_windows.remove(window); });
    }
    m_windows.insert(window, id);
}

bool DockNativeDrag::prepare(QQuickItem *panel, const QString &title, const QColor &surface,
                             const QColor &text, const QColor &border, int fontSize)
{
    if (!m_enabled || m_active || !panel || !panel->window())
        return false;
    m_pixmap = QPixmap();
    // Capture synchronously before detaching/reparenting the panel. Waiting for
    // grabToImage() after starting a native drag loses its initial drag icon.
    const auto screenshot = panel->window()->grabWindow();
    const auto rect = panel->mapRectToScene(panel->boundingRect());
    const qreal scale = screenshot.width() / qreal(qMax(1, panel->window()->width()));
    const QRect crop = QRectF(rect.topLeft() * scale, rect.size() * scale)
                           .toAlignedRect()
                           .intersected(screenshot.rect());
    const auto image = crop.isEmpty() || !panel->isVisible() ? QImage() : screenshot.copy(crop);
    const qreal dpr = panel->window()->devicePixelRatio();
    const int width = 280;
    const int bodyHeight =
        image.isNull() ? 120 : qBound(80, qRound(264.0 * image.height() / image.width()), 200);
    m_pixmap = QPixmap(qRound(width * dpr), qRound((bodyHeight + 42) * dpr));
    m_pixmap.setDevicePixelRatio(dpr);
    m_pixmap.fill(Qt::transparent);
    QPainter painter(&m_pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(border, 2));
    painter.setBrush(surface);
    painter.drawRoundedRect(QRectF(1, 1, width - 2, bodyHeight + 40), 5, 5);
    auto font = painter.font();
    font.setPixelSize(fontSize);
    painter.setFont(font);
    painter.setPen(text);
    painter.drawText(QRect(10, 2, width - 20, 30), Qt::AlignVCenter,
                     painter.fontMetrics().elidedText(title, Qt::ElideRight, width - 20));
    if (!image.isNull())
        painter.drawImage(QRect(8, 34, width - 16, bodyHeight), image);
    return !image.isNull();
}

void DockNativeDrag::start(QQuickWindow *origin, QQuickWindow *floating, const QPoint &offset)
{
    if (!m_enabled || m_active || !origin || m_pixmap.isNull())
        return;
    QDrag drag(qGuiApp);
    auto *mime = new QMimeData;
    mime->setData(QLatin1String(panelMime), m_token);
    if (m_toplevelDragSupported && floating)
    {
        // Qt's Wayland dock-window bridge (Qt 6.8+). Keep this private MIME
        // contract isolated here; no QtWayland private headers are linked.
        floating->create();
        QByteArray windowData, offsetData;
        QDataStream windowStream(&windowData, QIODevice::WriteOnly);
        QDataStream offsetStream(&offsetData, QIODevice::WriteOnly);
        windowStream << qintptr(floating);
        offsetStream << offset;
        mime->setData(QStringLiteral("application/x-qt-mainwindowdrag-window"), windowData);
        mime->setData(QStringLiteral("application/x-qt-mainwindowdrag-position"), offsetData);
    }
    drag.setMimeData(mime);
    drag.setPixmap(m_pixmap);
    drag.setHotSpot(QPoint(-20, -24));
    QPointer<DockNativeDrag> guard(this);
    connect(this, &QObject::destroyed, &drag, [] { QDrag::cancel(); });
    connect(origin, &QObject::destroyed, &drag, [] { QDrag::cancel(); });
    m_active = true;
    m_acceptsDrop = false;
    emit activeChanged();
    if (!guard)
        return;
    const auto result = drag.exec(Qt::MoveAction, Qt::MoveAction);
    if (!guard)
        return;
    m_active = false;
    emit activeChanged();
    if (guard)
        emit finished(result == Qt::MoveAction);
}

bool DockNativeDrag::moveWindow(QQuickWindow *window)
{
    return m_enabled && window && window->startSystemMove();
}

void DockNativeDrag::cancel()
{
    if (m_active)
        QDrag::cancel();
}

bool DockNativeDrag::eventFilter(QObject *object, QEvent *event)
{
    if (!m_enabled || !m_active || !m_windows.contains(object))
        return false;
    if (event->type() == QEvent::DragLeave)
    {
        m_acceptsDrop = false;
        emit left();
        return true;
    }
    if (event->type() != QEvent::DragEnter && event->type() != QEvent::DragMove &&
        event->type() != QEvent::Drop)
        return false;
    auto *drop = static_cast<QDropEvent *>(event);
    if (drop->mimeData()->data(QLatin1String(panelMime)) != m_token)
        return false;
    m_acceptsDrop = false;
    QPointer<DockNativeDrag> guard(this);
    emit moved(m_windows.value(object), drop->position());
    if (!guard)
        return true;
    // Accept entry even over a currently invalid area, so subsequent native
    // motion reaches the workspace. Only a valid destination accepts a drop.
    if (event->type() == QEvent::DragEnter || m_acceptsDrop)
    {
        drop->setDropAction(Qt::MoveAction);
        drop->accept();
    }
    else
    {
        drop->ignore();
    }
    if (event->type() == QEvent::Drop && m_acceptsDrop)
        emit dropped();
    return true;
}
} // namespace QCatUiKit
