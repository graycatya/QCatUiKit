// SPDX-License-Identifier: MIT
#include "gallerychartdata.h"
#include <QCoreApplication>
#include <QDateTime>
#include <algorithm>
#include <cmath>

using namespace QCatUiKit;
namespace GalleryDemos
{
enum Demo
{
    Line,
    Area,
    Bar,
    StackedBar,
    Scatter,
    Bubble,
    Pie,
    Doughnut,
    Radar,
    Heatmap,
    Mixed,
    TimeSeries,
    Logarithmic,
    SignedStack,
    Trend,
    Count
};
struct Description
{
    const char *name;
    const char *shortcut;
    const char *hint;
};
const Description &description(int index)
{
    static const Description demos[] = {
        {"Line", "1",
         "Two signals, 100,000 initial samples each. Add or remove data, or start a continuous "
         "stress test."},
        {"Area", "2",
         "Daily solar and wind output. Each area fills to zero; the two areas overlap."},
        {"Grouped bars", "3",
         "Compare online and store orders by month. Click a legend entry to hide a series."},
        {"Stacked bars", "4",
         "Monthly orders stacked by channel. The column height represents the total."},
        {"Scatter", "5",
         "Two sensor populations: temperature versus power. Hover to inspect individual readings."},
        {"Bubble", "6",
         "Product reach versus conversion. Bubble radius represents relative order volume."},
        {"Pie", "7",
         "Traffic sources as shares of a whole. Click legend entries to toggle individual slices."},
        {"Doughnut", "8",
         "Budget allocation with a central total. Hover over a segment to inspect its value."},
        {"Radar", "9",
         "Compare two product profiles across six dimensions on a common 0-100 scale."},
        {"Heatmap", "0",
         "Synthetic frequency intensity over time. Hover over cells; the color bar shows "
         "intensity."},
        {"Bar + line", "M",
         "Monthly actual orders as bars and target orders as a line, sharing the same value axis."},
        {"Time axis", "T",
         "A full day of indoor and outdoor temperatures, sampled every 15 minutes (UTC)."},
        {"Log axis", "L",
         "Two positive growth curves spanning orders of magnitude on a logarithmic Y axis."},
        {"Positive / negative", "N",
         "Quarterly changes for two regions. Positive and negative values stack separately."},
        {"Scatter + trend", "F",
         "Observed measurements and their least-squares fitted line, sharing XY coordinates."}};
    return demos[index];
}

void populate(int index, ChartItem *chart, ChartSeries *first, ChartSeries *second)
{
    chart->clearSeries();
    chart->addSeries(first);
    chart->addSeries(second);
    first->setType(ChartSeries::Inherit);
    second->setType(ChartSeries::Inherit);
    first->setVisible(true);
    second->setVisible(true);
    first->setName("Signal A");
    second->setName("Signal B");
    first->setColor({});
    second->setColor({});
    first->setLineWidth(2);
    second->setLineWidth(2);
    chart->xAxis()->setScale(ChartAxis::Linear);
    chart->yAxis()->setScale(ChartAxis::Linear);
    chart->xAxis()->setCategories({});
    chart->yAxis()->setCategories({});
    chart->xAxis()->setTitle("X");
    chart->yAxis()->setTitle("Value");
    chart->xAxis()->setTickCount(6);
    chart->yAxis()->setTickCount(6);
    chart->setCellWidth(1);
    chart->setCellHeight(1);
    chart->setChartType(index <= Heatmap ? ChartSeries::Type(index) : ChartSeries::Line);
    chart->setTitle(QString::fromUtf8(description(index).name));
    chart->resetView();
    QVector<ChartSeries::Sample> a, b;
    const auto names = [&](const char *one, const char *two)
    {
        first->setName(QString::fromUtf8(one));
        second->setName(QString::fromUtf8(two));
    };
    const auto axes = [&](const char *x, const char *y)
    {
        chart->xAxis()->setTitle(QString::fromUtf8(x));
        chart->yAxis()->setTitle(QString::fromUtf8(y));
    };
    switch (index)
    {
    case Line:
        chart->setTitle("Live signals");
        axes("Sample index", "Amplitude");
        for (int i = 0; i < 100000; ++i)
        {
            a.append({double(i), std::sin(i * .007) * 8 + std::sin(i * .057) * 2, 1, {}});
            b.append({double(i), std::cos(i * .008) * 5, 1, {}});
        }
        break;
    case Area:
        names("Solar", "Wind");
        axes("Hour of day", "Power (kW)");
        for (int i = 0; i <= 96; ++i)
        {
            const double hour = i / 4.;
            a.append(
                {hour, std::max(0., 70 * std::sin((hour - 6) * 3.141592653589793 / 12)), 1, {}});
            b.append({hour, 22 + 8 * std::sin(hour * .5) + 4 * std::cos(hour * 1.7), 1, {}});
        }
        break;
    case Bar:
    case StackedBar:
    case Mixed:
    {
        const double online[] = {120, 145, 132, 180, 205, 226};
        const double store[] = {85, 92, 108, 98, 120, 136};
        names("Online", "Store");
        axes("Month", "Orders");
        chart->xAxis()->setScale(ChartAxis::Category);
        chart->xAxis()->setCategories({"Jan", "Feb", "Mar", "Apr", "May", "Jun"});
        if (index == Mixed)
        {
            chart->setChartType(ChartSeries::Bar);
            second->setType(ChartSeries::Line);
            names("Actual", "Target");
        }
        for (int i = 0; i < 6; ++i)
        {
            a.append({double(i), online[i], 1, {}});
            b.append({double(i), index == Mixed ? 130. + i * 20 : store[i], 1, {}});
        }
        break;
    }
    case Scatter:
        names("Sensor A", "Sensor B");
        axes("Temperature (C)", "Power (W)");
        for (int i = 0; i < 100; ++i)
        {
            const double x = 18 + (i * 37 % 101) * .18;
            a.append({x, 20 + x * 1.6 + 8 * std::sin(i * 2.4), 1, {}});
            b.append({x + 3, 40 + x * 1.4 + 10 * std::cos(i * 1.7), 1, {}});
        }
        break;
    case Bubble:
        names("Core products", "New products");
        axes("Reach (thousands)", "Conversion (%)");
        chart->setViewRange(0, 110, 0, 10);
        for (int i = 0; i < 12; ++i)
        {
            a.append({15. + (i * 23 % 85), 2. + (i * 7 % 19) * .35, 5. + i * 1.3,
                      QString("Core %1").arg(i + 1)});
            b.append({10. + (i * 31 % 90), 1. + (i * 11 % 23) * .3, 5. + (i * 3 % 13),
                      QString("New %1").arg(i + 1)});
        }
        break;
    case Pie:
    case Doughnut:
    {
        second->setVisible(false);
        first->setName(index == Pie ? "Visits" : "Budget");
        const QStringList labels =
            index == Pie
                ? QStringList{"Search", "Direct", "Social", "Referral", "Email"}
                : QStringList{"Engineering", "Marketing", "Operations", "Support", "Research"};
        const double values[] = {420, 260, 160, 95, 65};
        for (int i = 0; i < labels.size(); ++i)
            a.append({double(i), values[i], 1, labels[i]});
        break;
    }
    case Radar:
    {
        names("Product A", "Product B");
        chart->xAxis()->setScale(ChartAxis::Category);
        chart->xAxis()->setCategories(
            {"Speed", "Quality", "Usability", "Features", "Support", "Value"});
        chart->yAxis()->setRange(0, 100);
        const double av[] = {90, 76, 88, 65, 80, 72};
        const double bv[] = {68, 92, 70, 88, 65, 94};
        for (int i = 0; i < 6; ++i)
        {
            a.append({double(i), av[i], 1, {}});
            b.append({double(i), bv[i], 1, {}});
        }
        break;
    }
    case Heatmap:
        first->setName("Intensity");
        second->setVisible(false);
        axes("Time step", "Frequency bin");
        for (int x = 0; x < 100; ++x)
            for (int y = 0; y < 40; ++y)
            {
                const double center = 12 + 8 * std::sin(x * .07);
                const double value =
                    std::exp(-std::pow((y - center) / 4., 2)) +
                    .55 * std::exp(-std::pow((y - 30) / 3., 2)) * (.6 + .4 * std::cos(x * .12));
                a.append({double(x), double(y), value, {}});
            }
        break;
    case TimeSeries:
    {
        names("Indoor", "Outdoor");
        axes("Time (UTC)", "Temperature (C)");
        chart->xAxis()->setScale(ChartAxis::DateTime);
        const double start =
            QDateTime::fromString("2026-10-01T00:00:00Z", Qt::ISODate).toMSecsSinceEpoch();
        for (int i = 0; i <= 96; ++i)
        {
            const double phase = (i / 4. - 8) * 3.141592653589793 / 12;
            a.append({start + i * 900000., 23 + 1.5 * std::sin(phase), 1, {}});
            b.append({start + i * 900000., 19 + 7 * std::sin(phase), 1, {}});
        }
        break;
    }
    case Logarithmic:
        names("Growth A", "Growth B");
        axes("Time step", "Population (log scale)");
        chart->yAxis()->setScale(ChartAxis::Logarithmic);
        for (int i = 0; i <= 100; ++i)
        {
            a.append({double(i), std::pow(10., i / 25.), 1, {}});
            b.append({double(i), 3 * std::pow(10., i / 35.), 1, {}});
        }
        break;
    case SignedStack:
    {
        names("North", "South");
        axes("Quarter", "Change (thousands)");
        chart->setChartType(ChartSeries::StackedBar);
        chart->xAxis()->setScale(ChartAxis::Category);
        chart->xAxis()->setCategories({"Q1", "Q2", "Q3", "Q4", "Q5", "Q6"});
        const double av[] = {32, -18, 26, -22, 38, 15};
        const double bv[] = {18, -12, -14, 16, 24, -20};
        for (int i = 0; i < 6; ++i)
        {
            a.append({double(i), av[i], 1, {}});
            b.append({double(i), bv[i], 1, {}});
        }
        break;
    }
    case Trend:
    {
        names("Measurements", "Least-squares fit");
        axes("Input", "Response");
        chart->setChartType(ChartSeries::Scatter);
        second->setType(ChartSeries::Line);
        double sx = 0, sy = 0, sxx = 0, sxy = 0;
        for (int i = 0; i < 60; ++i)
        {
            const double x = i * .5, y = 8 + x * 1.8 + 6 * std::sin(i * 2.1);
            a.append({x, y, 1, {}});
            sx += x;
            sy += y;
            sxx += x * x;
            sxy += x * y;
        }
        const double slope = (a.size() * sxy - sx * sy) / (a.size() * sxx - sx * sx);
        const double intercept = (sy - slope * sx) / a.size();
        b = {{0, intercept, 1, {}}, {a.back().x, intercept + slope * a.back().x, 1, {}}};
        break;
    }
    }
    if (chart->xAxis()->scale() == ChartAxis::Category)
        chart->xAxis()->setTickCount(8);
    first->replaceSamples(a);
    second->replaceSamples(b);
}
} // namespace GalleryDemos

