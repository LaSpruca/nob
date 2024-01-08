#include "nob.hpp"

int configure_build(nob::BuildConfig &builder) {
  nob::Library lib = builder.create_library("nob");
  lib.emit("nob.hpp", nob::library::Emit::HEADER_ONLY);
  lib.set_root(std::filesystem::u8path(__FILE__).parent_path());
  lib.build("src/utils.cpp");
  lib.build("src/library.cpp");
  lib.build("src/build_config.cpp");
  lib.build("src/file.cpp");
  lib.build("src/builders/library.cpp");

  return 0;
}
