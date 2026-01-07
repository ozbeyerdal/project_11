#include "core/JsonLite.h"
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstdlib>

namespace core {

static void skipWs(const std::string &s, size_t &i) {
  while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
}

static bool expectChar(const std::string &s, size_t &i, char c) {
  skipWs(s, i);
  if (i >= s.size() || s[i] != c) return false;
  ++i;
  return true;
}

static bool parseString(const std::string &s, size_t &i, std::string &out) {
  skipWs(s, i);
  if (i >= s.size() || s[i] != '"') return false;
  ++i;
  std::string r;
  while (i < s.size() && s[i] != '"') {
    if (s[i] == '\\' && i + 1 < s.size()) {
      char n = s[i + 1];
      if (n == '"' || n == '\\' || n == '/') { r.push_back(n); i += 2; continue; }
    }
    r.push_back(s[i++]);
  }
  if (i >= s.size() || s[i] != '"') return false;
  ++i;
  out = r;
  return true;
}

static bool parseNumber(const std::string &s, size_t &i, double &out) {
  skipWs(s, i);
  if (i >= s.size()) return false;
  size_t start = i;
  if (s[i] == '-') ++i;
  bool any = false;
  while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) { any = true; ++i; }
  if (i < s.size() && s[i] == '.') {
    ++i;
    while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) { any = true; ++i; }
  }
  if (!any) return false;
  out = std::atof(s.substr(start, i - start).c_str());
  return true;
}

static bool skipValue(const std::string &s, size_t &i);

static bool skipArray(const std::string &s, size_t &i) {
  if (!expectChar(s, i, '[')) return false;
  skipWs(s, i);
  if (i < s.size() && s[i] == ']') { ++i; return true; }
  while (i < s.size()) {
    if (!skipValue(s, i)) return false;
    skipWs(s, i);
    if (i < s.size() && s[i] == ',') { ++i; continue; }
    if (i < s.size() && s[i] == ']') { ++i; return true; }
    return false;
  }
  return false;
}

static bool skipObject(const std::string &s, size_t &i) {
  if (!expectChar(s, i, '{')) return false;
  skipWs(s, i);
  if (i < s.size() && s[i] == '}') { ++i; return true; }
  while (i < s.size()) {
    std::string k;
    if (!parseString(s, i, k)) return false;
    if (!expectChar(s, i, ':')) return false;
    if (!skipValue(s, i)) return false;
    skipWs(s, i);
    if (i < s.size() && s[i] == ',') { ++i; continue; }
    if (i < s.size() && s[i] == '}') { ++i; return true; }
    return false;
  }
  return false;
}

static bool skipValue(const std::string &s, size_t &i) {
  skipWs(s, i);
  if (i >= s.size()) return false;
  if (s[i] == '{') return skipObject(s, i);
  if (s[i] == '[') return skipArray(s, i);
  if (s[i] == '"') { std::string tmp; return parseString(s, i, tmp); }
  if (s[i] == '-' || std::isdigit(static_cast<unsigned char>(s[i]))) { double d; return parseNumber(s, i, d); }
  if (s.compare(i, 4, "true") == 0) { i += 4; return true; }
  if (s.compare(i, 5, "false") == 0) { i += 5; return true; }
  if (s.compare(i, 4, "null") == 0) { i += 4; return true; }
  return false;
}

static bool parseNumberArray(const std::string &s, size_t &i, LinkedList<double> &out) {
  if (!expectChar(s, i, '[')) return false;
  skipWs(s, i);
  if (i < s.size() && s[i] == ']') { ++i; return true; }
  while (i < s.size()) {
    double d = 0;
    if (!parseNumber(s, i, d)) return false;
    out.push_back(d);
    skipWs(s, i);
    if (i < s.size() && s[i] == ',') { ++i; continue; }
    if (i < s.size() && s[i] == ']') { ++i; return true; }
    return false;
  }
  return false;
}

static SourceType parseSourceType(const std::string &t) {
  if (t == "solar") return SourceType::Solar;
  if (t == "wind") return SourceType::Wind;
  return SourceType::Diesel;
}

