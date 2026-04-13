#pragma once

#include <string>
#include <boost/uuid/uuid.hpp>
#include <userver/storages/postgres/io/chrono.hpp>
#include <userver/storages/postgres/io/uuid.hpp>
#include <userver/utils/boost_uuid4.hpp>

namespace Models {

enum class FileStatus {
    Pending,
    Scanning,
    Available,
    Infected
};

struct File {
    boost::uuids::uuid uuid;
    std::string name;
    std::int64_t size;
    std::string mime_type;
    boost::uuids::uuid directory_uuid;
    boost::uuids::uuid owner_uuid;
    userver::storages::postgres::TimePointTz created_at;
    userver::storages::postgres::TimePointTz updated_at;
    // FileStatus status;

    // Compatibility getters for string-based API
    std::string GetId() const {
        return userver::utils::ToString(uuid);
    }
    
    std::string GetDirectoryId() const {
        return userver::utils::ToString(directory_uuid);
    }
    
    std::string GetOwnerId() const {
        return userver::utils::ToString(owner_uuid);
    }
};

} // namespace Models
