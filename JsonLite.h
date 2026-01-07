#pragma once
#include <string>
#include "core/Model.h"

namespace core {

class JsonLite {
public:
  static bool loadFile(const std::string &path, std::string &outText);
  static bool parseModel(const std::string &text, MicrogridModel &outModel, std::string &err);
};

} // namespace core
