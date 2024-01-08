#pragma once

#include "data.hpp"
#include <map>
#include <string>

namespace nob {
class BuildFile;
class Library;
namespace {
struct Data;
}

class BuildConfig {
public:
  BuildConfig();
  const void print_targets();
  const void print_executables();

  Library create_library(const std::string name);

private:
  std::map<std::string, library::Data> m_libraries;

  friend int command_build(const BuildConfig &builder,
                           const std::string &target);
  friend class Library;
};

} // namespace nob
