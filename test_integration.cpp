#include "../Test.h"
#include "core/JsonLite.h"
#include "core/Controller.h"

static bool runFile(const char* path, core::SummaryMetrics &outSum, core::Battery &outBatt) {
  std::string text;
  if (!core::JsonLite::loadFile(path, text)) return false;
  core::MicrogridModel m;
  std::string err;
  if (!core::JsonLite::parseModel(text, m, err)) return false;
  core::Controller c;
  outSum = c.runSimulation(m, false);
  outBatt = m.battery;
  return true;
}

bool test_integration_sample_invariants() {
  core::SummaryMetrics sum;
  core::Battery batt;
  ASSERT_TRUE(runFile("data/input_sample.json", sum, batt));

  ASSERT_NEAR(sum.unmet_load_kwh, 0.0, 1e-6);
  ASSERT_TRUE(batt.soc >= 0.20 - 1e-9);
  ASSERT_TRUE(batt.soc <= 0.95 + 1e-9);
  ASSERT_TRUE(sum.diesel_kwh >= 0.0);
  ASSERT_TRUE(sum.diesel_kwh < 1e6);
  return true;
}
