#include "runtime/value.hpp"
#include <sstream>
#include <iomanip>

namespace darix {

std::string FloatValue::toString() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6) << value_;
    std::string str = oss.str();
    // Remove trailing zeros
    size_t dot = str.find('.');
    if (dot != std::string::npos) {
        size_t last = str.find_last_not_of('0');
        if (last != std::string::npos && last > dot) {
            str = str.substr(0, last + 1);
        } else if (last == dot) {
            str = str.substr(0, dot);
        }
    }
    return str;
}

} // namespace darix
