#pragma once

#include <string>

namespace darix {

constexpr const char* DARIX_VERSION = "1.0.0";

inline std::string versionString() {
    return std::string("DariX (C++) v") + DARIX_VERSION;
}

} // namespace darix
