#pragma once

#include <string>
#include <userver/utils/datetime/timepoint_tz.hpp>

namespace Models {

enum class FileStatus {
    Pending,
    Scanning,
    Available,
    Infected
};

struct File {
    std::string id;
    std::string name;
    std::int64_t size;
    std::string mime_type;
    std::string directory_id;
    std::string owner_id;
    userver::utils::datetime::TimePointTz created_at;
    userver::utils::datetime::TimePointTz updated_at;
    FileStatus status;
};

} // namespace Models
