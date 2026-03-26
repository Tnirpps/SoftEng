#pragma once

#include <map>
#include <string>
#include <userver/concurrent/variable.hpp>

#include "auth/auth_repository.hpp"

namespace Auth::Repositories {

// TODO: convert into database schema
struct User {
    std::string uuid;
    std::string login;
    std::string password;
    std::string first_name;
    std::string last_name;
    userver::utils::datetime::TimePointTz created_at;
};

class InMemoryAuthRepository : public IAuthRepository {
  public:
    InMemoryAuthRepository() = default;

    CheckUserResult CheckUser(const std::string &login, const std::string &password) override;
    AddUserResult AddUser(const std::string &login, const std::string &password, const std::string &first_name, const std::string &last_name) override;
    SearchUserResult SearchUserByPattern(const std::string &pattern) override;
    void DeleteAllUsers() override;

  private:
    userver::concurrent::Variable<std::map<std::string, User>> users_;
};

} // namespace Auth::Repositories
