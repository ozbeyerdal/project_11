#pragma once
#include "core/Model.h"

namespace frontend {

class Tui {
public:
  static void printHeader();
  static void renderStep(int step, double timeHours, const core::StepMetrics &s);
  static void printSummary(const core::SummaryMetrics &sum, const core::Battery &batt);
};

} // namespace frontend
