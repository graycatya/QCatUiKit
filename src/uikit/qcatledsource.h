// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "qcatuikitglobal.h"
#include <QByteArray>
#include <QObject>
#include <QVector>
#include <QtQml/qqmlregistration.h>
#include <memory>

struct QCatLedSnapshot;
class QCatLedPreviewRenderer;

class QCATUIKIT_EXPORT QCatLedSource : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LedSource)
    Q_PROPERTY(int pointCount READ pointCount NOTIFY dataChanged FINAL)
    Q_PROPERTY(quint64 frameId READ frameId NOTIFY dataChanged FINAL)
    Q_PROPERTY(quint64 layoutId READ layoutId NOTIFY dataChanged FINAL)
    Q_PROPERTY(QString lastError READ lastError NOTIFY dataChanged FINAL)
  public:
    struct Position
    {
        float x, y, z;
    };
    explicit QCatLedSource(QObject *parent = nullptr);
    ~QCatLedSource() override;

    // Thread-safe. Returns 0 on invalid coordinates. Empty layouts are valid.
    quint64 setLayout(const QVector<Position> &positions);
    // RGBA byte order, exactly 4 * pointCount bytes. IDs must increase per layout.
    // Pass the setLayout return value to reject frames for an obsolete layout.
    bool submitFrame(const QByteArray &rgba, quint64 frameId, quint64 layoutId = 0);
    int pointCount() const;
    quint64 frameId() const;
    quint64 layoutId() const;
    quint64 submittedFrames() const;
    QString lastError() const;
  Q_SIGNALS:
    // Coalesced and emitted on this object's thread; never one event per frame.
    void dataChanged();

  private:
    void scheduleNotification();
    std::shared_ptr<const QCatLedSnapshot> snapshot() const;
    struct Impl;
    const std::unique_ptr<Impl> d;
    friend class QCatLedPreviewRenderer;
};
