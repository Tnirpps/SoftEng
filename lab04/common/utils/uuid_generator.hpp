#pragma once

#include <string>
#include <userver/utils/boost_uuid4.hpp>
#include <userver/utils/uuid4.hpp>

namespace Utils {

/**
 * @brief Generate a random UUID string
 * @return std::string Generated UUID in string format
 */
inline std::string GenerateUuid() {
    return userver::utils::ToString(
        userver::utils::BoostUuidFromString(userver::utils::generators::GenerateUuid()));
}

} // namespace Utils
