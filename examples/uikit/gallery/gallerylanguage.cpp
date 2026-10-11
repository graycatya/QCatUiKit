// SPDX-License-Identifier: Apache-2.0
#include "gallerylanguage.h"

#include <QCoreApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QQmlEngine>

GalleryLanguage::GalleryLanguage(QQmlEngine *engine) : QObject(engine), m_engine(engine)
{
    setLanguage(QLocale().language() == QLocale::Chinese ? QStringLiteral("zh_CN")
                                                       : QStringLiteral("en"));
}

GalleryLanguage::~GalleryLanguage()
{
    if (m_translator)
        QCoreApplication::removeTranslator(m_translator.get());
    for (auto &translator : m_qtTranslators)
        QCoreApplication::removeTranslator(&translator);
}

GalleryLanguage *GalleryLanguage::create(QQmlEngine *engine, QJSEngine *)
{
    return new GalleryLanguage(engine);
}

QString GalleryLanguage::localeName() const
{
    return m_language == QStringLiteral("zh_CN") ? QStringLiteral("zh_CN")
                                                : QStringLiteral("en_US");
}

void GalleryLanguage::setLanguage(const QString &language)
{
    if ((language != QStringLiteral("en") && language != QStringLiteral("zh_CN"))
        || m_language == language)
        return;

    // Load first so a missing catalog cannot leave a partially switched UI.
    const QString catalog = QStringLiteral(":/i18n/qcatgallery_%1.qm").arg(language);
    auto translator = std::make_unique<QTranslator>();
    if (!translator->load(catalog))
    {
        qWarning("Could not load gallery translation catalog: %s", qPrintable(catalog));
        return;
    }
    if (m_translator)
        QCoreApplication::removeTranslator(m_translator.get());
    for (auto &translator : m_qtTranslators)
        QCoreApplication::removeTranslator(&translator);
    if (language == QStringLiteral("zh_CN"))
    {
        const std::array<const char *, 2> catalogs = {"qtbase_zh_CN", "qtdeclarative_zh_CN"};
        for (std::size_t i = 0; i < catalogs.size(); ++i)
            if (m_qtTranslators[i].load(QString::fromLatin1(catalogs[i]),
                                      QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
                QCoreApplication::installTranslator(&m_qtTranslators[i]);
    }
    m_translator = std::move(translator);
    QCoreApplication::installTranslator(m_translator.get());
    m_language = language;
    QLocale::setDefault(QLocale(localeName()));
    emit languageChanged();
    m_engine->retranslate();
}
