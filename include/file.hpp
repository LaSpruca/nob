#pragma once

#include <ctime>
#include <filesystem>
#include <string>
#include <tuple>
#include <vector>

namespace nob {
template <typename Tp> std::time_t to_time_t(Tp tp);
std::vector<std::string> get_includes(const std::string &source);
std::tuple<bool, std::string> get_include_from_line(const std::string &line);
bool should_recompile(const std::filesystem::path &output_file,
                      const std::vector<std::filesystem::path> paths);
} // namespace nob
