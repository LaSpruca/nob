#pragma once
#include "../data.hpp"
#include "../utils.hpp"

namespace nob::library {
CompileResult build(const std::string &name, const Data libData);
}
