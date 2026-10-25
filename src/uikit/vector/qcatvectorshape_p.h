// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QColor>
#include <QJSValue>
#include <QVariant>
#include <cmath>

namespace QCatUiKit::VectorPrivate
{
inline QVariant plain(const QVariant &value)
{
    return value.metaType() == QMetaType::fromType<QJSValue>() ? value.value<QJSValue>().toVariant()
                                                               : value;
}
inline bool number(const QVariant &value)
{
    bool ok = false;
    const double n = value.toDouble(&ok);
    return ok && std::isfinite(n) && std::abs(n) <= 10000000;
}
inline QColor color(const QVariant &value)
{
    return value.canConvert<QColor>() ? value.value<QColor>() : QColor(value.toString());
}
inline QString fillError(const QVariant &input)
{
    const auto value = plain(input);
    if (!value.isValid())
        return {};
    const auto map = value.toMap();
    if (map.isEmpty())
        return color(value).isValid() ? QString() : QStringLiteral("Invalid fill/stroke color");
    const auto type = map.value("type").toString();
    if (type != "linear" && type != "radial")
        return QStringLiteral("Gradient type must be linear or radial");
    const QStringList keys =
        type == "linear" ? QStringList{"x1", "y1", "x2", "y2"} : QStringList{"cx", "cy", "radius"};
    for (const auto &key : keys)
        if (!number(map.value(key)))
            return QStringLiteral("Gradient requires finite coordinates");
    if (type == "radial" && map.value("radius").toDouble() <= 0)
        return QStringLiteral("Radial gradient radius must be positive");
    const auto stops = plain(map.value("stops")).toList();
    if (stops.size() < 2 || stops.size() > 256)
        return QStringLiteral("Gradient requires 2..256 stops");
    double previous = -1;
    for (const auto &entry : stops)
    {
        const auto stop = plain(entry).toMap();
        const auto offset = stop.value("offset");
        if (!number(offset) || offset.toDouble() < previous || offset.toDouble() > 1 ||
            offset.toDouble() < 0 || !color(stop.value("color")).isValid())
            return QStringLiteral(
                "Gradient stops require ordered offsets in [0,1] and valid colors");
        previous = offset.toDouble();
    }
    return {};
}
inline QString shapesError(const QVariantList &shapes)
{
    if (shapes.size() > 4096)
        return QStringLiteral("Canvas supports at most 4096 shapes");
    for (const auto &entry : shapes)
    {
        const auto shape = plain(entry).toMap();
        const auto type = shape.value("type").toString();
        if (type != "rectangle" && type != "ellipse" && type != "path")
            return QStringLiteral("Shape type must be rectangle, ellipse or path");
        if (type != "path")
        {
            for (const auto &key : {"x", "y", "width", "height"})
                if (!number(shape.value(key, 0)))
                    return QStringLiteral("Shape coordinates must be finite");
            if (shape.value("width").toDouble() <= 0 || shape.value("height").toDouble() <= 0)
                return QStringLiteral("Shape width and height must be positive");
        }
        else
        {
            const auto commands = plain(shape.value("commands")).toList();
            if (commands.isEmpty() || commands.size() > 65536)
                return QStringLiteral("Path requires 1..65536 commands");
            bool moved = false;
            for (const auto &input : commands)
            {
                const auto command = plain(input).toList();
                const auto op = command.value(0).toString();
                const int count = op == "M" || op == "L" ? 3 : op == "C" ? 7 : op == "Z" ? 1 : 0;
                if (!count || command.size() != count || (!moved && op != "M"))
                    return QStringLiteral(
                        "Path uses M(x,y), L(x,y), C(x1,y1,x2,y2,x,y), Z; start with M");
                for (int i = 1; i < count; ++i)
                    if (!number(command[i]))
                        return QStringLiteral("Path coordinates must be finite");
                moved = true;
            }
        }
        for (const auto &key : {"fill", "stroke"})
        {
            if (shape.contains(key) && !plain(shape.value(key)).isValid())
                return QStringLiteral("Explicit fill/stroke must be a color or gradient");
            const auto error = fillError(shape.value(key));
            if (!error.isEmpty())
                return error;
        }
        for (const auto &key : {"radius", "strokeWidth", "opacity"})
            if (shape.contains(key) &&
                (!number(shape.value(key)) || shape.value(key).toDouble() < 0))
                return QStringLiteral("Style numbers must be finite and nonnegative");
        if (shape.contains("opacity") && shape.value("opacity").toDouble() > 1)
            return QStringLiteral("Opacity must be in [0,1]");
        if (shape.contains("fillRule") && shape.value("fillRule") != "nonzero" &&
            shape.value("fillRule") != "evenodd")
            return QStringLiteral("Fill rule must be nonzero or evenodd");
    }
    return {};
}
} // namespace QCatUiKit::VectorPrivate
