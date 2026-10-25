// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatleddeck.h"

class QCATUIKIT_EXPORT QCatLedMixer : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LedMixerRuntime)
    Q_PROPERTY(QCatLedDeck *deckA READ deckA WRITE setDeckA NOTIFY settingsChanged)
    Q_PROPERTY(QCatLedDeck *deckB READ deckB WRITE setDeckB NOTIFY settingsChanged)
    Q_PROPERTY(QCatLedLayoutModel *layout READ layout WRITE setLayout NOTIFY settingsChanged)
    Q_PROPERTY(QCatLedSource *source READ source CONSTANT)
    Q_PROPERTY(double crossfader READ crossfader WRITE setCrossfader NOTIFY settingsChanged)
    Q_PROPERTY(double master READ master WRITE setMaster NOTIFY settingsChanged)
    Q_PROPERTY(bool blackout READ blackout WRITE setBlackout NOTIFY settingsChanged)
    Q_PROPERTY(bool freeze READ freeze WRITE setFreeze NOTIFY settingsChanged)
    Q_PROPERTY(int workingSchema READ workingSchema WRITE setWorkingSchema NOTIFY settingsChanged)
    Q_PROPERTY(int conversion READ conversion WRITE setConversion NOTIFY settingsChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY statusChanged)
  public:
    explicit QCatLedMixer(QObject *parent = nullptr);
    ~QCatLedMixer() override;
    QCatLedDeck *deckA() const;
    void setDeckA(QCatLedDeck *deck);
    QCatLedDeck *deckB() const;
    void setDeckB(QCatLedDeck *deck);
    QCatLedLayoutModel *layout() const;
    void setLayout(QCatLedLayoutModel *layout);
    QCatLedSource *source() const;
    double crossfader() const;
    void setCrossfader(double value);
    double master() const;
    void setMaster(double value);
    bool blackout() const;
    void setBlackout(bool value);
    bool freeze() const;
    void setFreeze(bool value);
    int workingSchema() const;
    void setWorkingSchema(int value);
    int conversion() const;
    void setConversion(int value);
    QString lastError() const;
    QCatLightingFrame latestFrame() const;
    bool mixFrames(const QCatLightingFrame &a, const QCatLightingFrame &b);
    Q_INVOKABLE QVariantList channelSample() const;
    Q_INVOKABLE void remix();
    Q_INVOKABLE QVariantMap configuration() const;
    Q_INVOKABLE bool restoreConfiguration(const QVariantMap &configuration);
  Q_SIGNALS:
    void settingsChanged();
    void statusChanged();
    void frameReady();

  private:
    void schedule();
    struct Impl;
    const std::unique_ptr<Impl> d;
};
