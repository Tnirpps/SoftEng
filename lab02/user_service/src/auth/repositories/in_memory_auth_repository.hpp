#pragma once

#include <map>
#include <string>
#include <userver/concurrent/variable.hpp>

#include "auth/auth_repository.hpp"

namespace Auth::Repositories {

class InMemoryAuthRepository : public IAuthRepository {
  public:
    InMemoryAuthRepository() = default;
    explicit InMemoryAuthRepository(const std::map<std::string, std::string> &initial_users);

    CheckUserResult CheckUser(const std::string &login, const std::string &password) override;
    AddUserResult AddUser(const std::string &login, const std::string &password) override;
    bool SearchUserByPattern(const std::string &pattern) override;
    void DeleteAllUsers() override;

  private:
    userver::concurrent::Variable<std::map<std::string, std::string>> users_;
};

} // namespace Auth::Repositories
