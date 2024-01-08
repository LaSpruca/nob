#pragma once
#include "build_config.hpp"
#include <set>
#include <string>

namespace nob {
void error(const char *__restrict __format, ...);
void warn(const char *__restrict __format, ...);
void info(const char *__restrict __format, ...);
void debug(const char *__restrict __format, ...);

enum class CompileResult { SUCCESS, FAILURE };

int command_build(const BuildConfig &builder, const std::string &name);
std::string trim(std::string s);

std::tuple<std::string, std::string> get_current_compilers();

template <typename T>
bool set_includes_all(std::set<T> set, std::vector<T> vector);
} // namespace nob