static bool parseSources(const std::string &s, size_t &i, LinkedList<Source> &out) {
  if (!expectChar(s, i, '[')) return false;
  skipWs(s, i);
  if (i < s.size() && s[i] == ']') { ++i; return true; }
  while (i < s.size()) {
    if (!expectChar(s, i, '{')) return false;
    Source src;
    while (true) {
      std::string key;
      if (!parseString(s, i, key)) return false;
      if (!expectChar(s, i, ':')) return false;

      if (key == "id") {
        if (!parseString(s, i, src.id)) return false;
      } else if (key == "type") {
        std::string ty; if (!parseString(s, i, ty)) return false;
        src.type = parseSourceType(ty);
      } else if (key == "peak_kw") {
        double d=0; if (!parseNumber(s, i, d)) return false; src.peak_kw = d;
      } else if (key == "capacity_kw") {
        double d=0; if (!parseNumber(s, i, d)) return false; src.capacity_kw = d;
      } else {
        if (!skipValue(s, i)) return false;
      }

      skipWs(s, i);
      if (i < s.size() && s[i] == ',') { ++i; skipWs(s, i); continue; }
      if (i < s.size() && s[i] == '}') { ++i; break; }
      return false;
    }
    out.push_back(src);
    skipWs(s, i);
    if (i < s.size() && s[i] == ',') { ++i; continue; }
    if (i < s.size() && s[i] == ']') { ++i; return true; }
    return false;
  }
  return false;
}

static bool parseLoads(const std::string &s, size_t &i, LinkedList<Load> &out) {
  if (!expectChar(s, i, '[')) return false;
  skipWs(s, i);
  if (i < s.size() && s[i] == ']') { ++i; return true; }
  while (i < s.size()) {
    if (!expectChar(s, i, '{')) return false;
    Load ld;
    while (true) {
      std::string key;
      if (!parseString(s, i, key)) return false;
      if (!expectChar(s, i, ':')) return false;

      if (key == "id") {
        if (!parseString(s, i, ld.id)) return false;
      } else if (key == "base_kw") {
        double d=0; if (!parseNumber(s, i, d)) return false; ld.base_kw = d;
      } else {
        if (!skipValue(s, i)) return false;
      }

      skipWs(s, i);
      if (i < s.size() && s[i] == ',') { ++i; skipWs(s, i); continue; }
      if (i < s.size() && s[i] == '}') { ++i; break; }
      return false;
    }
    out.push_back(ld);
    skipWs(s, i);
    if (i < s.size() && s[i] == ',') { ++i; continue; }
    if (i < s.size() && s[i] == ']') { ++i; return true; }
    return false;
  }
  return false;
}

static bool parseStorageSingle(const std::string &s, size_t &i, Battery &out) {
  if (!expectChar(s, i, '[')) return false;
  skipWs(s, i);
  if (!expectChar(s, i, '{')) return false;

  Battery b;
  while (true) {
    std::string key;
    if (!parseString(s, i, key)) return false;
    if (!expectChar(s, i, ':')) return false;

    if (key == "id") {
      if (!parseString(s, i, b.id)) return false;
    } else if (key == "capacity_kwh") {
      double d=0; if (!parseNumber(s, i, d)) return false; b.capacity_kwh = d;
    } else if (key == "soc") {
      double d=0; if (!parseNumber(s, i, d)) return false; b.soc = d;
    } else if (key == "charge_rate") {
      double d=0; if (!parseNumber(s, i, d)) return false; b.charge_rate_kw = d;
    } else if (key == "discharge_rate") {
      double d=0; if (!parseNumber(s, i, d)) return false; b.discharge_rate_kw = d;
    } else {
      if (!skipValue(s, i)) return false;
    }

    skipWs(s, i);
    if (i < s.size() && s[i] == ',') { ++i; skipWs(s, i); continue; }
    if (i < s.size() && s[i] == '}') { ++i; break; }
    return false;
  }

  // skip any remaining objects until closing ']'
  skipWs(s, i);
  while (i < s.size() && s[i] != ']') {
    if (s[i] == ',') { ++i; skipWs(s, i); continue; }
    if (i < s.size() && s[i] == '{') { if (!skipObject(s, i)) return false; }
    else break;
    skipWs(s, i);
  }
  if (!expectChar(s, i, ']')) return false;

  b.min_soc = 0.20;
  b.max_soc = 0.95;
  out = b;
  return true;
}

