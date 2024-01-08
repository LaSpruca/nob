#pragma once
/******************************************************************************************
nob.hpp was built by nob, do not edit
******************************************************************************************/
#include <filesystem>
#include <string>
#include <vector>

namespace nob {
enum class FileType { C, CPP };

struct BuildFile {
  FileType ft;
  std::filesystem::path path;
  std::vector<std::string> extra_flags;
};

namespace library {
enum class Emit { HEADER_ONLY, STATIC, DYNAMIC };

struct Data {
  library::Emit libraryType;
  std::vector<BuildFile> files;
  std::filesystem::path emitName;
  std::vector<std::filesystem::path> includePath;
  std::vector<std::string> cliArugments;
  std::filesystem::path root;
};
} // namespace library
} // namespace nob


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


namespace nob::library {
CompileResult build(const std::string &name, const Data libData);
}

#ifndef ____nob__IMPL
#define __nob__IMPL

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
#include <cerrno>
#include <cstring>
#include <fstream>
#include <set>
#include <string>

namespace fs = std::filesystem;
namespace nob::library {

struct Header {
  fs::path path;
  std::vector<fs::path> includes;
  std::string content;
};

CompileResult include_headers(fs::path path,
                              std::map<fs::path, Header> &paths) {
  std::vector<fs::path> includes;
  std::stringstream source;
  std::ifstream file(path);

  if (!file.is_open()) {
    error("File %s would not open: %s", path.c_str(), std::strerror(errno));
    return CompileResult::FAILURE;
  }

  std::string line;
  while (std::getline(file, line)) {
    const auto [is_include, header_path] = get_include_from_line(line);
    if (is_include) {
      fs::path included_path = path.parent_path();
      included_path /= header_path;

      if (fs::exists(included_path)) {
        fs::path abs_path = fs::canonical(included_path);
        if (paths.find(abs_path) == paths.end()) {
          if (include_headers(abs_path, paths) == CompileResult::FAILURE) {
            return CompileResult::FAILURE;
          }
        }

        includes.push_back(abs_path);
        continue;
      }
    }

    if (trim(line) == "#pragma once") {
      continue;
    }

    source << line << "\n";
  }

  Header current = {
      .path = path,
      .includes = includes,
      .content = source.str(),
  };

  paths.insert({path, current});

  return CompileResult::SUCCESS;
}

CompileResult build_header_only(const std::string &name, const Data lib_data) {
  std::set<fs::path> headers{};
  std::stringstream implementation;
  std::string impl_macro = "__" + name + "__IMPL";

  implementation << "#ifndef __" << impl_macro
                 << "\n"
                    "#define "
                 << impl_macro << "\n";

  for (const auto file : lib_data.files) {
    fs::path path = lib_data.root;
    path += file.path;
    path = fs::absolute(path);

    info("Building path: %s", path.c_str());

    std::ifstream ifs;
    ifs.open(path);
    if (!ifs.is_open()) {
      error("File %s would not open: %s", path.c_str(), std::strerror(errno));
      return CompileResult::FAILURE;
    }

    std::string line;

    while (std::getline(ifs, line)) {
      const auto [is_include, include_path] = get_include_from_line(line);
      if (is_include) {
        fs::path project_include = path.parent_path();
        project_include /= include_path;
        project_include = std::filesystem::absolute(project_include);

        debug("Including: %s", project_include.c_str());
        if (fs::exists(project_include)) {
          headers.insert(fs::canonical(project_include));
          continue;
        }
      }
      implementation << line << "\n";
    }

    for (const auto item : headers) {
      debug("Found header %s", item.c_str());
    }
  }

  implementation << "#endif";

  std::map<fs::path, Header> header_map{};
  for (const auto header : headers) {
    if (include_headers(header, header_map) == CompileResult::FAILURE) {
      return CompileResult::FAILURE;
    }
  }

  std::ofstream output_file(lib_data.emitName);

  if (!output_file.is_open()) {
    error("File %s would not open: %s", lib_data.emitName.c_str(),
          std::strerror(errno));
    return CompileResult::FAILURE;
  }

  output_file << "#pragma once\n"
                 "/************************************************************"
                 "******************************\n"
              << lib_data.emitName.string()
              << " was built by nob, do not edit\n"
                 "*************************************************************"
                 "*****************************/\n";

  std::set<fs::path> included{};
  if (header_map.size() > 0) {
    while (included.size() < header_map.size()) {
      bool included_once = false;

      for (const auto [path, header] : header_map) {
        if (included.find(path) == included.end() &&
            set_includes_all(included, header.includes)) {
          included_once = true;
          output_file << (header.content) << "\n";
          included.insert(path);
        }
#ifdef NOB_DEBUG
        else {
          debug("------------------------");
          debug("%s requires", path.c_str());
          for (const auto a : header.includes) {
            if (included.find(a) == included.end()) {
              debug("[✗] %s", a.c_str());
            } else {
              debug("[✔] %s", a.c_str());
            }
          }
        }
#endif
      }

      if (!included_once) {
        error("Could not complie due to headers containing cyclical "
              "dependencies");
        for (const auto inc : included) {
          debug("%s", inc.c_str());
        }
        return CompileResult::FAILURE;
      }
    }
  }

  output_file << implementation.str();

  return CompileResult::SUCCESS;
} // namespace nob::library

CompileResult build(const std::string &name, const Data lib_data) {
  switch (lib_data.libraryType) {
  case Emit::HEADER_ONLY:
    return build_header_only(name, lib_data);
  case Emit::STATIC:
  case Emit::DYNAMIC:
    break;
  }
  error("Cannot complie this library type");
  return CompileResult::FAILURE;
}

} // namespace nob::library
#endif