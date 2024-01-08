#pragma once
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
