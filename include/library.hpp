#pragma once

#include "build_config.hpp"

namespace nob {
class Library {
public:
  Library(BuildConfig &build_config, const std::string name);
  void emit(const std::string name, library::Emit = library::Emit::DYNAMIC);
  void add_include(const std::string path);
  void build(const std::string path, const FileType ft = FileType::CPP);
  void set_command_line(const std::vector<std::string> args);
  void set_root(const std::string root);

private:
  BuildConfig &m_build_config;
  std::string m_name;
};
} // namespace nob
