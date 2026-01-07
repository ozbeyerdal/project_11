#include "../Test.h"
#include "core/JsonLite.h"

bool test_jsonlite_parse_sample() {
  std::string text;
  ASSERT_TRUE(core::JsonLite::loadFile("data/input_sample.json", text));

  core::MicrogridModel m;
  std::string err;
  ASSERT_TRUE(core::JsonLite::parseModel(text, m, err));

  ASSERT_TRUE(m.sources.size() >= 1);
  ASSERT_TRUE(m.loads.size() >= 1);
  ASSERT_TRUE(m.ts.solar_profile.size() > 0);
  ASSERT_TRUE(m.ts.load_profile.size() > 0);

  ASSERT_TRUE(m.battery.soc >= 0.0 && m.battery.soc <= 1.0);
  return true;
}
