#pragma once

#include <string>

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/logging/log.hpp>

#include "auth/auth_repository.hpp"
#include <UserService/sql_queries.hpp>

namespace Auth::Repositories {

class PostgresAuthRepository : public IAuthRepository {
  public:
    explicit PostgresAuthRepository(userver::storages::postgres::ClusterPtr cluster);

    CheckUserResult CheckUser(const std::string &login, const std::string &password) override;
    AddUserResult AddUser(const std::string &login, const std::string &password, const std::string &first_name, const std::string &last_name) override;
    SearchUserResult SearchUserByPattern(const std::string &pattern) override;
    void DeleteAllUsers() override;

  private:
    userver::storages::postgres::ClusterPtr cluster_;
};

} // namespace Auth::Repositories
