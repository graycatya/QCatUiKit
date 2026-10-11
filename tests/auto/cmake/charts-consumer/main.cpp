// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/ChartAxis.h>
#include <QCatUiKit/ChartItem>
#include <QCatUiKit/qcatchartseries.h>
#include <QDebug>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <memory>
int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    if (argc != 2)
        return 1;
    QCatUiKit::ChartItem chart;
    QCatUiKit::ChartSeries series;
    series.setCapacity(3);
    chart.setSize({640, 360});
    chart.addSeries(&series);
    if (!series.appendData({QVariantMap{{"x", 0}, {"y", 2}}, QVariantMap{{"x", 1}, {"y", 4}},
                            QVariantMap{{"x", 2}, {"y", 1}}, QVariantMap{{"x", 3}, {"y", 3}}}))
        return 2;
    if (series.count() != 3 || series.sampleAt(0).x != 1)
        return 3;
    if (chart.dataAt(chart.mapToPosition({3, 3})).value("index").toInt() != 2)
        return 4;
    QQmlEngine engine;
    engine.addImportPath(QString::fromLocal8Bit(argv[1]));
    QQmlComponent component(&engine);
    component.setData(R"(import QtQuick
import QCatUiKit
CatChart {
    width: 640; height: 360
    chartType: CatChartSeries.Bar
    xAxis.scale: CatChartAxis.Category
    xAxis.categories: ["A", "B"]
    CatChartSeries { name: "Orders"; Component.onCompleted: setData([{x: 0, y: 2}, {x: 1, y: 5}]) }
})",
                      QUrl());
    std::unique_ptr<QObject> object(component.create());
    if (!object)
    {
        qWarning() << component.errors();
        return 5;
    }
    auto *qmlChart = qobject_cast<QCatUiKit::ChartItem *>(object.get());
    if (!qmlChart || qmlChart->seriesCount() != 1 || qmlChart->seriesAt(0)->count() != 2)
        return 6;
    qInfo() << "Installed chart C++ and QML consumer OK";
    return 0;
}
