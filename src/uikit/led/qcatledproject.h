// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatledoutput.h"

// Optional assembly. Applications can instead compose all these public parts independently.
class QCATUIKIT_EXPORT QCatLedProject : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LedProject)
    Q_PROPERTY(QCatLedLayoutModel *layout READ layout CONSTANT)
    Q_PROPERTY(QCatLedEffectRegistry *registry READ registry CONSTANT)
    Q_PROPERTY(QCatLedStorageRepository *repository READ repository CONSTANT)
    Q_PROPERTY(QCatLedDeck *deckA READ deckA CONSTANT)
    Q_PROPERTY(QCatLedDeck *deckB READ deckB CONSTANT)
    Q_PROPERTY(QCatLedMixer *mixer READ mixer CONSTANT)
    Q_PROPERTY(QCatLedOutput *output READ output CONSTANT)
    Q_PROPERTY(QCatMemoryOutputBackend *memoryBackend READ memoryBackend CONSTANT)
    Q_PROPERTY(QVariantList previewPreferences READ previewPreferences WRITE setPreviewPreferences
                   NOTIFY preferencesChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY statusChanged)
  public:
    explicit QCatLedProject(QObject *parent = nullptr);
    ~QCatLedProject() override;
    QCatLedLayoutModel *layout() const;
    QCatLedEffectRegistry *registry() const;
    QCatLedStorageRepository *repository() const;
    QCatLedDeck *deckA() const;
    QCatLedDeck *deckB() const;
    QCatLedMixer *mixer() const;
    QCatLedOutput *output() const;
    QCatMemoryOutputBackend *memoryBackend() const;
    QVariantList previewPreferences() const;
    void setPreviewPreferences(const QVariantList &preferences);
    QString lastError() const;
    Q_INVOKABLE QString exportJson() const;
    Q_INVOKABLE bool importJson(const QString &json);
    Q_INVOKABLE bool saveFile(const QUrl &url);
    Q_INVOKABLE bool loadFile(const QUrl &url);
  Q_SIGNALS:
    void preferencesChanged();
    void statusChanged();

  private:
    struct Impl;
    const std::unique_ptr<Impl> d;
};
