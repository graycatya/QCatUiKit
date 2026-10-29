// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QPoint>
#include <QRect>
#include <QSizeF>
#include <QtMath>

namespace QCatUiKit::MascotPrivate
{
// All arguments are Qt logical pixels. Native DPR conversion belongs to input adapters.
inline qreal fittedScale(const QSizeF &logicalSize, const QRect &available, qreal requested)
{
    if (logicalSize.isEmpty() || available.isEmpty())
        return requested;
    return qMin(requested, qMin(available.width() / logicalSize.width(),
                                available.height() / logicalSize.height()));
}

inline QPoint boundedCharacterOrigin(const QRect &available, const QSizeF &characterSize,
                                     const QPoint &characterOffset, const QPointF &windowOrigin)
{
    const QPoint minimum = available.topLeft() - characterOffset;
    const QPoint maximum =
        available.topLeft() +
        QPoint(available.width() -
                   qCeil(qBound(0., characterSize.width(), qreal(available.width()))),
               available.height() -
                   qCeil(qBound(0., characterSize.height(), qreal(available.height())))) -
        characterOffset;
    return {qBound(minimum.x(), qRound(windowOrigin.x()), maximum.x()),
            qBound(minimum.y(), qRound(windowOrigin.y()), maximum.y())};
}
} // namespace QCatUiKit::MascotPrivate
