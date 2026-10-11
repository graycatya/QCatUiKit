// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <QObject>
#include <QTranslator>
#include <QtQml/qqmlregistration.h>
#include <memory>
#include <array>

class QQmlEngine;
class QJSEngine;

class GalleryLanguage : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(QString localeName READ localeName NOTIFY languageChanged)

public:
    explicit GalleryLanguage(QQmlEngine *engine);
    ~GalleryLanguage() override;
    static GalleryLanguage *create(QQmlEngine *engine, QJSEngine *);

    QString language() const { return m_language; }
    QString localeName() const;
    void setLanguage(const QString &language);

signals:
    void languageChanged();

private:
    QQmlEngine *m_engine;
    QString m_language;
    std::unique_ptr<QTranslator> m_translator;
    std::array<QTranslator, 2> m_qtTranslators;
};
