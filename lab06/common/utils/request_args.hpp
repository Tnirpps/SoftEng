#pragma once

#include <optional>
#include <string_view>

namespace Utils {

/**
 * @brief Parse a boolean query argument from string
 * @param value String value of the argument
 * @return std::optional<bool> Parsed boolean value, or std::nullopt if parsing failed
 */
std::optional<bool> ParseBoolArg(std::string_view value);

/**
 * @brief Get boolean query argument from request
 * @param request_arg String value of the query argument
 * @param default_value Default value if argument is empty or not provided
 * @return bool Parsed boolean value or default
 */
bool GetBoolArg(std::string_view request_arg, bool default_value = false);

/**
 * @brief Parse an integer query argument from string
 * @param value String value of the argument
 * @return std::optional<int> Parsed integer value, or std::nullopt if parsing failed
 */
std::optional<int> ParseIntArg(std::string_view value);

} // namespace Utils
