// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QCatUiKit/qcatuikitglobal.h>
#include <QColor>
#include <QObject>
#include <QUrl>
#include <QVariant>
#include <QVector3D>
#include <QtQml/qqmlregistration.h>
#include <memory>

namespace QCatUiKit
{
class QCATUIKIT_EXPORT ChartSeries3D : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CatChartSeries3D)
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY changed)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY changed)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY changed)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY changed)
    Q_PROPERTY(Shape shape READ shape WRITE setShape NOTIFY changed)
    Q_PROPERTY(double pointSize READ pointSize WRITE setPointSize NOTIFY changed)
    Q_PROPERTY(double baseline READ baseline WRITE setBaseline NOTIFY changed)
    Q_PROPERTY(double barWidth READ barWidth WRITE setBarWidth NOTIFY changed)
    Q_PROPERTY(bool polar READ polar WRITE setPolar NOTIFY changed)
    Q_PROPERTY(bool degrees READ degrees WRITE setDegrees NOTIFY changed)
    Q_PROPERTY(bool closed READ closed WRITE setClosed NOTIFY changed)
    Q_PROPERTY(double tension READ tension WRITE setTension NOTIFY changed)
    Q_PROPERTY(int subdivisions READ subdivisions WRITE setSubdivisions NOTIFY changed)
    Q_PROPERTY(bool nodesVisible READ nodesVisible WRITE setNodesVisible NOTIFY changed)
    Q_PROPERTY(SurfaceMode surfaceMode READ surfaceMode WRITE setSurfaceMode NOTIFY changed)
    Q_PROPERTY(bool colorByHeight READ colorByHeight WRITE setColorByHeight NOTIFY changed)
    Q_PROPERTY(QUrl textureSource READ textureSource WRITE setTextureSource NOTIFY changed)
    Q_PROPERTY(QVariantList customMesh READ customMesh WRITE setCustomMesh NOTIFY changed)
    Q_PROPERTY(QVariantList points READ points WRITE setPoints NOTIFY dataChanged)
    Q_PROPERTY(int count READ count NOTIFY dataChanged)
    Q_PROPERTY(int rows READ rows NOTIFY dataChanged)
    Q_PROPERTY(int columns READ columns NOTIFY dataChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(MeshDetail meshDetail READ meshDetail WRITE setMeshDetail NOTIFY changed)
  public:
    enum Type
    {
        Scatter,
        Bars,
        Curve,
        Surface
    };
    Q_ENUM(Type)
    enum Shape
    {
        Sphere,
        Cube,
        Cylinder,
        CustomMesh,
        RoundedCube
    };
    Q_ENUM(Shape)
    enum SurfaceMode
    {
        Filled,
        Wireframe,
        FilledWireframe
    };
    Q_ENUM(SurfaceMode)
    enum MeshDetail
    {
        LowDetail,
        MediumDetail,
        HighDetail
    };
    Q_ENUM(MeshDetail)
    // Separate contract: ChartSeries::Sample and its value field remain unchanged.
    struct Point
    {
        double x = 0, y = 0, z = 0;
        double size = 1;
        QColor color;
        QVector3D rotation;
        QString label;
        bool valid = true;
    };
    explicit ChartSeries3D(QObject *parent = nullptr);
    ~ChartSeries3D() override;
    Type type() const;
    void setType(Type value);
    QString name() const;
    void setName(const QString &value);
    bool visible() const;
    void setVisible(bool value);
    QColor color() const;
    void setColor(const QColor &value);
    Shape shape() const;
    void setShape(Shape value);
    double pointSize() const;
    void setPointSize(double value);
    double baseline() const;
    void setBaseline(double value);
    double barWidth() const;
    void setBarWidth(double value);
    bool polar() const;
    void setPolar(bool value);
    bool degrees() const;
    void setDegrees(bool value);
    bool closed() const;
    void setClosed(bool value);
    double tension() const;
    void setTension(double value);
    int subdivisions() const;
    void setSubdivisions(int value);
    bool nodesVisible() const;
    void setNodesVisible(bool value);
    SurfaceMode surfaceMode() const;
    void setSurfaceMode(SurfaceMode value);
    bool colorByHeight() const;
    void setColorByHeight(bool value);
    QUrl textureSource() const;
    void setTextureSource(const QUrl &value);
    QVariantList customMesh() const;
    void setCustomMesh(const QVariantList &value);
    QVariantList points() const;
    void setPoints(const QVariantList &value);
    const QVector<Point> &samples() const;
    bool setSamples(const QVector<Point> &value, int rows = 0, int columns = 0);
    Q_INVOKABLE bool setGrid(int rows, int columns, const QVariantList &points);
    Q_INVOKABLE bool setHeightMap(const QUrl &source, double heightScale = 1.0);
    Q_INVOKABLE bool append(const QVariantMap &point);
    Q_INVOKABLE bool replace(int index, const QVariantMap &point);
    Q_INVOKABLE bool remove(int index, int count = 1);
    Q_INVOKABLE void clear();
    Q_INVOKABLE QVariantMap at(int index) const;
    int count() const;
    int rows() const;
    int columns() const;
    QString error() const;
    MeshDetail meshDetail() const;
    void setMeshDetail(MeshDetail value);
  signals:
    void changed();
    void dataChanged();
    void errorChanged();

  private:
    bool fail(const QString &error);
    Q_DISABLE_COPY_MOVE(ChartSeries3D)
    class Impl;
    const std::unique_ptr<Impl> d;
};
} // namespace QCatUiKit
