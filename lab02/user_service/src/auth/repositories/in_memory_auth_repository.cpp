#include "in_memory_auth_repository.hpp"

#include <regex>
#include <userver/utils/datetime.hpp>

#include "models/user.hpp"

namespace Auth::Repositories {

InMemoryAuthRepository::InMemoryAuthRepository(const std::map<std::string, std::string> &initial_users)
    : users_(initial_users) {
}

CheckUserResult InMemoryAuthRepository::CheckUser(const std::string &login, const std::string &password) {
    auto data_ptr = users_.Lock();
    auto it = data_ptr->find(login);
    if (it == data_ptr->end() || it->second != password) {
        return std::nullopt;
    }
    return Models::User{
        .login = login,
        .first_name = "",
        .last_name = "",
        .created_at = userver::utils::datetime::TimePointTz{userver::utils::datetime::Now()}};
}

AddUserResult InMemoryAuthRepository::AddUser(const std::string &login, const std::string &password) {
    auto data_ptr = users_.Lock();
    auto [it, inserted] = data_ptr->emplace(login, password);

    if (inserted) {
        return Models::User{
            .login = login,
            .first_name = "",
            .last_name = "",
            .created_at = userver::utils::datetime::TimePointTz{userver::utils::datetime::Now()}};
    } else {
        return AddUserError::UserAlreadyExists;
    }
}

bool InMemoryAuthRepository::SearchUserByPattern(const std::string &pattern) {
    auto data_ptr = users_.Lock();

    std::string regex_pattern = "^";
    for (char c : pattern) {
        if (c == '%') {
            regex_pattern += ".*";
        } else if (c == '_') {
            regex_pattern += ".";
        } else {
            regex_pattern += std::regex_replace(std::string(1, c), std::regex(R"([\^$.|?*+(){}\\])"), R"(\$&)");
        }
    }
    regex_pattern += "$";

    std::regex re(regex_pattern, std::regex::icase);

    for (const auto &[login, _] : *data_ptr) {
        if (std::regex_match(login, re)) {
            return true;
        }
    }
    return false;
}

void InMemoryAuthRepository::DeleteAllUsers() {
    auto data_ptr = users_.Lock();
    data_ptr->clear();
}

} // namespace Auth::Repositories
