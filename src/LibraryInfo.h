// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

namespace QCatUiKit
{
// Private, stateless QML adapter. The C++ public API is Version.h.
class LibraryInfo : public QObject
{
    Q_OBJECT
    QML_ELEMENT
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
    explicit LibraryInfo(QObject *parent = nullptr);
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
