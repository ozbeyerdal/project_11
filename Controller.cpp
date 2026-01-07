#include "core/Controller.h"
#include "frontend/Tui.h"

namespace core {

Controller::Controller() {}

double Controller::stepHours(const SimulationConfig &cfg) const {
  return (double)cfg.time_step_minutes / 60.0;
}

double Controller::clamp(double v, double lo, double hi) const {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

double Controller::windMultiplier(int step) const {
  unsigned int x = (unsigned int)(step * 1103515245u + 12345u);
  x = (x / 65536u) % 32768u;
  double r = (double)x / 32768.0;
  return 0.2 + 0.8 * r;
}

double Controller::totalLoadKw(const MicrogridModel &m, int step) const {
  int len = m.ts.load_profile.size();
  double dt = stepHours(m.cfg);
  int totalSteps = (int)(m.cfg.duration_hours / dt);
  if (totalSteps <= 0) totalSteps = 1;
  int idx = (step * len) / totalSteps;
  if (idx < 0) idx = 0;
  if (idx >= len) idx = len - 1;

  double mult = m.ts.load_profile[idx];
  double sum = 0.0;
  for (int i = 0; i < m.loads.size(); ++i) sum += m.loads[i].base_kw * mult;
  return sum;
}

double Controller::totalRenewKw(const MicrogridModel &m, int step) const {
  int len = m.ts.solar_profile.size();
  double dt = stepHours(m.cfg);
  int totalSteps = (int)(m.cfg.duration_hours / dt);
  if (totalSteps <= 0) totalSteps = 1;
  int idx = (step * len) / totalSteps;
  if (idx < 0) idx = 0;
  if (idx >= len) idx = len - 1;

  double solarMult = m.ts.solar_profile[idx];
  double windMult = windMultiplier(step);

  double sum = 0.0;
  for (int i = 0; i < m.sources.size(); ++i) {
    const Source &src = m.sources[i];
    if (src.type == SourceType::Solar) sum += src.peak_kw * solarMult;
    else if (src.type == SourceType::Wind) sum += src.peak_kw * windMult;
  }
  return sum;
}

double Controller::chooseReserveSoc(const MicrogridModel &m, int step, int lookaheadSteps) const {
  double dt = stepHours(m.cfg);
  int totalSteps = (int)(m.cfg.duration_hours / dt);
  if (totalSteps <= 0) totalSteps = 1;

  double deficit_kwh = 0.0;
  for (int k = 1; k <= lookaheadSteps; ++k) {
    int s = step + k;
    if (s >= totalSteps) break;
    double net = totalLoadKw(m, s) - totalRenewKw(m, s);
    if (net > 0) deficit_kwh += net * dt;
  }

  double frac = 0.0;
  if (m.battery.capacity_kwh > 0) frac = deficit_kwh / m.battery.capacity_kwh;

  double reserve = m.battery.min_soc + clamp(frac, 0.0, 0.6);
  if (reserve > 0.8) reserve = 0.8;
  if (reserve < m.battery.min_soc) reserve = m.battery.min_soc;
  return reserve;
}

void Controller::updateDegradation(const Battery &before, const Battery &after, SummaryMetrics &accum) const {
  double d = before.soc - after.soc;
  if (after.soc < 0.30) accum.degradation_score += (0.30 - after.soc) * 10.0;
  if (d > 0.10) accum.degradation_score += (d - 0.10) * 5.0;
}

void Controller::dispatchStep(MicrogridModel &m, int step, double reserveSoc, StepMetrics &outStep,
                              SummaryMetrics &accum) const {
  double dt = stepHours(m.cfg);
  Battery before = m.battery;

  double load_kw = totalLoadKw(m, step);
  double renew_kw = totalRenewKw(m, step);

  double remaining = load_kw;

  // renewables first
  double used_renew = (renew_kw > remaining) ? remaining : renew_kw;
  remaining -= used_renew;
  double surplus = renew_kw - used_renew;

  // charge battery
  double chg_kw = 0.0;
  if (surplus > 0.0 && m.battery.capacity_kwh > 0.0) {
    double maxByRate = m.battery.charge_rate_kw;
    double maxBySoc = (m.battery.max_soc - m.battery.soc) * m.battery.capacity_kwh / dt;
    if (maxBySoc < 0) maxBySoc = 0;
    double lim = clamp(maxByRate, 0.0, maxBySoc);
    chg_kw = clamp(surplus, 0.0, lim);
    m.battery.soc += (chg_kw * dt) / m.battery.capacity_kwh;
    if (m.battery.soc > m.battery.max_soc) m.battery.soc = m.battery.max_soc;
  }

  // discharge battery with reserve
  double dis_kw = 0.0;
  if (remaining > 0.0 && m.battery.capacity_kwh > 0.0) {
    double availableSoc = m.battery.soc - reserveSoc;
    if (availableSoc < 0) availableSoc = 0;
    double maxBySoc = availableSoc * m.battery.capacity_kwh / dt;
    double maxByRate = m.battery.discharge_rate_kw;
    double can = clamp(maxByRate, 0.0, maxBySoc);
    dis_kw = (remaining < can) ? remaining : can;
    m.battery.soc -= (dis_kw * dt) / m.battery.capacity_kwh;
    if (m.battery.soc < m.battery.min_soc) m.battery.soc = m.battery.min_soc;
    remaining -= dis_kw;
  }

  // diesel last
  double diesel_kw = 0.0;
  if (remaining > 0.0) {
    double dieselCap = 0.0;
    for (int i = 0; i < m.sources.size(); ++i)
      if (m.sources[i].type == SourceType::Diesel) dieselCap += m.sources[i].capacity_kw;
    diesel_kw = (remaining < dieselCap) ? remaining : dieselCap;
    remaining -= diesel_kw;
  }

  double unmet = remaining;

  outStep.gen_renew_kw = renew_kw;
  outStep.load_kw = load_kw;
  outStep.batt_charge_kw = chg_kw;
  outStep.batt_discharge_kw = dis_kw;
  outStep.diesel_kw = diesel_kw;
  outStep.soc = m.battery.soc;
  outStep.unmet_kw = unmet;

  accum.served_load_kwh += (load_kw - unmet) * dt;
  accum.renew_used_kwh += used_renew * dt;
  accum.batt_charge_kwh += chg_kw * dt;
  accum.batt_discharge_kwh += dis_kw * dt;
  accum.diesel_kwh += diesel_kw * dt;
  accum.unmet_load_kwh += unmet * dt;

  updateDegradation(before, m.battery, accum);
}

SummaryMetrics Controller::runSimulation(MicrogridModel &model, bool tui) {
  SummaryMetrics sum;
  double dt = stepHours(model.cfg);
  int totalSteps = (int)(model.cfg.duration_hours / dt);
  if (totalSteps <= 0) totalSteps = 1;

  if (tui) frontend::Tui::printHeader();

  for (int step = 0; step < totalSteps; ++step) {
    double reserve = chooseReserveSoc(model, step, 8);
    StepMetrics s;
    dispatchStep(model, step, reserve, s, sum);
    if (tui) frontend::Tui::renderStep(step, step * dt, s);
  }

  if (tui) frontend::Tui::printSummary(sum, model.battery);
  return sum;
}

} // namespace core
