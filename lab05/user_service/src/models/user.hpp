#pragma once
#include <boost/uuid/uuid.hpp>
#include <string>
#include <userver/storages/postgres/io/chrono.hpp>
#include <userver/storages/postgres/io/uuid.hpp>

namespace Models {
struct User {
    boost::uuids::uuid uuid;
    std::string login;
    std::string first_name;
    std::string last_name;
    userver::storages::postgres::TimePointTz created_at;
};

} // namespace Models
