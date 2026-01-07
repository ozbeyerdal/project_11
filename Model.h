#pragma once
#include <string>
#include "data_structures/LinkedList.h"

namespace core {

enum class SourceType { Solar, Wind, Diesel };

struct Source {
  std::string id;
  SourceType type;
  double peak_kw;
  double capacity_kw;
  Source() : id(), type(SourceType::Solar), peak_kw(0), capacity_kw(0) {}
};

struct Battery {
  std::string id;
  double capacity_kwh;
  double soc;
  double charge_rate_kw;
  double discharge_rate_kw;
  double min_soc;
  double max_soc;
  Battery()
      : id(), capacity_kwh(0), soc(0.5), charge_rate_kw(0), discharge_rate_kw(0),
        min_soc(0.20), max_soc(0.95) {}
};

struct Load {
  std::string id;
  double base_kw;
  Load() : id(), base_kw(0) {}
};

struct TimeSeries {
  LinkedList<double> solar_profile;
  LinkedList<double> load_profile;
};

struct SimulationConfig {
  int time_step_minutes;
  int duration_hours;
  SimulationConfig() : time_step_minutes(15), duration_hours(24) {}
};

struct MicrogridModel {
  LinkedList<Source> sources;
  Battery battery;
  LinkedList<Load> loads;
  TimeSeries ts;
  SimulationConfig cfg;
};

struct StepMetrics {
  double gen_renew_kw, load_kw, batt_charge_kw, batt_discharge_kw, diesel_kw, soc, unmet_kw;
  StepMetrics()
      : gen_renew_kw(0), load_kw(0), batt_charge_kw(0), batt_discharge_kw(0),
        diesel_kw(0), soc(0), unmet_kw(0) {}
};

struct SummaryMetrics {
  double served_load_kwh, renew_used_kwh, batt_charge_kwh, batt_discharge_kwh;
  double diesel_kwh, unmet_load_kwh, degradation_score;
  SummaryMetrics()
      : served_load_kwh(0), renew_used_kwh(0), batt_charge_kwh(0), batt_discharge_kwh(0),
        diesel_kwh(0), unmet_load_kwh(0), degradation_score(0) {}
};

} // namespace core