static bool parseSimulation(const std::string &s, size_t &i, SimulationConfig &out) {
  if (!expectChar(s, i, '{')) return false;
  SimulationConfig cfg;
  while (true) {
    std::string key;
    if (!parseString(s, i, key)) return false;
    if (!expectChar(s, i, ':')) return false;

    if (key == "time_step_minutes") {
      double d=0; if (!parseNumber(s, i, d)) return false; cfg.time_step_minutes = (int)d;
    } else if (key == "duration_hours") {
      double d=0; if (!parseNumber(s, i, d)) return false; cfg.duration_hours = (int)d;
    } else {
      if (!skipValue(s, i)) return false;
    }

    skipWs(s, i);
    if (i < s.size() && s[i] == ',') { ++i; skipWs(s, i); continue; }
    if (i < s.size() && s[i] == '}') { ++i; break; }
    return false;
  }
  out = cfg;
  return true;
}

bool JsonLite::loadFile(const std::string &path, std::string &outText) {
  std::ifstream ifs(path.c_str());
  if (!ifs) return false;
  std::ostringstream oss;
  oss << ifs.rdbuf();
  outText = oss.str();
  return true;
}

bool JsonLite::parseModel(const std::string &text, MicrogridModel &outModel, std::string &err) {
  size_t i = 0;
  skipWs(text, i);
  if (!expectChar(text, i, '{')) { err = "Expected root object"; return false; }

  MicrogridModel m;

  while (true) {
    skipWs(text, i);
    if (i < text.size() && text[i] == '}') { ++i; break; }

    std::string key;
    if (!parseString(text, i, key)) { err = "Expected key"; return false; }
    if (!expectChar(text, i, ':')) { err = "Expected ':'"; return false; }

    if (key == "sources") {
      if (!parseSources(text, i, m.sources)) { err = "Bad sources"; return false; }
    } else if (key == "storage") {
      if (!parseStorageSingle(text, i, m.battery)) { err = "Bad storage"; return false; }
    } else if (key == "loads") {
      if (!parseLoads(text, i, m.loads)) { err = "Bad loads"; return false; }
    } else if (key == "time_series") {
      if (!expectChar(text, i, '{')) { err = "Bad time_series"; return false; }
      while (true) {
        std::string k2;
        if (!parseString(text, i, k2)) { err = "Bad time_series key"; return false; }
        if (!expectChar(text, i, ':')) { err = "Bad time_series ':'"; return false; }

        if (k2 == "solar_profile") {
          if (!parseNumberArray(text, i, m.ts.solar_profile)) { err = "Bad solar_profile"; return false; }
        } else if (k2 == "load_profile") {
          if (!parseNumberArray(text, i, m.ts.load_profile)) { err = "Bad load_profile"; return false; }
        } else {
          if (!skipValue(text, i)) { err = "Bad time_series value"; return false; }
        }

        skipWs(text, i);
        if (i < text.size() && text[i] == ',') { ++i; skipWs(text, i); continue; }
        if (i < text.size() && text[i] == '}') { ++i; break; }
        err = "Bad time_series object";
        return false;
      }
    } else if (key == "simulation") {
      if (!parseSimulation(text, i, m.cfg)) { err = "Bad simulation"; return false; }
    } else {
      if (!skipValue(text, i)) { err = "Bad value"; return false; }
    }

    skipWs(text, i);
    if (i < text.size() && text[i] == ',') { ++i; continue; }
    if (i < text.size() && text[i] == '}') { ++i; break; }
  }

  if (m.sources.empty()) { err = "No sources"; return false; }
  if (m.loads.empty()) { err = "No loads"; return false; }
  if (m.ts.solar_profile.empty() || m.ts.load_profile.empty()) { err = "Missing profiles"; return false; }

  outModel = m;
  return true;
}

} // namespace core
