#pragma once
#include "core/Model.h"

namespace core {

class Controller {
public:
  Controller();
  SummaryMetrics runSimulation(MicrogridModel &model, bool tui);

private:
  double stepHours(const SimulationConfig &cfg) const;
  double clamp(double v, double lo, double hi) const;
  double windMultiplier(int step) const;

  double totalLoadKw(const MicrogridModel &m, int step) const;
  double totalRenewKw(const MicrogridModel &m, int step) const;

  double chooseReserveSoc(const MicrogridModel &m, int step, int lookaheadSteps) const;

  void dispatchStep(MicrogridModel &m, int step, double reserveSoc, StepMetrics &outStep,
                    SummaryMetrics &accum) const;

  void updateDegradation(const Battery &before, const Battery &after, SummaryMetrics &accum) const;
};

} // namespace core
