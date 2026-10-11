// SPDX-License-Identifier: MIT
#include "gallerychartdata.h"
#include <QCoreApplication>
#include <QDateTime>
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
         "Two signals, 100,000 samples each. Start live to stream; zoom or drag to inspect "
         "history."},
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
        chart->setTitle("Live signals - 100,000 samples per series");
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
        for (int y = 0; y < 40; ++y)
            for (int x = 0; x < 100; ++x)
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
    QT_TRANSLATE_NOOP("GalleryChartData", "Live signals - 100,000 samples per series"),
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
    QT_TRANSLATE_NOOP("GalleryChartData", "Two signals, 100,000 samples each. Start live to "
                                          "stream; zoom or drag to inspect history."),
    QT_TRANSLATE_NOOP("GalleryChartData", "Usability"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Value"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Visits"),
    QT_TRANSLATE_NOOP("GalleryChartData", "Wind"),
};
}

GalleryChartData::GalleryChartData(QObject *parent) : QObject(parent) {}
void GalleryChartData::load(int mode, ChartItem *chart, ChartSeries *first, ChartSeries *second)
{
    if (!chart || !first || !second || mode < 0 || mode >= GalleryDemos::Count)
        return;
    m_mode = mode;
    m_chart = chart;
    m_first = first;
    m_second = second;
    GalleryDemos::populate(mode, chart, first, second);
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
}
void GalleryChartData::retranslate()
{
    if (!m_chart || !m_first || !m_second)
        return;
    const auto translated = [](const QString &source)
    { return QCoreApplication::translate("GalleryChartData", source.toUtf8().constData()); };
    emit metadataChanged();
    m_chart->setTitle(translated(m_title));
    m_chart->xAxis()->setTitle(translated(m_xTitle));
    m_chart->yAxis()->setTitle(translated(m_yTitle));
    QStringList categories;
    for (const auto &category : m_categories)
        categories.append(translated(category));
    m_chart->xAxis()->setCategories(categories);
    m_first->setName(translated(m_firstName));
    m_second->setName(translated(m_secondName));
    for (auto pair :
         {qMakePair(m_first.data(), &m_firstLabels), qMakePair(m_second.data(), &m_secondLabels)})
    {
        if (pair.second->isEmpty() || pair.second->size() != pair.first->count())
            continue;
        QVector<ChartSeries::Sample> samples;
        for (int i = 0; i < pair.first->count(); ++i)
        {
            auto sample = pair.first->sampleAt(i);
            const auto &label = pair.second->at(i);
            if (label.startsWith("Core "))
                sample.label = translated("Core %1").arg(i + 1);
            else if (label.startsWith("New "))
                sample.label = translated("New %1").arg(i + 1);
            else
                sample.label = translated(label);
            samples.append(sample);
        }
        pair.first->replaceSamples(samples);
    }
}
void GalleryChartData::advance()
{
    if (!m_first || !m_second)
        return;
    const double start = m_first->count() ? m_first->sampleAt(m_first->count() - 1).x + 1 : 0;
    QVector<ChartSeries::Sample> a, b;
    for (int i = 0; i < 17; ++i)
    {
        const double x = start + i;
        a.append({x, std::sin(x * .007) * 8 + std::sin(x * .057) * 2, 1, {}});
        b.append({x, std::cos(x * .008) * 5, 1, {}});
    }
    m_first->appendSamples(a);
    m_second->appendSamples(b);
}

QString GalleryChartData::hint() const
{
    return QCoreApplication::translate("GalleryChartData", GalleryDemos::description(m_mode).hint);
}
