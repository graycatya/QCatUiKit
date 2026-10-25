// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/VectorItem.h>
#include <QCatUiKit/VectorItem>
#include <QCatUiKit/qcatvectoritem.h>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QTimer>
#include <memory>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQuickWindow window;
    window.resize(320, 200);
    QCatUiKit::VectorItem svg(window.contentItem());
    if (!svg.backendAvailable())
        return 1;
    svg.setSize({160, 200});
    svg.setSourceData("<svg xmlns='http://www.w3.org/2000/svg' width='100' height='100'><rect "
                      "width='100' height='100' fill='red'/></svg>");
    QQmlEngine engine;
    engine.addImportPath(QStringLiteral(QCAT_INSTALLED_QML_PATH));
    QQmlComponent component(&engine);
    component.setData(R"(
import QCatUiKit
CatVectorCanvas {
    x: 160; width: 160; height: 200
    shapes: [{type: "ellipse", x: 0, y: 0, width: 100, height: 100, fill: "blue"}]
}
)",
                      QUrl());
    std::unique_ptr<QObject> object(component.create());
    auto *canvas = qobject_cast<QCatUiKit::VectorItem *>(object.get());
    if (!canvas)
        return 2;
    canvas->setParentItem(window.contentItem());
    QTimer poll;
    QObject::connect(
        &poll, &QTimer::timeout, &app,
        [&]
        {
            if (svg.status() == QCatUiKit::VectorItem::Error ||
                canvas->status() == QCatUiKit::VectorItem::Error)
                app.exit(3);
            else if (svg.status() == QCatUiKit::VectorItem::Ready &&
                     canvas->status() == QCatUiKit::VectorItem::Ready)
            {
                const auto image = window.grabWindow();
                const bool ok =
                    !image.isNull() &&
                    image.pixelColor(image.width() / 4, image.height() / 2).red() > 200 &&
                    image.pixelColor(3 * image.width() / 4, image.height() / 2).blue() > 200;
                app.exit(ok ? 0 : 4);
            }
        });
    QTimer::singleShot(10000, &app, [&] { app.exit(5); });
    poll.start(20);
    window.show();
    const int result = app.exec();
    canvas->setParentItem(nullptr);
    return result;
}
