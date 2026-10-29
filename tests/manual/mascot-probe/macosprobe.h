// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QJsonObject>
#include <QObject>
#include <memory>

class MacosProbe final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString report READ report NOTIFY changed)
  public:
    explicit MacosProbe(QJsonObject environment);
    ~MacosProbe() override;
    QString report() const;
    static int runTarget();
    Q_INVOKABLE void command(const QString &action);
    void start(const QString &output = {});
  signals:
    void changed();
    void finished(int result);

  private:
    struct Private;
    std::unique_ptr<Private> d;
};
