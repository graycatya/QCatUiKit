// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatledsource.h"
#include <QVector3D>

struct QCatLedLayout
{
    QVector<QCatLedSource::Position> positions;
    QVector3D center;
    float radius = 1;
    quint64 id = 0;
};
struct QCatLedSnapshot
{
    std::shared_ptr<const QCatLedLayout> layout;
    QByteArray rgba;
    quint64 frameId = 0;
};
