// SPDX-License-Identifier: Apache-2.0
#include "qcatdockcontroller.h"
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRectF>
#include <QScreen>
#include <QSet>
#include <QUuid>
#include <algorithm>
#include <cmath>
#include <functional>
#include <vector>

namespace QCatUiKit
{
namespace
{
struct Node
{
    QString key = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QStringList tabs;
    QString active;
    bool horizontal = true;
    double ratio = 0.5;
    std::unique_ptr<Node> first, second;
    bool split() const
    {
        return bool(first);
    }
};
Node *findPanel(Node *n, const QString &id)
{
    if (!n)
        return nullptr;
    if (n->tabs.contains(id))
        return n;
    if (auto *p = findPanel(n->first.get(), id))
        return p;
    return findPanel(n->second.get(), id);
}
std::unique_ptr<Node> *findSlot(std::unique_ptr<Node> &n, const QString &key)
{
    if (!n)
        return nullptr;
    if (n->key == key)
        return &n;
    if (auto *p = findSlot(n->first, key))
        return p;
    return findSlot(n->second, key);
}
void removePanel(std::unique_ptr<Node> &n, const QString &id)
{
    if (!n)
        return;
    if (n->split())
    {
        removePanel(n->first, id);
        removePanel(n->second, id);
        if (!n->first)
        {
            auto remaining = std::move(n->second);
            n = std::move(remaining);
        }
        else if (!n->second)
        {
            auto remaining = std::move(n->first);
            n = std::move(remaining);
        }
    }
    else
    {
        const int index = n->tabs.indexOf(id);
        if (index < 0)
            return;
        n->tabs.removeAt(index);
        if (n->tabs.isEmpty())
            n.reset();
        else if (n->active == id)
            n->active = n->tabs.at(std::min(index, int(n->tabs.size()) - 1));
    }
}
QVariantMap encode(const Node *n)
{
    if (!n)
        return {};
    QVariantMap v{{"id", n->key}};
    if (n->split())
    {
        v.insert("kind", "split");
        v.insert("orientation", n->horizontal ? "horizontal" : "vertical");
        v.insert("ratio", n->ratio);
        v.insert("first", encode(n->first.get()));
        v.insert("second", encode(n->second.get()));
    }
    else
    {
        v.insert("kind", "tabs");
        v.insert("tabs", n->tabs);
        v.insert("active", n->active);
    }
    return v;
}
struct Floating
{
    std::unique_ptr<Node> group;
    QRectF geometry{100, 100, 480, 320};
    QString target;
    DockController::Position position = DockController::Center;
    int index = 0;
    double ratio = 0.5;
};
Node *firstGroup(Node *n)
{
    while (n && n->split())
        n = n->first.get();
    return n;
}
bool captureReturn(Node *n, const QString &key, Floating &f)
{
    if (!n || !n->split())
        return false;
    if (n->first->key == key || n->second->key == key)
    {
        const bool first = n->first->key == key;
        f.target = firstGroup(first ? n->second.get() : n->first.get())->active;
        f.position = n->horizontal ? (first ? DockController::Left : DockController::Right)
                                   : (first ? DockController::Top : DockController::Bottom);
        f.ratio = n->ratio;
        return true;
    }
    return captureReturn(n->first.get(), key, f) || captureReturn(n->second.get(), key, f);
}
bool validGeometry(const QRectF &r)
{
    return std::isfinite(r.x()) && std::isfinite(r.y()) && std::abs(r.x()) < 1e7 &&
           std::abs(r.y()) < 1e7 && std::isfinite(r.width()) && std::isfinite(r.height()) &&
           r.width() >= 1 && r.height() >= 1 && r.width() <= 32768 && r.height() <= 32768;
}
QRectF onScreen(QRectF r)
{
    if (!qobject_cast<QGuiApplication *>(QCoreApplication::instance()))
        return r;
    const auto screens = QGuiApplication::screens();
    if (screens.isEmpty())
        return r;
    QScreen *best = QGuiApplication::primaryScreen();
    if (!best)
        best = screens.first();
    double area = 0;
    for (auto *screen : screens)
    {
        const auto intersection = r.intersected(screen->availableGeometry());
        const double a = intersection.width() * intersection.height();
        if (a > area)
        {
            area = a;
            best = screen;
        }
    }
    const QRectF available = best->availableGeometry();
    r.setSize(
        QSizeF(std::min(r.width(), available.width()), std::min(r.height(), available.height())));
    r.moveLeft(std::clamp(r.x(), available.left(), available.right() - r.width()));
    r.moveTop(std::clamp(r.y(), available.top(), available.bottom() - r.height()));
    return r;
}
} // namespace
class DockController::Impl
{
  public:
    QStringList registered;
    std::unique_ptr<Node> root;
    std::vector<Floating> floating;
    Node *find(const QString &id)
    {
        if (auto *n = findPanel(root.get(), id))
            return n;
        for (auto &f : floating)
            if (f.group->tabs.contains(id))
                return f.group.get();
        return nullptr;
    }
    void remove(const QString &id)
    {
        removePanel(root, id);
        for (auto &f : floating)
            removePanel(f.group, id);
        floating.erase(std::remove_if(floating.begin(), floating.end(),
                                      [](const auto &f) { return !f.group; }),
                       floating.end());
    }
};
DockController::DockController(QObject *parent) : QObject(parent), d(std::make_unique<Impl>())
{
    if (auto *app = qobject_cast<QGuiApplication *>(QCoreApplication::instance()))
    {
        auto watch = [this](QScreen *screen)
        {
            connect(screen, &QScreen::availableGeometryChanged, this,
                    &DockController::constrainFloatingWindows);
        };
        for (auto *screen : app->screens())
            watch(screen);
        connect(app, &QGuiApplication::screenAdded, this,
                [this, watch](QScreen *screen)
                {
                    watch(screen);
                    constrainFloatingWindows();
                });
        connect(app, &QGuiApplication::screenRemoved, this,
                &DockController::constrainFloatingWindows);
    }
}
DockController::~DockController() = default;
QVariantMap DockController::layout() const
{
    return encode(d->root.get());
}
QStringList DockController::panelIds() const
{
    return d->registered;
}
bool DockController::reject(const QString &reason)
{
    emit operationRejected(reason);
    return false;
}
bool DockController::registerPanel(const QString &id)
{
    if (id.trimmed().isEmpty() || d->registered.contains(id))
        return reject(QStringLiteral("Invalid or duplicate panel ID"));
    d->registered.append(id);
    if (!d->root)
        d->root = std::make_unique<Node>();
    Node *n = d->root.get();
    while (n->split())
        n = n->first.get();
    n->tabs.append(id);
    n->active = id;
    emit panelsChanged();
    emit layoutChanged();
    return true;
}
bool DockController::unregisterPanel(const QString &id)
{
    if (!d->registered.removeOne(id))
        return reject(QStringLiteral("Unknown panel ID"));
    d->remove(id);
    emit panelsChanged();
    emit layoutChanged();
    return true;
}
bool DockController::dock(const QString &id, const QString &target, Position position)
{
    if (!d->registered.contains(id) || !d->registered.contains(target) || id == target ||
        position < Center || position > Bottom)
        return reject(QStringLiteral("Invalid dock operation"));
    auto *dest = findPanel(d->root.get(), target);
    if (!dest)
        return reject(QStringLiteral("Target panel must be docked"));
    const QString key = dest->key;
    // Target is a different panel, so its group survives removal.
    d->remove(id);
    auto *slot = findSlot(d->root, key);
    if (position == Center)
    {
        (*slot)->tabs.append(id);
        (*slot)->active = id;
    }
    else
    {
        auto split = std::make_unique<Node>();
        split->horizontal = position == Left || position == Right;
        auto panel = std::make_unique<Node>();
        panel->tabs.append(id);
        panel->active = id;
        const bool before = position == Left || position == Top;
        split->first = before ? std::move(panel) : std::move(*slot);
        split->second = before ? std::move(*slot) : std::move(panel);
        *slot = std::move(split);
    }
    emit layoutChanged();
    return true;
}
bool DockController::closePanel(const QString &id)
{
    if (!d->find(id))
        return reject(QStringLiteral("Panel is not open"));
    d->remove(id);
    emit layoutChanged();
    return true;
}
bool DockController::openPanel(const QString &id)
{
    if (!d->registered.contains(id))
        return reject(QStringLiteral("Unknown panel ID"));
    if (d->find(id))
        return activatePanel(id);
    if (!d->root)
        d->root = std::make_unique<Node>();
    Node *n = d->root.get();
    while (n->split())
        n = n->first.get();
    n->tabs.append(id);
    n->active = id;
    emit layoutChanged();
    return true;
}
bool DockController::activatePanel(const QString &id)
{
    auto *n = d->find(id);
    if (!n)
        return reject(QStringLiteral("Panel is not open"));
    if (n->active != id)
    {
        n->active = id;
        emit layoutChanged();
    }
    return true;
}
bool DockController::moveTab(const QString &id, int index)
{
    auto *n = d->find(id);
    if (!n || index < 0 || index >= n->tabs.size())
        return reject(QStringLiteral("Invalid tab index"));
    const int old = n->tabs.indexOf(id);
    if (old != index)
    {
        n->tabs.move(old, index);
        emit layoutChanged();
    }
    return true;
}
bool DockController::setSplitRatio(const QString &nodeId, double ratio)
{
    auto *slot = findSlot(d->root, nodeId);
    if (!slot || !(*slot)->split() || !std::isfinite(ratio) || ratio <= 0 || ratio >= 1)
        return reject(QStringLiteral("Invalid split ratio"));
    if ((*slot)->ratio != ratio)
    {
        (*slot)->ratio = ratio;
        emit layoutChanged();
    }
    return true;
}
QVariantList DockController::floatingWindows() const
{
    QVariantList result;
    for (const auto &f : d->floating)
    {
        auto v = encode(f.group.get());
        v.insert("geometry", QVariantMap{{"x", f.geometry.x()},
                                         {"y", f.geometry.y()},
                                         {"width", f.geometry.width()},
                                         {"height", f.geometry.height()}});
        v.insert("returnTarget", f.target);
        v.insert("returnPosition", int(f.position));
        v.insert("returnIndex", f.index);
        v.insert("returnRatio", f.ratio);
        result.append(v);
    }
    return result;
}
QString DockController::floatingWindowForPanel(const QString &id) const
{
    for (const auto &f : d->floating)
        if (f.group->tabs.contains(id))
            return f.group->key;
    return {};
}
bool DockController::floatPanel(const QString &id, bool wholeGroup)
{
    auto *source = findPanel(d->root.get(), id);
    if (!source)
        return reject(QStringLiteral("Panel must be docked before floating"));
    Floating f;
    f.group = std::make_unique<Node>();
    f.group->tabs = wholeGroup ? source->tabs : QStringList{id};
    f.group->active = wholeGroup ? source->active : id;
    f.geometry = onScreen(f.geometry);
    if (!wholeGroup && source->tabs.size() > 1)
    {
        f.index = source->tabs.indexOf(id);
        f.target = source->tabs.at(f.index == 0 ? 1 : 0);
    }
    else
        captureReturn(d->root.get(), source->key, f);
    for (const auto &panel : f.group->tabs)
        removePanel(d->root, panel);
    d->floating.push_back(std::move(f));
    emit layoutChanged();
    return true;
}
bool DockController::dockBack(const QString &windowId)
{
    const auto it = std::find_if(d->floating.begin(), d->floating.end(),
                                 [&](const auto &f) { return f.group->key == windowId; });
    if (it == d->floating.end())
        return reject(QStringLiteral("Unknown floating window"));
    Floating f = std::move(*it);
    d->floating.erase(it);
    Node *target = findPanel(d->root.get(), f.target);
    if (!target)
    {
        target = firstGroup(d->root.get());
        f.position = Center;
        f.index = target ? target->tabs.size() : 0;
    }
    if (!target)
        d->root = std::move(f.group);
    else if (f.position == Center)
    {
        int at = std::clamp(f.index, 0, int(target->tabs.size()));
        for (const auto &id : f.group->tabs)
            target->tabs.insert(at++, id);
        target->active = f.group->active;
    }
    else
    {
        auto *slot = findSlot(d->root, target->key);
        auto split = std::make_unique<Node>();
        split->horizontal = f.position == Left || f.position == Right;
        split->ratio = f.ratio;
        const bool before = f.position == Left || f.position == Top;
        split->first = before ? std::move(f.group) : std::move(*slot);
        split->second = before ? std::move(*slot) : std::move(f.group);
        *slot = std::move(split);
    }
    emit layoutChanged();
    return true;
}
bool DockController::setFloatingGeometry(const QString &windowId, double x, double y, double width,
                                         double height)
{
    const QRectF r(x, y, width, height);
    if (!validGeometry(r))
        return reject(QStringLiteral("Invalid floating window geometry"));
    for (auto &f : d->floating)
        if (f.group->key == windowId)
        {
            if (f.geometry != r)
            {
                f.geometry = r;
                emit layoutChanged();
            }
            return true;
        }
    return reject(QStringLiteral("Unknown floating window"));
}
void DockController::constrainFloatingWindows()
{
    bool changed = false;
    for (auto &f : d->floating)
    {
        const auto r = onScreen(f.geometry);
        if (r != f.geometry)
        {
            f.geometry = r;
            changed = true;
        }
    }
    if (changed)
        emit layoutChanged();
}
QString DockController::saveLayout() const
{
    return QString::fromUtf8(
        QJsonDocument(QJsonObject::fromVariantMap(
                          {{"version", 2}, {"root", layout()}, {"floating", floatingWindows()}}))
            .toJson(QJsonDocument::Compact));
}
bool DockController::restoreLayout(const QString &json)
{
    if (json.size() > 1024 * 1024)
        return reject(QStringLiteral("Layout exceeds size limit"));
    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(json.toUtf8(), &error);
    const auto top = doc.object();
    if (error.error != QJsonParseError::NoError || !doc.isObject() ||
        (top.value("version").toDouble(-1) != 1 && top.value("version").toDouble(-1) != 2) ||
        !top.value("root").isObject())
        return reject(QStringLiteral("Invalid layout document or version"));
    bool valid = true;
    QSet<QString> keys, panels;
    int count = 0;
    std::function<std::unique_ptr<Node>(QJsonObject, int)> parse =
        [&](QJsonObject v, int depth) -> std::unique_ptr<Node>
    {
        if (++count > 4096 || depth > 64)
        {
            valid = false;
            return {};
        }
        const QString key = v.value("id").toString();
        if (key.isEmpty() || keys.contains(key))
        {
            valid = false;
            return {};
        }
        keys.insert(key);
        auto n = std::make_unique<Node>();
        n->key = key;
        if (v.value("kind") == "split")
        {
            const QString orientation = v.value("orientation").toString();
            n->ratio = v.value("ratio").toDouble(-1);
            if ((orientation != "horizontal" && orientation != "vertical") ||
                !std::isfinite(n->ratio) || n->ratio <= 0 || n->ratio >= 1 ||
                !v.value("first").isObject() || !v.value("second").isObject())
            {
                valid = false;
                return {};
            }
            n->horizontal = orientation == "horizontal";
            n->first = parse(v.value("first").toObject(), depth + 1);
            n->second = parse(v.value("second").toObject(), depth + 1);
            if (!n->first)
                return std::move(n->second);
            if (!n->second)
                return std::move(n->first);
        }
        else if (v.value("kind") == "tabs")
        {
            if (!v.value("tabs").isArray() || !v.value("active").isString())
            {
                valid = false;
                return {};
            }
            const auto values = v.value("tabs").toVariant().toList();
            QStringList all;
            for (const auto &value : values)
            {
                const auto id = value.toString();
                if (value.metaType().id() != QMetaType::QString || id.trimmed().isEmpty() ||
                    panels.contains(id))
                {
                    valid = false;
                    return {};
                }
                panels.insert(id);
                all.append(id);
                if (d->registered.contains(id))
                    n->tabs.append(id);
            }
            const auto active = v.value("active").toString();
            if (all.isEmpty() || !all.contains(active))
            {
                valid = false;
                return {};
            }
            if (n->tabs.isEmpty())
                return {};
            n->active = n->tabs.contains(active) ? active : n->tabs.first();
        }
        else
        {
            valid = false;
            return {};
        }
        return n;
    };
    const auto root = top.value("root").toObject();
    auto candidate = root.isEmpty() ? std::unique_ptr<Node>() : parse(root, 0);
    std::vector<Floating> floating;
    if (top.value("version").toInt() == 2)
    {
        if (!top.value("floating").isArray())
            valid = false;
        for (const auto &entry : top.value("floating").toArray())
        {
            if (!entry.isObject())
            {
                valid = false;
                break;
            }
            const auto v = entry.toObject();
            if (v.value("kind") != "tabs")
            {
                valid = false;
                break;
            }
            Floating f;
            f.group = parse(v, 0);
            const auto g = v.value("geometry").toObject();
            for (const auto &key : {"x", "y", "width", "height"})
                if (!g.value(key).isDouble())
                    valid = false;
            f.geometry = QRectF(g.value("x").toDouble(), g.value("y").toDouble(),
                                g.value("width").toDouble(), g.value("height").toDouble());
            if (!validGeometry(f.geometry) || !v.value("returnTarget").isString())
                valid = false;
            f.target = v.value("returnTarget").toString();
            const double position = v.value("returnPosition").toDouble(-1);
            const double index = v.value("returnIndex").toDouble(-1);
            f.ratio = v.value("returnRatio").toDouble(-1);
            if (position < Center || position > Bottom || position != std::floor(position) ||
                index < 0 || index > 4096 || index != std::floor(index) ||
                !std::isfinite(f.ratio) || f.ratio <= 0 || f.ratio >= 1)
                valid = false;
            if (!valid)
                break;
            f.position = Position(int(position));
            f.index = int(index);
            f.geometry = onScreen(f.geometry);
            if (f.group)
                floating.push_back(std::move(f));
        }
    }
    if (!valid)
        return reject(QStringLiteral("Invalid layout tree"));
    d->root = std::move(candidate);
    d->floating = std::move(floating);
    emit layoutChanged();
    return true;
}
} // namespace QCatUiKit
