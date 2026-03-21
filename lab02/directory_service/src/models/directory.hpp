#pragma once

#include <optional>
#include <string>
#include <userver/utils/datetime/timepoint_tz.hpp>

namespace Models {

struct Directory {
    std::string id;
    std::string name;
    std::optional<std::string> parent_id;
    std::string owner_id;
    userver::utils::datetime::TimePointTz created_at;
    userver::utils::datetime::TimePointTz updated_at;
    bool is_root;
};

} // namespace Models
