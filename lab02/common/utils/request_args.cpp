#include "request_args.hpp"

#include <algorithm>
#include <cctype>

namespace Utils {

std::optional<bool> ParseBoolArg(std::string_view value) {
    if (value.empty()) {
        return std::nullopt;
    }

    // Convert to lowercase for comparison
    std::string lower_value{value};
    std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (lower_value == "true" || lower_value == "1" || lower_value == "yes") {
        return true;
    }

    if (lower_value == "false" || lower_value == "0" || lower_value == "no") {
        return false;
    }

    return std::nullopt;
}

bool GetBoolArg(std::string_view request_arg, bool default_value) {
    if (request_arg.empty()) {
        return default_value;
    }

    auto parsed = ParseBoolArg(request_arg);
    return parsed.value_or(default_value);
}

} // namespace Utils
