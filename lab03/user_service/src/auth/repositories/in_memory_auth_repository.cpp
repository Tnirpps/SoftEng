#include "in_memory_auth_repository.hpp"

#include <regex>
#include <userver/utils/boost_uuid4.hpp>
#include <userver/utils/datetime.hpp>
#include <userver/utils/uuid4.hpp>

#include "models/user.hpp"

namespace Auth::Repositories {

CheckUserResult InMemoryAuthRepository::CheckUser(const std::string &login, const std::string &password) {
    auto data_ptr = users_.Lock();
    auto it = data_ptr->find(login);
    if (it == data_ptr->end() || it->second.password != password) {
        return std::nullopt;
    }
    const auto &user = it->second;
    return Models::User{
        .uuid = user.uuid,
        .login = user.login,
        .first_name = user.first_name,
        .last_name = user.last_name,
        .created_at = user.created_at};
}

AddUserResult InMemoryAuthRepository::AddUser(const std::string &login, const std::string &password, const std::string &first_name, const std::string &last_name) {
    User user{
        .uuid = userver::utils::generators::GenerateUuid(),
        .login = login,
        .password = password,
        .first_name = first_name,
        .last_name = last_name,
        .created_at = userver::utils::datetime::TimePointTz{userver::utils::datetime::Now()}};

    auto data_ptr = users_.Lock();

    auto [it, inserted] = data_ptr->emplace(login, user);

    if (inserted) {
        const auto &user = it->second;
        return Models::User{
            .uuid = user.uuid,
            .login = user.login,
            .first_name = user.first_name,
            .last_name = user.last_name,
            .created_at = user.created_at};
    } else {
        return AddUserError::UserAlreadyExists;
    }
}

SearchUserResult InMemoryAuthRepository::SearchUserByPattern(const std::string &pattern) {
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

    for (const auto &[login, user] : *data_ptr) {
        if (std::regex_match(user.last_name, re)) {
            return Models::User{
                .uuid = user.uuid,
                .login = user.login,
                .first_name = user.first_name,
                .last_name = user.last_name,
                .created_at = user.created_at};
        }
    }
    return std::nullopt;
}

void InMemoryAuthRepository::DeleteAllUsers() {
    auto data_ptr = users_.Lock();
    data_ptr->clear();
}

} // namespace Auth::Repositories
