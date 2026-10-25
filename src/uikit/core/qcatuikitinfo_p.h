// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

namespace QCatUiKit
{
// Private C++ implementation of the public QCatUiKit QML singleton.
// The C++ public API is qcatuikitversion.h.
class QCatUiKitInfo : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(QCatUiKit)
    QML_SINGLETON
    Q_PROPERTY(QString version READ version CONSTANT FINAL)
    Q_PROPERTY(int versionMajor READ versionMajor CONSTANT FINAL)
    Q_PROPERTY(int versionMinor READ versionMinor CONSTANT FINAL)
    Q_PROPERTY(int versionPatch READ versionPatch CONSTANT FINAL)
    Q_PROPERTY(int abiVersion READ abiVersion CONSTANT FINAL)
    Q_PROPERTY(QString releaseChannel READ releaseChannel CONSTANT FINAL)
    Q_PROPERTY(bool isStable READ isStable CONSTANT FINAL)
    Q_PROPERTY(bool isExperimental READ isExperimental CONSTANT FINAL)

  public:
    explicit QCatUiKitInfo(QObject *parent = nullptr);
    QString version() const;
    int versionMajor() const;
    int versionMinor() const;
    int versionPatch() const;
    int abiVersion() const;
    QString releaseChannel() const;
    bool isStable() const;
    bool isExperimental() const;
};
} // namespace QCatUiKit
