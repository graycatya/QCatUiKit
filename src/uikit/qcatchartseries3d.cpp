// SPDX-License-Identifier: Apache-2.0
#include "qcatchartseries3d.h"
#include <QImageReader>
#include <QThread>
#include <cmath>
namespace QCatUiKit
{
namespace
{
bool finite3(QVector3D v)
{
    return std::isfinite(v.x()) && std::isfinite(v.y()) && std::isfinite(v.z());
}
QString localImage(const QUrl &u)
{
    return u.isLocalFile()        ? u.toLocalFile()
           : u.scheme() == "qrc"  ? ":" + u.path()
           : u.scheme().isEmpty() ? u.toString()
                                  : QString();
}
bool decode(const QVariantMap &m, ChartSeries3D::Point &p)
{
    bool x = false, y = false, z = false;
    p.x = m.value("x").toDouble(&x);
    p.y = m.value("y").toDouble(&y);
    p.z = m.value("z").toDouble(&z);
    p.size = m.value("size", 1.).toDouble();
    p.label = m.value("label").toString();
    p.valid = m.value("valid", true).toBool();
    if (m.contains("color"))
    {
        p.color = QColor(m.value("color").toString());
        if (!p.color.isValid())
            return false;
    }
    if (m.contains("rotation"))
    {
        if (!m.value("rotation").canConvert<QVector3D>())
            return false;
        p.rotation = m.value("rotation").value<QVector3D>();
    }
    return x && y && z && std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z) &&
           std::isfinite(p.size) && p.size > 0 && p.size <= 100 && finite3(p.rotation);
}
} // namespace
class ChartSeries3D::Impl
{
  public:
    Type type = Scatter;
    QString name = {};
    bool visible = true;
    QColor color = QColor("#1677ff");
    Shape shape = Sphere;
    MeshDetail meshDetail = MediumDetail;
    double pointSize = .06;
    double baseline = 0;
    double barWidth = .7;
    bool polar = false;
    bool degrees = true;
    bool closed = false;
    double tension = 0;
    int subdivisions = 12;
    bool nodesVisible = false;
    SurfaceMode surfaceMode = Filled;
    bool colorByHeight = false;
    QUrl textureSource = {};
    QVector<Point> samples;
    QVariantList mesh;
    int rows = 0, columns = 0;
    QString error;
};
ChartSeries3D::ChartSeries3D(QObject *parent) : QObject(parent), d(new Impl) {}
ChartSeries3D::~ChartSeries3D() = default;
ChartSeries3D::Type ChartSeries3D::type() const
{
    return d->type;
}
void ChartSeries3D::setType(Type value)
{
    if (QThread::currentThread() != thread() || value < Scatter || value > Surface ||
        d->type == value)
        return;
    d->type = value;
    emit changed();
}
QString ChartSeries3D::name() const
{
    return d->name;
}
void ChartSeries3D::setName(const QString &value)
{
    if (QThread::currentThread() != thread() || d->name == value)
        return;
    d->name = value;
    emit changed();
}
bool ChartSeries3D::visible() const
{
    return d->visible;
}
void ChartSeries3D::setVisible(bool value)
{
    if (QThread::currentThread() != thread() || d->visible == value)
        return;
    d->visible = value;
    emit changed();
}
QColor ChartSeries3D::color() const
{
    return d->color;
}
void ChartSeries3D::setColor(const QColor &value)
{
    if (QThread::currentThread() != thread() || !value.isValid() || d->color == value)
        return;
    d->color = value;
    emit changed();
}
ChartSeries3D::Shape ChartSeries3D::shape() const
{
    return d->shape;
}
void ChartSeries3D::setShape(Shape value)
{
    if (QThread::currentThread() != thread() || value < Sphere || value > RoundedCube ||
        d->shape == value)
        return;
    d->shape = value;
    emit changed();
}
ChartSeries3D::MeshDetail ChartSeries3D::meshDetail() const
{
    return d->meshDetail;
}
void ChartSeries3D::setMeshDetail(MeshDetail value)
{
    if (QThread::currentThread() != thread() || value < LowDetail || value > HighDetail ||
        d->meshDetail == value)
        return;
    d->meshDetail = value;
    emit changed();
}
double ChartSeries3D::pointSize() const
{
    return d->pointSize;
}
void ChartSeries3D::setPointSize(double value)
{
    if (QThread::currentThread() != thread() || !std::isfinite(value) || value <= 0 || value > 2 ||
        d->pointSize == value)
        return;
    d->pointSize = value;
    emit changed();
}
double ChartSeries3D::baseline() const
{
    return d->baseline;
}
void ChartSeries3D::setBaseline(double value)
{
    if (QThread::currentThread() != thread() || !std::isfinite(value) || d->baseline == value)
        return;
    d->baseline = value;
    emit changed();
}
double ChartSeries3D::barWidth() const
{
    return d->barWidth;
}
void ChartSeries3D::setBarWidth(double value)
{
    if (QThread::currentThread() != thread() || !std::isfinite(value) || value <= 0 || value > 1 ||
        d->barWidth == value)
        return;
    d->barWidth = value;
    emit changed();
}
bool ChartSeries3D::polar() const
{
    return d->polar;
}
void ChartSeries3D::setPolar(bool value)
{
    if (QThread::currentThread() != thread() || d->polar == value)
        return;
    d->polar = value;
    emit changed();
}
bool ChartSeries3D::degrees() const
{
    return d->degrees;
}
void ChartSeries3D::setDegrees(bool value)
{
    if (QThread::currentThread() != thread() || d->degrees == value)
        return;
    d->degrees = value;
    emit changed();
}
bool ChartSeries3D::closed() const
{
    return d->closed;
}
void ChartSeries3D::setClosed(bool value)
{
    if (QThread::currentThread() != thread() || d->closed == value)
        return;
    d->closed = value;
    emit changed();
}
double ChartSeries3D::tension() const
{
    return d->tension;
}
void ChartSeries3D::setTension(double value)
{
    if (QThread::currentThread() != thread() || !std::isfinite(value) || value < 0 || value > 1 ||
        d->tension == value)
        return;
    d->tension = value;
    emit changed();
}
int ChartSeries3D::subdivisions() const
{
    return d->subdivisions;
}
void ChartSeries3D::setSubdivisions(int value)
{
    if (QThread::currentThread() != thread() || value < 1 || value > 64 || d->subdivisions == value)
        return;
    d->subdivisions = value;
    emit changed();
}
bool ChartSeries3D::nodesVisible() const
{
    return d->nodesVisible;
}
void ChartSeries3D::setNodesVisible(bool value)
{
    if (QThread::currentThread() != thread() || d->nodesVisible == value)
        return;
    d->nodesVisible = value;
    emit changed();
}
ChartSeries3D::SurfaceMode ChartSeries3D::surfaceMode() const
{
    return d->surfaceMode;
}
void ChartSeries3D::setSurfaceMode(SurfaceMode value)
{
    if (QThread::currentThread() != thread() || value < Filled || value > FilledWireframe ||
        d->surfaceMode == value)
        return;
    d->surfaceMode = value;
    emit changed();
}
bool ChartSeries3D::colorByHeight() const
{
    return d->colorByHeight;
}
void ChartSeries3D::setColorByHeight(bool value)
{
    if (QThread::currentThread() != thread() || d->colorByHeight == value)
        return;
    d->colorByHeight = value;
    emit changed();
}
QUrl ChartSeries3D::textureSource() const
{
    return d->textureSource;
}
void ChartSeries3D::setTextureSource(const QUrl &value)
{
    if (QThread::currentThread() != thread() || d->textureSource == value)
        return;
    d->textureSource = value;
    emit changed();
}

