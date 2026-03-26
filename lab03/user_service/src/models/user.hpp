#pragma once
#include <string>
#include <userver/utils/datetime/timepoint_tz.hpp>

namespace Models {
struct User {
    std::string uuid;
    std::string login;
    std::string first_name;
    std::string last_name;
    userver::utils::datetime::TimePointTz created_at;
};

} // namespace Models