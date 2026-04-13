#pragma once

#include <optional>
#include <string>
#include <boost/uuid/uuid.hpp>
#include <userver/storages/postgres/io/chrono.hpp>
#include <userver/storages/postgres/io/uuid.hpp>
#include <userver/utils/boost_uuid4.hpp>

namespace Models {

struct Directory {
    boost::uuids::uuid uuid;
    std::string name;
    std::optional<boost::uuids::uuid> parent_uuid;
    boost::uuids::uuid owner_uuid;
    userver::storages::postgres::TimePointTz created_at;
    userver::storages::postgres::TimePointTz updated_at;
    bool is_root;

    // Compatibility getters for string-based API
    std::string GetId() const {
        return userver::utils::ToString(uuid);
    }
    
    std::optional<std::string> GetParentId() const {
        if (!parent_uuid.has_value()) {
            return std::nullopt;
        }
        return userver::utils::ToString(parent_uuid.value());
    }
    
    std::string GetOwnerId() const {
        return userver::utils::ToString(owner_uuid);
    }
};

} // namespace Models
