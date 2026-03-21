// #include <userver/engine/task/task_with_result.hpp>
// #include <userver/utest/utest.hpp>
// #include <userver/engine/run_standalone.hpp>
// #include <userver/engine/task/task.hpp>
// #include <userver/utest/utest.hpp>
// #include <userver/engine/async.hpp>
// #include <userver/engine/sleep.hpp>
// #include <vector>



// UTEST(InMemoryAuthRepository, AddUser_CheckUser_ReturnsTrue) {
//     Repositories::InMemoryAuthRepository repository;
    
//     auto result = repository.AddUser("testuser", "testpass");
//     EXPECT_TRUE(std::holds_alternative<Models::User>(result));
//     EXPECT_TRUE(repository.CheckUser("testuser", "testpass"));
// }

// UTEST(InMemoryAuthRepository, ConcurrentAddUsers_ThreadSafe) {
//     Repositories::InMemoryAuthRepository repository;
//     const int num_threads = 10;
//     std::vector<userver::engine::TaskWithResult<void>> tasks;
//     tasks.reserve(num_threads);

//     for (int i = 0; i < num_threads; ++i) {
//         tasks.push_back(userver::engine::AsyncNoSpan([&repository, i] {
//             std::string nick = "user" + std::to_string(i);
//             std::string pass = "pass" + std::to_string(i);
//             repository.AddUser(std::move(nick), std::move(pass));
//         }));
//     }

//     for (auto& task : tasks) {
//         task.Get();
//     }

//     for (int i = 0; i < num_threads; ++i) {
//         std::string nick = "user" + std::to_string(i);
//         std::string pass = "pass" + std::to_string(i);
//         EXPECT_TRUE(repository.CheckUser(nick, pass)) << "Failed for " << nick;
//     }
// }