namespace
{
[[maybe_unused]] const char *const translationSources[] = {
    QT_TRANSLATE_NOOP(
        "GalleryChartData",
        "A full day of indoor and outdoor temperatures, sampled every 15 minutes (UTC)."),
    QT_TRANSLATE_NOOP("GalleryChartData", "Actual"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Amplitude"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Apr"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Area"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Bar + line"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Bubble"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Budget"),
    QT_TRANSLATE_NOOP(
        "GalleryChartData",
        "Budget allocation with a central total. Hover over a segment to inspect its value."),
    QT_TRANSLATE_NOOP("GalleryChartData", "Change (thousands)"),
    QT_TRANSLATE_NOOP(
        "GalleryChartData",
        "Compare online and store orders by month. Click a legend entry to hide a series."),
    QT_TRANSLATE_NOOP(
        "GalleryChartData",
        "Compare two product profiles across six dimensions on a common 0-100 scale."),
    QT_TRANSLATE_NOOP("GalleryChartData", "Conversion (%)"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Core %1"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Core products"),
    QT_TRANSLATE_NOOP(
        "GalleryChartData",
        "Daily solar and wind output. Each area fills to zero; the two areas overlap."),
    QT_TRANSLATE_NOOP("GalleryChartData", "Direct"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Doughnut"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Email"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Engineering"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Features"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Feb"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Frequency bin"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Grouped bars"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Growth A"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Growth B"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Heatmap"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Hour of day"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Indoor"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Input"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Intensity"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Jan"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Jun"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Least-squares fit"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Line"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Live signals"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Log axis"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Mar"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Marketing"),
    QT_TRANSLATE_NOOP("GalleryChartData", "May"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Measurements"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Month"),
    QT_TRANSLATE_NOOP(
        "GalleryChartData",
        "Monthly actual orders as bars and target orders as a line, sharing the same value axis."),
    QT_TRANSLATE_NOOP("GalleryChartData",
                      "Monthly orders stacked by channel. The column height represents the total."),
    QT_TRANSLATE_NOOP("GalleryChartData", "New %1"),
    QT_TRANSLATE_NOOP("GalleryChartData", "New products"),
    QT_TRANSLATE_NOOP("GalleryChartData", "North"),
    QT_TRANSLATE_NOOP(
        "GalleryChartData",
        "Observed measurements and their least-squares fitted line, sharing XY coordinates."),
    QT_TRANSLATE_NOOP("GalleryChartData", "Online"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Operations"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Orders"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Outdoor"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Pie"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Population (log scale)"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Positive / negative"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Power (W)"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Power (kW)"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Product A"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Product B"),
    QT_TRANSLATE_NOOP(
        "GalleryChartData",
        "Product reach versus conversion. Bubble radius represents relative order volume."),
    QT_TRANSLATE_NOOP("GalleryChartData", "Q1"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Q2"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Q3"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Q4"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Q5"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Q6"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Quality"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Quarter"),
    QT_TRANSLATE_NOOP(
        "GalleryChartData",
        "Quarterly changes for two regions. Positive and negative values stack separately."),
    QT_TRANSLATE_NOOP("GalleryChartData", "Radar"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Reach (thousands)"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Referral"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Research"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Response"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Sample index"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Scatter"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Scatter + trend"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Search"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Sensor A"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Sensor B"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Signal A"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Signal B"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Social"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Solar"),
    QT_TRANSLATE_NOOP("GalleryChartData", "South"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Speed"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Stacked bars"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Store"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Support"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Synthetic frequency intensity over time. Hover over "
                                          "cells; the color bar shows intensity."),
    QT_TRANSLATE_NOOP("GalleryChartData", "Target"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Temperature (C)"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Time (UTC)"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Time axis"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Time step"),
    QT_TRANSLATE_NOOP(
        "GalleryChartData",
        "Traffic sources as shares of a whole. Click legend entries to toggle individual slices."),
    QT_TRANSLATE_NOOP(
        "GalleryChartData",
        "Two positive growth curves spanning orders of magnitude on a logarithmic Y axis."),
    QT_TRANSLATE_NOOP(
        "GalleryChartData",
        "Two sensor populations: temperature versus power. Hover to inspect individual readings."),
    QT_TRANSLATE_NOOP("GalleryChartData", "Two signals, 100,000 initial samples each. Add or "
                                          "remove data, or start a continuous stress test."),
    QT_TRANSLATE_NOOP("GalleryChartData", "Category %1"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Dimension %1"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Slice %1"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Usability"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Value"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Visits"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Wind"),
};
}

GalleryChartData::GalleryChartData(QObject *parent) : QObject(parent)
{
    m_timer.setInterval(100);
    connect(&m_timer, &QTimer::timeout, this, &GalleryChartData::stressTick);
}
void GalleryChartData::load(int mode, ChartItem *chart, ChartSeries *first, ChartSeries *second)
{
    m_fit.setSource(nullptr);
    if (!chart || !first || !second || mode < 0 || mode >= GalleryDemos::Count)
        return;
    stopStress();
    m_mode = mode;
    m_barVariant = mode == GalleryDemos::StackedBar || mode == GalleryDemos::SignedStack ? 1 : 0;
    m_chart = chart;
    m_first = m_previewFirst = first;
    m_second = m_previewSecond = second;
    m_bufferFirst.reset();
    m_bufferSecond.reset();
    static const int capacities[] = {200000, 2000, 48, 48,   2000, 128, 16,  16,
                                     32,     200,  48, 2000, 2000, 48,  2000};
    static const int batches[] = {500, 4, 1, 1, 10, 1, 1, 1, 1, 1, 1, 4, 4, 1, 10};
    m_capacity = capacities[mode];
    m_batchSize = batches[mode];
    first->setCapacity(m_capacity * samplesPerUnit());
    second->setCapacity(m_capacity);
    GalleryDemos::populate(mode, chart, first, second);
    m_nextIndex = unitCount();
    resetStatistics();
    m_title = chart->title();
    m_xTitle = chart->xAxis()->title();
    m_yTitle = chart->yAxis()->title();
    m_categories = chart->xAxis()->categories();
    m_firstName = first->name();
    m_secondName = second->name();
    m_firstLabels.clear();
    m_secondLabels.clear();
    for (auto pair : {qMakePair(first, &m_firstLabels), qMakePair(second, &m_secondLabels)})
    {
        bool hasLabels = false;
        for (int i = 0; i < pair.first->count(); ++i)
        {
            const auto label = pair.first->sampleAt(i).label;
            pair.second->append(label);
            hasLabels |= !label.isEmpty();
        }
        if (!hasLabels)
            pair.second->clear();
    }
    retranslate();
    emit settingsChanged();
    emit statisticsChanged();
}
void GalleryChartData::retranslate()
{
    if (!m_chart || !m_first || !m_second)
        return;
    const auto translated = [](const QString &source)
    { return QCoreApplication::translate("GalleryChartData", source.toUtf8().constData()); };
    emit metadataChanged();
    m_chart->setTitle(translated(m_title));
    if (m_mode == GalleryDemos::Bar || m_mode == GalleryDemos::StackedBar ||
        m_mode == GalleryDemos::SignedStack)
    {
        static const ChartSeries::Type types[] = {ChartSeries::Bar,
                                                  ChartSeries::StackedBar,
                                                  ChartSeries::PercentBar,
                                                  ChartSeries::HorizontalBar,
                                                  ChartSeries::HorizontalStackedBar,
                                                  ChartSeries::HorizontalPercentBar};
        m_chart->setChartType(types[m_barVariant]);
        m_chart->xAxis()->setScale(horizontalBars() ? ChartAxis::Linear : ChartAxis::Category);
        m_chart->yAxis()->setScale(horizontalBars() ? ChartAxis::Category : ChartAxis::Linear);
        m_chart->xAxis()->setTickCount(horizontalBars() ? 6 : 8);
        m_chart->yAxis()->setTickCount(horizontalBars() ? 8 : 6);
    }
    const QString valueTitle =
        (m_barVariant == 2 || m_barVariant == 5) ? tr("Share (%)") : translated(m_yTitle);
    m_chart->xAxis()->setTitle(horizontalBars() ? valueTitle : translated(m_xTitle));
    m_chart->yAxis()->setTitle(horizontalBars() ? translated(m_xTitle) : valueTitle);
    m_first->setName(translated(m_firstName));
    m_second->setName(translated(m_secondName));
    synchronizeMetadata();
    updatePreview();
    emit statisticsChanged();
}

bool GalleryChartData::horizontalBars() const
{
    return m_barVariant >= 3 &&
           (m_mode == GalleryDemos::Bar || m_mode == GalleryDemos::StackedBar ||
            m_mode == GalleryDemos::SignedStack);
}
void GalleryChartData::setBarVariant(int value)
{
    if (value < 0 || value > 5 || value == m_barVariant || !m_chart ||
        (m_mode != GalleryDemos::Bar && m_mode != GalleryDemos::StackedBar &&
         m_mode != GalleryDemos::SignedStack))
        return;
    m_barVariant = value;
    m_chart->resetView();
    retranslate();
    emit settingsChanged();
}
QString GalleryChartData::hint() const
{
    return QCoreApplication::translate("GalleryChartData", GalleryDemos::description(m_mode).hint);
}

namespace
{
QVector<ChartSeries::Sample> retainedSamples(ChartSeries *series, int count)
{
    QVector<ChartSeries::Sample> result;
    result.reserve(count);
    for (int i = 0; i < count; ++i)
        result.append(series->sampleAt(i));
    return result;
}
QString translatedLabel(const QString &source)
{
    for (const auto *prefix : {"Core ", "New ", "Category ", "Dimension ", "Slice "})
        if (source.startsWith(QLatin1String(prefix)))
            return QCoreApplication::translate("GalleryChartData",
                                               (QByteArray(prefix) + "%1").constData())
                .arg(source.mid(qstrlen(prefix)));
    return QCoreApplication::translate("GalleryChartData", source.toUtf8().constData());
}
} // namespace

bool GalleryChartData::categorical() const
{
    using namespace GalleryDemos;
    return m_mode == Bar || m_mode == StackedBar || m_mode == Mixed || m_mode == SignedStack ||
           m_mode == Radar;
}
bool GalleryChartData::singleSeries() const
{
    using namespace GalleryDemos;
    return m_mode == Pie || m_mode == Doughnut || m_mode == Heatmap || m_mode == Trend;
}
int GalleryChartData::samplesPerUnit() const
{
    return m_mode == GalleryDemos::Heatmap ? 40 : 1;
}
int GalleryChartData::unitCount() const
{
    return m_first ? m_first->count() / samplesPerUnit() : 0;
}
int GalleryChartData::sampleCount() const
{
    return (m_first ? m_first->count() : 0) + (m_second ? m_second->count() : 0);
}
int GalleryChartData::maximumCapacity() const
{
    return 1000000 / samplesPerUnit();
}
int GalleryChartData::maximumBatchSize() const
{
    return std::min(m_capacity, 50000 / samplesPerUnit());
}
int GalleryChartData::displayedSampleCount() const
{
    return (m_previewFirst ? m_previewFirst->count() : 0) +
           (m_previewSecond ? m_previewSecond->count() : 0);
}
int GalleryChartData::previewUnitLimit() const
{
    using namespace GalleryDemos;
    if (m_mode == Radar)
        return 32;
    if (m_mode == Pie || m_mode == Doughnut)
        return 64;
    if (categorical() || m_mode == Bubble || m_mode == Heatmap)
        return 128;
    return 512;
}
bool GalleryChartData::aggregated() const
{
    return m_bufferFirst && unitCount() > previewUnitLimit();
}
QString GalleryChartData::previewDescription() const
{
    using namespace GalleryDemos;
    if (m_mode == Radar)
        return tr("Overview: adjacent dimensions are grouped by their mean.");
    if (m_mode == Pie || m_mode == Doughnut || m_mode == Bar || m_mode == StackedBar ||
        m_mode == Mixed)
        return tr("Overview: adjacent records are grouped by their sum.");
    if (m_mode == SignedStack)
        return tr("Overview: adjacent records are summed, keeping positive and negative values "
                  "separate.");
    if (m_mode == Heatmap)
        return tr("Overview: adjacent columns are averaged for each row.");
    if (m_mode == Scatter || m_mode == Bubble || m_mode == Trend)
        return tr("Overview: evenly spaced source records are shown; the trend uses all records.");
    return tr(
        "Overview: each group retains its minimum and maximum. Raw records remain available.");
}
QVariantMap GalleryChartData::sourceAt(int series, int index) const
{
    auto *source = series == 0 ? m_first.data() : series == 1 ? m_second.data() : nullptr;
    return source ? source->at(index) : QVariantMap{};
}
void GalleryChartData::configureMillionSamples()
{
    stopStress();
    const int samples = samplesPerUnit() + (singleSeries() ? 0 : 1);
    setCapacity(1000000 / samples);
    setBatchSize(50000 / samples);
    setInterval(200);
}
void GalleryChartData::bufferSources()
{
    if (m_bufferFirst || !m_first || !m_second)
        return;
    const auto copy = [&](ChartSeries *source, const QStringList &labels)
    {
        auto buffer = std::make_unique<ChartSeries>();
        buffer->setCapacity(source->capacity());
        buffer->setName(source->name());
        buffer->setType(source->type());
        auto samples = retainedSamples(source, source->count());
        for (int i = 0; i < samples.size(); ++i)
        {
            // Preserve original labels before switching to a bounded overview.
            if (categorical() && i < m_categories.size())
                samples[i].label = m_categories[i];
            else if (i < labels.size())
                samples[i].label = labels[i];
        }
        buffer->replaceSamples(samples);
        return buffer;
    };
    m_bufferFirst = copy(m_first, m_firstLabels);
    m_bufferSecond = copy(m_second, m_secondLabels);
    m_first = m_bufferFirst.get();
    m_second = m_bufferSecond.get();
}
void GalleryChartData::updatePreview()
{
    if (!m_bufferFirst || !m_chart || !m_previewFirst || !m_previewSecond)
        return;
    using namespace GalleryDemos;
    const int units = unitCount(), limit = previewUnitLimit();
    const bool reduce = aggregated();
    QVector<ChartSeries::Sample> a, b;
    QStringList categories;
    const auto rangeLabel = [&](int from, int to) {
        return (m_mode == Heatmap ? tr("Columns %1–%2") : tr("Records %1–%2"))
            .arg(from + 1)
            .arg(to);
    };
    const auto curve = [&](ChartSeries *source)
    {
        const int count = source->count();
        if (count <= limit)
            return retainedSamples(source, count);
        QVector<ChartSeries::Sample> result;
        result.reserve(limit);
        result.append(source->sampleAt(0));
        const int buckets = (limit - 2) / 2;
        for (int bucket = 0; bucket < buckets; ++bucket)
        {
            const int from = 1 + qint64(count - 2) * bucket / buckets;
            const int to = 1 + qint64(count - 2) * (bucket + 1) / buckets;
            int low = from, high = from;
            for (int i = from + 1; i < to; ++i)
            {
                if (source->sampleAt(i).y < source->sampleAt(low).y)
                    low = i;
                if (source->sampleAt(i).y > source->sampleAt(high).y)
                    high = i;
            }
            result.append(source->sampleAt(std::min(low, high)));
            if (low != high)
                result.append(source->sampleAt(std::max(low, high)));
        }
        result.append(source->sampleAt(count - 1));
        return result;
    };
    if (!reduce)
    {
        a = retainedSamples(m_first, m_first->count());
        b = retainedSamples(m_second, m_second->count());
        if (categorical())
        {
            for (int i = 0; i < a.size(); ++i)
                categories.append(a[i].label.isEmpty()
                                      ? (m_mode == Radar ? tr("Dimension %1") : tr("Category %1"))
                                            .arg(qint64(a[i].x) + 1)
                                      : translatedLabel(a[i].label));
            for (auto *samples : {&a, &b})
                for (int i = 0; i < samples->size(); ++i)
                    (*samples)[i].x = i;
        }
        for (auto *samples : {&a, &b})
            for (int i = 0; i < samples->size(); ++i)
            {
                auto &sample = (*samples)[i];
                if (!sample.label.isEmpty())
                    sample.label = translatedLabel(sample.label);
                else if (m_mode == Pie || m_mode == Doughnut)
                    sample.label = tr("Slice %1").arg(qint64(sample.x) + 1);
                else if (m_mode == Bubble)
                    sample.label = rangeLabel(i, i + 1);
            }
    }
    else if (m_mode == Line || m_mode == Area || m_mode == TimeSeries || m_mode == Logarithmic)
    {
        a = curve(m_first);
        b = curve(m_second);
    }
    else if (m_mode == Scatter || m_mode == Bubble || m_mode == Trend)
    {
        const auto sample = [&](ChartSeries *source)
        {
            QVector<ChartSeries::Sample> result;
            const int count = std::min(limit, source->count());
            for (int i = 0; i < count; ++i)
            {
                const int index = count > 1 ? qint64(source->count() - 1) * i / (count - 1) : 0;
                auto point = source->sampleAt(index);
                point.label = rangeLabel(index, index + 1);
                result.append(point);
            }
            return result;
        };
        a = sample(m_first);
        b = m_mode == Trend ? retainedSamples(m_second, m_second->count()) : sample(m_second);
    }
    else
    {
        // Group every source record, including a partial final group. Signed stacks
        // keep two entries per group so opposite signs never cancel each other out.
        const int groups = m_mode == SignedStack ? limit / 2 : limit;
        for (int group = 0; group < groups; ++group)
        {
            const int from = qint64(units) * group / groups;
            const int to = qint64(units) * (group + 1) / groups;
            const QString label = rangeLabel(from, to);
            if (m_mode == Heatmap)
            {
                for (int row = 0; row < 40; ++row)
                {
                    double sum = 0;
                    for (int column = from; column < to; ++column)
                        sum += m_first->sampleAt(column * 40 + row).value;
                    a.append({double(group), double(row), sum / (to - from), label});
                }
                categories.append(label);
                continue;
            }
            for (auto pair : {qMakePair(m_first.data(), &a), qMakePair(m_second.data(), &b)})
            {
                if (pair.first == m_second && singleSeries())
                    continue;
                double positive = 0, negative = 0;
                for (int i = from; i < to; ++i)
                {
                    const double value = pair.first->sampleAt(i).y;
                    (value >= 0 ? positive : negative) += value;
                }
                if (m_mode == SignedStack)
                {
                    pair.second->append({double(group * 2), positive, 1, label + " (+)"});
                    pair.second->append({double(group * 2 + 1), negative, 1, label + " (−)"});
                }
                else
                {
                    const double value =
                        m_mode == Radar ? (positive + negative) / (to - from) : positive + negative;
                    pair.second->append({double(group), value, 1, label});
                }
            }
            if (categorical())
            {
                if (m_mode == SignedStack)
                    categories << label + " (+)" << label + " (−)";
                else
                    categories.append(label);
            }
        }
    }
    if (m_mode == Heatmap)
        m_chart->xAxis()->setScale(reduce ? ChartAxis::Category : ChartAxis::Linear);
    if (categorical() || m_mode == Heatmap)
        (horizontalBars() ? m_chart->yAxis() : m_chart->xAxis())->setCategories(categories);
    // Source capacities belong to the retained data; preview buffers only need
    // enough room for the bounded overview and the optional fitted endpoints.
    m_previewFirst->setCapacity(std::max(1, int(a.size())));
    m_previewSecond->setCapacity(std::max(2, int(b.size())));
    m_previewFirst->setName(m_first->name());
    m_previewSecond->setName(m_second->name());
    m_previewFirst->replaceSamples(a);
    m_previewSecond->replaceSamples(b);
}
QString GalleryChartData::unitName() const
{
    using namespace GalleryDemos;
    if (m_mode == Heatmap)
        return tr("columns (40 cells each)");
    if (m_mode == Radar)
        return tr("dimensions");
    if (m_mode == Pie || m_mode == Doughnut)
        return tr("slices");
    return categorical() ? tr("categories per series") : tr("points per series");
}
void GalleryChartData::setBatchSize(int value)
{
    value = std::clamp(value, 1, maximumBatchSize());
    if (m_batchSize == value)
        return;
    m_batchSize = value;
    emit settingsChanged();
}
void GalleryChartData::setInterval(int value)
{
    value = std::clamp(value, 16, 1000);
    if (m_timer.interval() == value)
        return;
    m_timer.setInterval(value);
    emit settingsChanged();
}
void GalleryChartData::setCapacity(int value)
{
    value = std::clamp(value, m_mode == GalleryDemos::Radar ? 3 : 1, maximumCapacity());
    if (m_capacity == value || !m_first || !m_second)
        return;
    stopStress();
    if (value > previewUnitLimit())
        bufferSources();
    m_capacity = value;
    m_first->setCapacity(value * samplesPerUnit());
    m_second->setCapacity(m_mode == GalleryDemos::Trend ? std::max(2, value) : value);
    m_firstLabels = m_firstLabels.mid(std::max(0, int(m_firstLabels.size()) - m_first->count()));
    m_secondLabels =
        m_secondLabels.mid(std::max(0, int(m_secondLabels.size()) - m_second->count()));
    m_categories = m_categories.mid(std::max(0, int(m_categories.size()) - unitCount()));
    setBatchSize(m_batchSize);
    synchronizeMetadata();
    updateTrend();
    emit settingsChanged();
    QElapsedTimer timer;
    timer.start();
    finishMutation(0, 0, timer);
}
void GalleryChartData::synchronizeMetadata()
{
    if (m_bufferFirst || !m_chart || !m_first || !m_second)
        return;
    if (categorical())
    {
        QStringList categories;
        for (const auto &category : m_categories)
            categories.append(translatedLabel(category));
        (horizontalBars() ? m_chart->yAxis() : m_chart->xAxis())->setCategories(categories);
        for (auto *series : {m_first.data(), m_second.data()})
        {
            auto samples = retainedSamples(series, series->count());
            for (int i = 0; i < samples.size(); ++i)
                samples[i].x = i;
            series->replaceSamples(samples);
        }
    }
    for (auto pair :
         {qMakePair(m_first.data(), &m_firstLabels), qMakePair(m_second.data(), &m_secondLabels)})
    {
        if (pair.second->isEmpty() || pair.second->size() != pair.first->count())
            continue;
        auto samples = retainedSamples(pair.first, pair.first->count());
        for (int i = 0; i < samples.size(); ++i)
            samples[i].label = translatedLabel(pair.second->at(i));
        pair.first->replaceSamples(samples);
    }
}
void GalleryChartData::updateTrend()
{
    if (m_mode != GalleryDemos::Trend || !m_first || !m_second)
        return;
    m_fit.setSource(m_first);
    if (!m_fit.refresh()) { m_second->clear(); return; }
    m_second->replaceSamples({m_fit.series()->sampleAt(0), m_fit.series()->sampleAt(1)});
}
void GalleryChartData::addData()
{
    if (!m_chart || !m_first || !m_second)
        return;
    using namespace GalleryDemos;
    QElapsedTimer timer;
    timer.start();
    const int before = m_first->count() + (singleSeries() ? 0 : m_second->count());
    QVector<ChartSeries::Sample> a, b;
    a.reserve(m_batchSize * samplesPerUnit());
    if (!singleSeries())
        b.reserve(m_batchSize);
    const double timeOrigin =
        QDateTime::fromString("2026-10-01T00:00:00Z", Qt::ISODate).toMSecsSinceEpoch();
    for (int i = 0; i < m_batchSize; ++i)
    {
        const qint64 n = m_nextIndex++;
        const double x = double(n);
        switch (m_mode)
        {
        case Line:
            a.append({x, std::sin(x * .007) * 8 + std::sin(x * .057) * 2, 1, {}});
            b.append({x, std::cos(x * .008) * 5, 1, {}});
            break;
        case Area:
        {
            const double hour = x / 4.;
            a.append(
                {hour, std::max(0., 70 * std::sin((hour - 6) * 3.141592653589793 / 12)), 1, {}});
            b.append({hour, 22 + 8 * std::sin(hour * .5) + 4 * std::cos(hour * 1.7), 1, {}});
            break;
        }
        case Bar:
        case StackedBar:
        case Mixed:
        case SignedStack:
        case Radar:
        {
            double av = 120 + 70 * std::sin(x * .6), bv = 90 + 40 * std::cos(x * .7);
            if (m_mode == SignedStack)
            {
                av = 38 * std::sin(x * 1.3);
                bv = 25 * std::cos(x * 1.7);
            }
            else if (m_mode == Radar)
            {
                av = 50 + 45 * std::sin(x * .7);
                bv = 50 + 45 * std::cos(x * .9);
            }
            else if (m_mode == Mixed)
                bv = 150 + 25 * std::sin(x * .3);
            a.append({x, av, 1, {}});
            b.append({x, bv, 1, {}});
            if (!m_bufferFirst)
                m_categories.append(
                    QString(m_mode == Radar ? "Dimension %1" : "Category %1").arg(n + 1));
            break;
        }
        case Scatter:
        {
            const double temperature = 18 + (n * 37 % 101) * .18;
            a.append({temperature, 20 + temperature * 1.6 + 8 * std::sin(x * 2.4), 1, {}});
            b.append({temperature + 3, 40 + temperature * 1.4 + 10 * std::cos(x * 1.7), 1, {}});
            break;
        }
        case Bubble:
            if (!m_bufferFirst)
            {
                m_firstLabels.append(QString("Core %1").arg(n + 1));
                m_secondLabels.append(QString("New %1").arg(n + 1));
            }
            a.append({15. + (n * 23 % 85), 2. + (n * 7 % 19) * .35, 5. + (n % 12) * 1.3, {}});
            b.append({10. + (n * 31 % 90), 1. + (n * 11 % 23) * .3, 5. + (n * 3 % 13), {}});
            break;
        case Pie:
        case Doughnut:
            if (!m_bufferFirst)
                m_firstLabels.append(QString("Slice %1").arg(n + 1));
            a.append({x, 50. + (n * 73 % 400), 1, {}});
            break;
        case Heatmap:
            for (int y = 0; y < 40; ++y)
            {
                const double center = 12 + 8 * std::sin(x * .07);
                const double value =
                    std::exp(-std::pow((y - center) / 4., 2)) +
                    .55 * std::exp(-std::pow((y - 30) / 3., 2)) * (.6 + .4 * std::cos(x * .12));
                a.append({x, double(y), value, {}});
            }
            break;
        case TimeSeries:
        {
            const double phase = (x / 4. - 8) * 3.141592653589793 / 12;
            a.append({timeOrigin + x * 900000., 23 + 1.5 * std::sin(phase), 1, {}});
            b.append({timeOrigin + x * 900000., 19 + 7 * std::sin(phase), 1, {}});
            break;
        }
        case Logarithmic:
            // Keep indefinitely streamed values finite and strictly positive.
            a.append({x, std::pow(10., (n % 200) / 25.), 1, {}});
            b.append({x, 3 * std::pow(10., (n % 200) / 35.), 1, {}});
            break;
        case Trend:
            a.append({x * .5, 8 + x * .9 + 6 * std::sin(x * 2.1), 1, {}});
            break;
        }
    }
    m_first->appendSamples(a);
    if (!singleSeries())
        m_second->appendSamples(b);
    m_categories = m_categories.mid(std::max(0, int(m_categories.size()) - unitCount()));
    m_firstLabels = m_firstLabels.mid(std::max(0, int(m_firstLabels.size()) - m_first->count()));
    m_secondLabels =
        m_secondLabels.mid(std::max(0, int(m_secondLabels.size()) - m_second->count()));
    synchronizeMetadata();
    updateTrend();
    const int after = m_first->count() + (singleSeries() ? 0 : m_second->count());
    finishMutation(a.size() + b.size(),
                   std::max<qsizetype>(0, before + a.size() + b.size() - after), timer);
}
void GalleryChartData::removeData()
{
    if (!m_chart || !m_first || !m_second)
        return;
    QElapsedTimer timer;
    timer.start();
    const int units = std::min(m_batchSize, unitCount());
    m_first->replaceSamples(retainedSamples(m_first, m_first->count() - units * samplesPerUnit()));
    if (!singleSeries())
        m_second->replaceSamples(retainedSamples(m_second, std::max(0, m_second->count() - units)));
    m_categories = m_categories.mid(0, unitCount());
    m_firstLabels = m_firstLabels.mid(0, m_first->count());
    m_secondLabels = m_secondLabels.mid(0, m_second->count());
    synchronizeMetadata();
    updateTrend();
    finishMutation(0, units * (samplesPerUnit() + (singleSeries() ? 0 : 1)), timer);
}
void GalleryChartData::finishMutation(qint64 added, qint64 removed, const QElapsedTimer &timer)
{
    updatePreview();
    if (m_mode == GalleryDemos::Pie || m_mode == GalleryDemos::Doughnut)
    {
        // Slice visibility is indexed: reset it when editing/reordering slices.
        m_chart->clearSeries();
        m_chart->addSeries(m_previewFirst);
        m_chart->addSeries(m_previewSecond);
    }
    if (running())
    {
        m_added += added;
        m_removed += removed;
        ++m_updates;
        m_elapsedMs = m_clock.elapsed();
        m_maximumBatchMs = std::max(m_maximumBatchMs, timer.nsecsElapsed() / 1000000.);
    }
    emit statisticsChanged();
}
void GalleryChartData::clearData()
{
    stopStress();
    if (!m_chart || !m_first || !m_second)
        return;
    m_first->clear();
    m_second->clear();
    m_categories.clear();
    m_firstLabels.clear();
    m_secondLabels.clear();
    m_nextIndex = 0;
    synchronizeMetadata();
    QElapsedTimer timer;
    timer.start();
    finishMutation(0, 0, timer);
}
void GalleryChartData::resetData()
{
    load(m_mode, m_chart, m_previewFirst, m_previewSecond);
}
void GalleryChartData::resetStatistics()
{
    m_added = m_removed = m_updates = m_elapsedMs = 0;
    m_maximumBatchMs = 0;
}
void GalleryChartData::startStress()
{
    if (running() || !m_chart || !m_first || !m_second)
        return;
    resetStatistics();
    m_clock.start();
    m_timer.start();
    emit statisticsChanged();
}
void GalleryChartData::stopStress()
{
    if (!running())
        return;
    m_timer.stop();
    m_elapsedMs = m_clock.elapsed();
    emit statisticsChanged();
}
void GalleryChartData::stressTick()
{
    if (!m_chart || !m_first || !m_second)
    {
        stopStress();
        return;
    }
    // Four additions followed by one deletion exercise growth, eviction and shrinking.
    if (m_updates % 5 == 4 && unitCount() > 0)
        removeData();
    else
        addData();
}

bool GalleryChartData::populateRangeSeries(ChartSeries *series, int count, bool broken,
                                           bool crossed)
{
    if (!series || count < 0 || count > 100000)
        return false;
    QVector<ChartSeries::RangeSample> samples;
    samples.reserve(count);
    for (int i = 0; i < count; ++i)
    {
        const double x = i < 50000 ? i : i + 5000;
        samples.append(
            {x,
             i == 4321 ? -3. : 2 + std::sin(i * .007),
             i == 1234 ? 12. : (crossed ? 2 + 4 * std::cos(i * .008) : 7 + std::cos(i * .008)),
             {},
             broken && i > 0 && i % 10000 == 0});
    }
    return series->replaceRangeSamples(samples);
}
