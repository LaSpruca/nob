#include "../include/file.hpp"
#include "../include/utils.hpp"
#include <chrono>
#include <filesystem>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

namespace nob {
template <typename Tp> std::time_t to_time_t(Tp tp) {
  using namespace std::chrono;
  auto sctp = time_point_cast<system_clock::duration>(tp - Tp::clock::now() +
                                                      system_clock::now());
  return system_clock::to_time_t(sctp);
}

std::vector<std::string> get_includes(const std::string &source) {
  std::istringstream iss(source);

  std::vector<std::string> lines;
  std::string line;
  while (std::getline(iss, line)) {
    const auto [is_include, path] = get_include_from_line(line);
    if (is_include) {
      lines.push_back(path);
    }
  }

  return lines;
}

std::tuple<bool, std::string> get_include_from_line(const std::string &line) {
  std::string trimmed = trim(line);
  if (trimmed.rfind("#include ") == 0) {
    std::string stripped = trim(trimmed.substr(9));
    return std::tuple{true, stripped.substr(1, stripped.length() - 2)};
  }

  return std::tuple{false, std::string()};
}

bool should_recompile(const fs::path &output_file,
                      const std::vector<fs::path> paths) {
  std::time_t last_build;
  try {
    last_build = to_time_t(fs::last_write_time(output_file));
  } catch (fs::filesystem_error ex) {
    return true;
  }

  for (const auto file : paths) {
    try {
      std::time_t last_write = to_time_t(fs::last_write_time(file));
      if (last_write >= last_build) {
        return true;
      }
    } catch (fs::filesystem_error ex) {
      error("Could not open file %s, %s", file.c_str(), ex.what());
      exit(1);
    }
  }

  return false;
}

} // namespace nob
