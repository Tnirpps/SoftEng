#include <userver/engine/task/task_with_result.hpp>
#include <userver/utest/utest.hpp>
#include <userver/engine/run_standalone.hpp>
#include <userver/engine/task/task.hpp>
#include <userver/utest/utest.hpp>
#include <userver/engine/async.hpp>
#include <userver/engine/sleep.hpp>
#include <vector>
#include <vector>

#include "auth/auth_repository.hpp"

using namespace auth;

/**
 * @brief Unit tests for InMemoryAuthRepository
 * 
 * These tests verify the thread-safe in-memory authentication repository
 * without requiring full component infrastructure.
 */

UTEST(InMemoryAuthRepository, EmptyRepository_CheckUser_ReturnsFalse) {
    InMemoryAuthRepository repository;
    
    EXPECT_FALSE(repository.CheckUser("nonexistent", "password"));
}

UTEST(InMemoryAuthRepository, AddUser_CheckUser_ReturnsTrue) {
    InMemoryAuthRepository repository;
    
    EXPECT_TRUE(repository.AddUser("testuser", "testpass"));
    EXPECT_TRUE(repository.CheckUser("testuser", "testpass"));
}

UTEST(InMemoryAuthRepository, AddUser_WrongPassword_ReturnsFalse) {
    InMemoryAuthRepository repository;
    
    EXPECT_TRUE(repository.AddUser("testuser", "testpass"));
    EXPECT_FALSE(repository.CheckUser("testuser", "wrongpass"));
}

UTEST(InMemoryAuthRepository, AddUser_DuplicateUser_ReturnsFalse) {
    InMemoryAuthRepository repository;
    
    EXPECT_TRUE(repository.AddUser("testuser", "testpass"));
    EXPECT_FALSE(repository.AddUser("testuser", "anotherpass"));
}

UTEST(InMemoryAuthRepository, AddUser_MultipleUsers_AllAccessible) {
    InMemoryAuthRepository repository;
    
    EXPECT_TRUE(repository.AddUser("user1", "pass1"));
    EXPECT_TRUE(repository.AddUser("user2", "pass2"));
    EXPECT_TRUE(repository.AddUser("user3", "pass3"));
    
    EXPECT_TRUE(repository.CheckUser("user1", "pass1"));
    EXPECT_TRUE(repository.CheckUser("user2", "pass2"));
    EXPECT_TRUE(repository.CheckUser("user3", "pass3"));
    
    EXPECT_FALSE(repository.CheckUser("user1", "pass2"));
    EXPECT_FALSE(repository.CheckUser("user2", "pass1"));
}

UTEST(InMemoryAuthRepository, ConstructorWithInitialUsers) {
    std::map<std::string, std::string> initial_users = {
        {"admin", "adminpass"},
        {"guest", "guestpass"}
    };
    
    InMemoryAuthRepository repository(initial_users);
    
    EXPECT_TRUE(repository.CheckUser("admin", "adminpass"));
    EXPECT_TRUE(repository.CheckUser("guest", "guestpass"));
    EXPECT_FALSE(repository.CheckUser("admin", "guestpass"));
}

/**
 * @brief Test thread-safety of concurrent AddUser operations
 */
UTEST(InMemoryAuthRepository, ConcurrentAddUsers_ThreadSafe) {
    InMemoryAuthRepository repository;
    const int num_threads = 10;
    std::vector<userver::engine::TaskWithResult<void>> tasks;
    tasks.reserve(num_threads);

    // Внутри UTEST мы уже находимся в контексте движка userver (coroutine environment)
    for (int i = 0; i < num_threads; ++i) {
        // Используем engine::Async для запуска корутин
        tasks.push_back(userver::engine::AsyncNoSpan([&repository, i] {
            std::string nick = "user" + std::to_string(i);
            std::string pass = "pass" + std::to_string(i);
            repository.AddUser(std::move(nick), std::move(pass));
        }));
    }

    // Ждем завершения всех задач и пробрасываем исключения, если они были
    for (auto& task : tasks) {
        task.Get(); 
    }

    // Проверяем результат
    for (int i = 0; i < num_threads; ++i) {
        std::string nick = "user" + std::to_string(i);
        std::string pass = "pass" + std::to_string(i);
        EXPECT_TRUE(repository.CheckUser(nick, pass)) << "Failed for " << nick;
    }
}

