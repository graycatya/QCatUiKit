// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/TiltController.h>
#include <QCatUiKit/TiltController>
#include <QCatUiKit/qcattiltcontroller.h>
#include <QDebug>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <memory>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQmlEngine engine;
    engine.addImportPath(QStringLiteral(QCAT_INSTALLED_QML_PATH));
    QQmlComponent component(&engine);
    component.setData(R"(
import QtQuick
import QCatUiKit
Item {
    width: 640; height: 800
    CatAttitudeIndicator { width: 280; height: 280; pitchDegrees: 10; rollDegrees: 20; attitudeValid: true }
    TiltController { objectName: "tilt" }
    CatTiltControlPanel { y: 300; width: 360 }
}
)",
                      QUrl());
    std::unique_ptr<QObject> root(component.create());
    if (!root)
    {
        qCritical() << component.errors();
        return 1;
    }
    auto *tilt = root->findChild<QCatUiKit::TiltController *>("tilt");
    if (!tilt)
        return 2;
    tilt->setCalibrationDurationMs(100);
    tilt->setEnabled(true);
    if (!tilt->submitOrientation({}, 0) || !tilt->calibrate())
        return 3;
    for (qint64 timestamp = 20000; timestamp <= 100000; timestamp += 20000)
        if (!tilt->submitOrientation({}, timestamp))
            return 4;
    if (!tilt->calibrated() || tilt->outputValid())
        return 5;
    tilt->setControlAllowed(true);
    if (!tilt->engage())
        return 6;
    tilt->disengage();
    if (tilt->outputValid())
        return 7;
    qInfo() << "Installed attitude/tilt consumer OK";
    return 0;
}
