#include "../../include/builders/library.hpp"
#include "../../include/file.hpp"
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
        } else {
          debug("------------------------");
          debug("%s requires", path.c_str());
          for (const auto a : header.includes) {
            debug("- %s", a.c_str());
          }
        }
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
