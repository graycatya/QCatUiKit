// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QCatUiKit/qcatledsource.h>
#include <QtQml/qqmlregistration.h>
#include <memory>

class GalleryLedDemo : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QCatLedSource *deckA READ deckA CONSTANT)
    Q_PROPERTY(QCatLedSource *deckB READ deckB CONSTANT)
    Q_PROPERTY(QCatLedSource *output READ output CONSTANT)
    Q_PROPERTY(int pointCount READ pointCount WRITE setPointCount NOTIFY settingsChanged)
    Q_PROPERTY(bool spatial READ spatial WRITE setSpatial NOTIFY settingsChanged)
    Q_PROPERTY(qreal mix READ mix WRITE setMix NOTIFY settingsChanged)
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY settingsChanged)
    Q_PROPERTY(int interval READ interval WRITE setInterval NOTIFY settingsChanged)
  public:
    explicit GalleryLedDemo(QObject *parent = nullptr);
    ~GalleryLedDemo() override;
    QCatLedSource *deckA() const;
    QCatLedSource *deckB() const;
    QCatLedSource *output() const;
    int pointCount() const;
    void setPointCount(int count);
    bool spatial() const;
    void setSpatial(bool spatial);
    qreal mix() const;
    void setMix(qreal mix);
    bool running() const;
    void setRunning(bool running);
    int interval() const;
    void setInterval(int ms);
  Q_SIGNALS:
    void settingsChanged();

  private:
    struct Impl;
    const std::unique_ptr<Impl> d;
};
