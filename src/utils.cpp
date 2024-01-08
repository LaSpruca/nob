#include "../include/utils.hpp"
#include "../include/build_config.hpp"
#include "../include/builders/library.hpp"
#include "../include/file.hpp"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>

namespace nob {
void error(const char *__restrict __format, ...) {
  std::va_list args;
  va_start(args, __format);
  std::fprintf(stderr, "[ERROR] ");
  std::vfprintf(stderr, __format, args);
  std::fprintf(stderr, "\n");
}

void warn(const char *__restrict __format, ...) {
  std::va_list args;
  va_start(args, __format);
  std::fprintf(stderr, "[WARNING] ");
  std::vfprintf(stderr, __format, args);
  std::fprintf(stderr, "\n");
}

void info(const char *__restrict __format, ...) {
  std::va_list args;
  va_start(args, __format);
  std::fprintf(stderr, "[INFO] ");
  std::vfprintf(stderr, __format, args);
  std::fprintf(stderr, "\n");
}

void debug(const char *__restrict __format, ...) {
#ifdef NOB_DEBUG
  std::va_list args;
  va_start(args, __format);
  std::fprintf(stderr, "[DEBUG] ");
  std::vfprintf(stderr, __format, args);
  std::fprintf(stderr, "\n");
#endif
}

std::tuple<std::string, std::string> get_current_compilers() {
#if defined(__clang__)
  return std::tuple{"clang", "clang++"};
#elif defined(__GNUC__) || defined(__GNUG__)
  return std::tuple{"gcc", "g++"};
#elif defined(_MSC_VER)
  return std::tuple{"cl.exe", "cl.exe"};
#endif
}

void rebuild_self(int argc, char **argv, bool invoke_self) {
  info("Rebuilding NOB");
  const auto [_, cpp] = get_current_compilers();
  const auto cmdline = std::getenv("NOB_CMDLINE");
  std::ostringstream ss;
  ss << cpp << " " __BASE_FILE__ " -o nob";
  if (cmdline != nullptr) {
    ss << " " << cmdline;
  }
  std::string command = ss.str();
  info("Executing: %s", command.c_str());
  std::system(command.c_str());

  if (!invoke_self) {
    std::exit(0);
  }

  ss = std::ostringstream();

  for (int i = 0; i < argc; i++) {
    ss << "\"" << argv[i] << "\""
       << " ";
  }

  command = ss.str();

  info("Executing: %s", command.c_str());
  std::system(command.c_str());
  std::exit(0);
}

std::string trim(std::string s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
          }));

  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); })
              .base(),
          s.end());
  return s;
}

int command_build(const BuildConfig &config, const std::string &name) {
  try {
    CompileResult res = library::build(name, config.m_libraries.at(name));
    if (res == CompileResult::FAILURE) {
      error("Could not compile %s", name.c_str());
      return 1;
    }
    return 0;
  } catch (std::out_of_range _) {
    error("Could not find target %s", name.c_str());
  }
  return 0;
}

template <typename T>
bool set_includes_all(std::set<T> set, std::vector<T> vector) {
  for (const auto elm : vector) {
    if (set.find(elm) == set.end()) {
      return false;
    }
  }

  return true;
}

bool set_includes_all(std::set<std::filesystem::path> set,
                      std::vector<std::filesystem::path> vector) {
  return set_includes_all<std::filesystem::path>(set, vector);
}
} // namespace nob

int configure_build(nob::BuildConfig &config);

int main(int argc, char **argv) {
  if (argc < 2) {
    nob::error("Please specify an opperation");
    std::cout << "Opperations: \n"
                 "\tbuild, b <target> build a target\n"
                 "\trun, r <binary> build and run a binary\n"
                 "\trebuild, R rebuild nob"
              << std::endl;
    return 1;
  }

  std::string command(argv[1]);
  if (nob::should_recompile(
          argv[0], std::vector{std::filesystem::path(__BASE_FILE__)}) ||
      nob::should_recompile(argv[0],
                            std::vector{std::filesystem::path(__FILE__)}) ||
      command == "rebuild" || command == "R") {
    nob::rebuild_self(argc, argv, !(command == "rebuild" || command == "R"));
  }

  nob::BuildConfig config;
  configure_build(config);

  if (command == "build" || command == "b") {
    if (argc < 3) {
      nob::error("Please specify a target to build");
      return 1;
    }

    return nob::command_build(config, argv[2]);
  } else {
    nob::error("Unknown command %s", argv[2]);
    std::cout << "Opperations: \n"
                 "\tbuild, b <target> build a target\n"
                 "\trun, r <binary> build and run a binary"
              << std::endl;
    return 1;
  }
}
