#include "frontend/Tui.h"
#include <iostream>
#include <iomanip>

namespace frontend {

static void bar(const char *label, double value, double scale, int width) {
  int n = 0;
  if (scale > 0) n = (int)((value / scale) * width);
  if (n < 0) n = 0;
  if (n > width) n = width;
  std::cout << label << " ";
  for (int i = 0; i < n; ++i) std::cout << '#';
  for (int i = n; i < width; ++i) std::cout << '.';
}

void Tui::printHeader() {
  std::cout << "Step | Time(h) | Load(kW) | Renew(kW) | Chg(kW) | Dis(kW) | Diesel(kW) | SoC  | Unmet\n";
  std::cout << "---------------------------------------------------------------------------------------------\n";
}

void Tui::renderStep(int step, double timeHours, const core::StepMetrics &s) {
  std::cout << std::setw(4) << step << " | "
            << std::setw(7) << std::fixed << std::setprecision(2) << timeHours << " | "
            << std::setw(8) << std::setprecision(2) << s.load_kw << " | "
            << std::setw(8) << s.gen_renew_kw << " | "
            << std::setw(7) << s.batt_charge_kw << " | "
            << std::setw(7) << s.batt_discharge_kw << " | "
            << std::setw(9) << s.diesel_kw << " | "
            << std::setw(4) << std::setprecision(2) << s.soc << " | "
            << std::setw(5) << std::setprecision(2) << s.unmet_kw << "\n";

  bar("     L", s.load_kw, 100.0, 20);
  std::cout << "  ";
  bar("R", s.gen_renew_kw, 100.0, 20);
  std::cout << "  ";
  bar("D", s.diesel_kw, 100.0, 10);
  std::cout << "\n";
}

void Tui::printSummary(const core::SummaryMetrics &sum, const core::Battery &batt) {
  std::cout << "\n=== SUMMARY ===\n";
  std::cout << "Served Load (kWh):       " << sum.served_load_kwh << "\n";
  std::cout << "Renewables Used (kWh):   " << sum.renew_used_kwh << "\n";
  std::cout << "Battery Charge (kWh):    " << sum.batt_charge_kwh << "\n";
  std::cout << "Battery Discharge (kWh): " << sum.batt_discharge_kwh << "\n";
  std::cout << "Diesel Used (kWh):       " << sum.diesel_kwh << "\n";
  std::cout << "Unmet Load (kWh):        " << sum.unmet_load_kwh << "\n";
  std::cout << "Degradation Score:       " << sum.degradation_score << "\n";
  std::cout << "Final SoC:               " << batt.soc << "\n";
}

} // namespace frontend
