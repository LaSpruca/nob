#include "../include/build_config.hpp"
#include "../include/library.hpp"
#include "../include/utils.hpp"

namespace nob {

BuildConfig::BuildConfig() {
  m_libraries = std::map<std::string, library::Data>();
}

Library BuildConfig::create_library(const std::string name) {
  if (m_libraries.find(name) != m_libraries.end()) {

    error("Connot create library \"%s\" becasue a library with that name "
          "alread exists",
          name.c_str());
  }

  std::filesystem::path base_path =
      std::filesystem::u8path(__BASE_FILE__).parent_path();

  library::Data data = {.libraryType = library::Emit::STATIC,
                        .files = std::vector<BuildFile>(),
                        .emitName = "build/outputs/" + name + ".a",
                        .includePath = std::vector<std::filesystem::path>(),
                        .cliArugments = std::vector<std::string>(),
                        .root = base_path};

  m_libraries.insert({name, data});

  return Library(*this, name);
} // namespace nob

const void BuildConfig::print_targets() {}
const void BuildConfig::print_executables() {}

} // namespace nob
