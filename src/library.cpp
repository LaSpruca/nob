#include "../include/library.hpp"

namespace nob {
Library::Library(BuildConfig &config, const std::string name)
    : m_name(name), m_build_config(config) {}

void Library::emit(const std::string name, library::Emit emit_type) {
  library::Data &lib = m_build_config.m_libraries.at(m_name);
  lib.emitName = name;
  lib.libraryType = emit_type;
}

void Library::add_include(const std::string path) {
  library::Data &lib = m_build_config.m_libraries.at(m_name);
  lib.includePath.push_back(path);
}

void Library::build(const std::string path, const FileType ft) {
  library::Data &lib = m_build_config.m_libraries.at(m_name);

  const BuildFile d = {
      .ft = ft,
      .path = std::filesystem::path{std::filesystem::u8path(path)},
      .extra_flags = std::vector<std::string>(),
  };

  lib.files.push_back(d);
}

void Library::set_root(const std::string root) {
  library::Data &lib = m_build_config.m_libraries.at(m_name);
  lib.root = std::filesystem::u8path(root);
}

void Library::set_command_line(const std::vector<std::string> args) {
  library::Data &lib = m_build_config.m_libraries.at(m_name);
  lib.cliArugments = args;
}
} // namespace nob
