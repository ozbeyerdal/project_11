#include "core/JsonLite.h"
#include "core/Controller.h"
#include <iostream>

static void usage() {
  std::cout << "Usage: ./bin/microgrid data/input.json [--tui]\n";
}

int main(int argc, char **argv) {
  if (argc < 2) { usage(); return 1; }
  std::string path = argv[1];
  bool tui = false;
  if (argc >= 3) { std::string opt = argv[2]; if (opt == "--tui") tui = true; }

  std::string text;
  if (!core::JsonLite::loadFile(path, text)) {
    std::cerr << "Failed to read: " << path << "\n";
    return 1;
  }

  core::MicrogridModel model;
  std::string err;
  if (!core::JsonLite::parseModel(text, model, err)) {
    std::cerr << "Parse error: " << err << "\n";
    return 1;
  }

  core::Controller ctrl;
  core::SummaryMetrics sum = ctrl.runSimulation(model, tui);

  if (!tui) {
    std::cout << "Served Load (kWh):       " << sum.served_load_kwh << "\n";
    std::cout << "Renewables Used (kWh):   " << sum.renew_used_kwh << "\n";
    std::cout << "Battery Charge (kWh):    " << sum.batt_charge_kwh << "\n";
    std::cout << "Battery Discharge (kWh): " << sum.batt_discharge_kwh << "\n";
    std::cout << "Diesel Used (kWh):       " << sum.diesel_kwh << "\n";
    std::cout << "Unmet Load (kWh):        " << sum.unmet_load_kwh << "\n";
    std::cout << "Degradation Score:       " << sum.degradation_score << "\n";
    std::cout << "Final SoC:               " << model.battery.soc << "\n";
  }
  return (sum.unmet_load_kwh > 1e-6) ? 2 : 0;
}
