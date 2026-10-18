// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "qcatchartseries.h"

namespace QCatUiKit
{
// Publish geometry and statistical provenance together, before dataChanged.
class ChartStatisticsAccess
{
  public:
    static bool publish(ChartSeries *series, const QVariantList &points, bool range);
    static void setVisual(ChartSeries *series, const QVariantMap &visual);
    static QVariantMap visual(const ChartSeries *series);
};
} // namespace QCatUiKit
