#include <userver/engine/async.hpp>
#include <userver/engine/run_standalone.hpp>
#include <userver/engine/sleep.hpp>
#include <userver/engine/task/task.hpp>
#include <userver/engine/task/task_with_result.hpp>
#include <userver/utest/utest.hpp>
#include <vector>

#include "auth/auth_repository.hpp"
#include "auth/repositories/in_memory_auth_repository.hpp"

using namespace Auth;
using namespace Auth::Repositories;

UTEST(InMemoryAuthRepository, EmptyRepository_CheckUser_ReturnsFalse) {
    Repositories::InMemoryAuthRepository repository;

    EXPECT_FALSE(repository.CheckUser("nonexistent", "password"));
}

UTEST(InMemoryAuthRepository, AddUser_CheckUser_ReturnsTrue) {
    Repositories::InMemoryAuthRepository repository;

    auto result = repository.AddUser("testuser", "testpass", "Test", "User");
    EXPECT_TRUE(std::holds_alternative<Models::User>(result));
    EXPECT_TRUE(repository.CheckUser("testuser", "testpass"));
}

UTEST(InMemoryAuthRepository, AddUser_WrongPassword_ReturnsFalse) {
    Repositories::InMemoryAuthRepository repository;

    auto result = repository.AddUser("testuser", "testpass", "Test", "User");
    EXPECT_TRUE(std::holds_alternative<Models::User>(result));
    EXPECT_FALSE(repository.CheckUser("testuser", "wrongpass"));
}

UTEST(InMemoryAuthRepository, AddUser_DuplicateUser_ReturnsError) {
    Repositories::InMemoryAuthRepository repository;

    auto result1 = repository.AddUser("testuser", "testpass", "Test", "User");
    EXPECT_TRUE(std::holds_alternative<Models::User>(result1));

    auto result2 = repository.AddUser("testuser", "anotherpass", "Test", "User");
    EXPECT_TRUE(std::holds_alternative<AddUserError>(result2));
    EXPECT_EQ(std::get<AddUserError>(result2), AddUserError::UserAlreadyExists);
}

UTEST(InMemoryAuthRepository, AddUser_MultipleUsers_AllAccessible) {
    Repositories::InMemoryAuthRepository repository;

    auto result1 = repository.AddUser("user1", "pass1", "First1", "Last1");
    EXPECT_TRUE(std::holds_alternative<Models::User>(result1));

    auto result2 = repository.AddUser("user2", "pass2", "First2", "Last2");
    EXPECT_TRUE(std::holds_alternative<Models::User>(result2));

    auto result3 = repository.AddUser("user3", "pass3", "First3", "Last3");
    EXPECT_TRUE(std::holds_alternative<Models::User>(result3));

    EXPECT_TRUE(repository.CheckUser("user1", "pass1"));
    EXPECT_TRUE(repository.CheckUser("user2", "pass2"));
    EXPECT_TRUE(repository.CheckUser("user3", "pass3"));

    EXPECT_FALSE(repository.CheckUser("user1", "pass2"));
    EXPECT_FALSE(repository.CheckUser("user2", "pass1"));
}

UTEST(InMemoryAuthRepository, ConcurrentAddUsers_ThreadSafe) {
    Repositories::InMemoryAuthRepository repository;
    const int num_threads = 10;
    std::vector<userver::engine::TaskWithResult<void>> tasks;
    tasks.reserve(num_threads);

    for (int i = 0; i < num_threads; ++i) {
        tasks.push_back(userver::engine::AsyncNoSpan([&repository, i] {
            std::string nick = "user" + std::to_string(i);
            std::string pass = "pass" + std::to_string(i);
            repository.AddUser(std::move(nick), std::move(pass), "First", "Last");
        }));
    }

    for (auto &task : tasks) {
        task.Get();
    }

    for (int i = 0; i < num_threads; ++i) {
        std::string nick = "user" + std::to_string(i);
        std::string pass = "pass" + std::to_string(i);
        EXPECT_TRUE(repository.CheckUser(nick, pass)) << "Failed for " << nick;
    }
}

UTEST(InMemoryAuthRepository, SearchUserByPattern_EmptyRepository_ReturnsNullopt) {
    Repositories::InMemoryAuthRepository repository;

    auto result = repository.SearchUserByPattern("test%");
    EXPECT_FALSE(result.has_value());
}

UTEST(InMemoryAuthRepository, SearchUserByPattern_ExactMatch_ReturnsUser) {
    Repositories::InMemoryAuthRepository repository;

    repository.AddUser("testuser", "testpass", "Test", "Testov");

    auto result = repository.SearchUserByPattern("Testov");
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->last_name, "Testov");
    EXPECT_EQ(result->login, "testuser");
}

UTEST(InMemoryAuthRepository, SearchUserByPattern_WildcardPercent_Matches) {
    Repositories::InMemoryAuthRepository repository;

    repository.AddUser("testuser", "testpass", "Test", "Testov");

    auto result = repository.SearchUserByPattern("Test%");
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->last_name, "Testov");
}

UTEST(InMemoryAuthRepository, SearchUserByPattern_WildcardUnderscore_Matches) {
    Repositories::InMemoryAuthRepository repository;

    repository.AddUser("testuser", "testpass", "Test", "Admin");

    auto result = repository.SearchUserByPattern("A____");
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->last_name, "Admin");
}

UTEST(InMemoryAuthRepository, SearchUserByPattern_NoMatch_ReturnsNullopt) {
    Repositories::InMemoryAuthRepository repository;

    repository.AddUser("testuser", "testpass", "Test", "Testov");

    auto result = repository.SearchUserByPattern("NonExistent%");
    EXPECT_FALSE(result.has_value());
}

UTEST(InMemoryAuthRepository, SearchUserByPattern_CaseInsensitive_Matches) {
    Repositories::InMemoryAuthRepository repository;

    repository.AddUser("testuser", "testpass", "Test", "Testov");

    auto result = repository.SearchUserByPattern("test%");
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->last_name, "Testov");
}

UTEST(InMemoryAuthRepository, SearchUserByPattern_MultipleUsers_ReturnsFirstMatch) {
    Repositories::InMemoryAuthRepository repository;

    repository.AddUser("user1", "pass1", "First1", "Ivanov");
    repository.AddUser("user2", "pass2", "First2", "Petrov");
    repository.AddUser("user3", "pass3", "First3", "Sidorov");

    auto result = repository.SearchUserByPattern("%ov");
    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(result->last_name == "Ivanov" || result->last_name == "Petrov" || result->last_name == "Sidorov");
}