bool ChartSeries3D::fail(const QString &error)
{
    if (d->error != error)
    {
        d->error = error;
        emit errorChanged();
    }
    return error.isEmpty();
}
QString ChartSeries3D::error() const
{
    return d->error;
}
const QVector<ChartSeries3D::Point> &ChartSeries3D::samples() const
{
    return d->samples;
}
int ChartSeries3D::count() const
{
    return int(d->samples.size());
}
int ChartSeries3D::rows() const
{
    return d->rows;
}
int ChartSeries3D::columns() const
{
    return d->columns;
}
QVariantMap ChartSeries3D::at(int i) const
{
    if (i < 0 || i >= count())
        return {};
    const auto &p = d->samples[i];
    QVariantMap m{{"x", p.x},
                  {"y", p.y},
                  {"z", p.z},
                  {"size", p.size},
                  {"rotation", QVariant::fromValue(p.rotation)},
                  {"label", p.label},
                  {"valid", p.valid}};
    if (p.color.isValid())
        m.insert("color", p.color);
    return m;
}
QVariantList ChartSeries3D::points() const
{
    QVariantList v;
    v.reserve(count());
    for (int i = 0; i < count(); ++i)
        v.append(at(i));
    return v;
}
bool ChartSeries3D::setSamples(const QVector<Point> &v, int r, int c)
{
    if (QThread::currentThread() != thread())
        return false;
    if (r < 0 || c < 0 || (r == 0) != (c == 0) || (r && qint64(r) * c != v.size()) ||
        v.size() > 1000000)
        return fail(
            QStringLiteral("Expected at most 1000000 points and a complete row-major grid."));
    for (const auto &p : v)
        if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z) ||
            !std::isfinite(p.size) || p.size <= 0 || p.size > 100 || !finite3(p.rotation))
            return fail(QStringLiteral("XYZ, size and rotation must be finite; size must be in (0, "
                                       "100]. Use valid:false for a missing grid point."));
    d->samples = v;
    d->rows = r;
    d->columns = c;
    fail({});
    emit dataChanged();
    return true;
}
void ChartSeries3D::setPoints(const QVariantList &v)
{
    setGrid(0, 0, v);
}
bool ChartSeries3D::setGrid(int r, int c, const QVariantList &v)
{
    if (QThread::currentThread() != thread())
        return false;
    if (v.size() > 1000000)
        return fail(QStringLiteral("Too many 3D points."));
    QVector<Point> points;
    points.reserve(v.size());
    for (const auto &record : v)
    {
        Point p;
        if (!decode(record.toMap(), p))
            return fail(QStringLiteral("Each 3D record requires finite x, y and z."));
        points.append(p);
    }
    return setSamples(points, r, c);
}
bool ChartSeries3D::append(const QVariantMap &m)
{
    if (QThread::currentThread() != thread() || d->rows || count() >= 1000000)
        return false;
    Point p;
    if (!decode(m, p))
        return fail(QStringLiteral("Invalid 3D point."));
    d->samples.append(p);
    fail({});
    emit dataChanged();
    return true;
}
bool ChartSeries3D::replace(int i, const QVariantMap &m)
{
    if (QThread::currentThread() != thread() || i < 0 || i >= count())
        return false;
    Point p;
    if (!decode(m, p))
        return fail(QStringLiteral("Invalid 3D point."));
    d->samples[i] = p;
    fail({});
    emit dataChanged();
    return true;
}
bool ChartSeries3D::remove(int i, int n)
{
    if (QThread::currentThread() != thread() || d->rows || i < 0 || n <= 0 || i > count() - n)
        return false;
    d->samples.remove(i, n);
    fail({});
    emit dataChanged();
    return true;
}
void ChartSeries3D::clear()
{
    setSamples({});
}
QVariantList ChartSeries3D::customMesh() const
{
    return d->mesh;
}
void ChartSeries3D::setCustomMesh(const QVariantList &v)
{
    if (QThread::currentThread() != thread() || v == d->mesh)
        return;
    if (v.size() % 3 || v.size() > 300000)
    {
        fail(QStringLiteral("Custom mesh must be a triangle list of at most 300000 vertices."));
        return;
    }
    for (const auto &p : v)
        if (!p.canConvert<QVector3D>() || !finite3(p.value<QVector3D>()) ||
            p.value<QVector3D>().length() > 100)
        {
            fail(QStringLiteral(
                "Custom mesh vertices must be finite QVector3D values within radius 100."));
            return;
        }
    d->mesh = v;
    fail({});
    emit changed();
}
bool ChartSeries3D::setHeightMap(const QUrl &u, double scale)
{
    if (QThread::currentThread() != thread() || !std::isfinite(scale))
        return false;
    const auto path = localImage(u);
    QImageReader reader(path);
    const auto size = path.isEmpty() ? QSize() : reader.size();
    if (path.isEmpty() || size.width() < 2 || size.height() < 2 || size.width() > 1000 ||
        size.height() > 1000)
        return fail(
            QStringLiteral("Height map requires a local/qrc image between 2x2 and 1000x1000."));
    const auto image = reader.read();
    if (image.isNull())
        return fail(reader.errorString());
    QVector<Point> points;
    points.reserve(image.width() * image.height());
    for (int r = 0; r < image.height(); ++r)
        for (int c = 0; c < image.width(); ++c)
        {
            Point p;
            p.x = c;
            p.z = r;
            const auto color = image.pixelColor(c, r);
            p.y = qGray(color.rgb()) / 255. * scale;
            p.valid = color.alpha() > 0;
            points.append(p);
        }
    return setSamples(points, image.height(), image.width());
}
} // namespace QCatUiKit
