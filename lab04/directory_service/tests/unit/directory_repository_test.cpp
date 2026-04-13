// #include <algorithm>
// #include <userver/engine/async.hpp>
// #include <userver/engine/run_standalone.hpp>
// #include <userver/engine/sleep.hpp>
// #include <userver/engine/task/task.hpp>
// #include <userver/engine/task/task_with_result.hpp>
// #include <userver/utest/utest.hpp>
// #include <vector>

// #include "repositories/in_memory_directory_repository.hpp"

// UTEST(InMemoryDirectoryRepository, CreateDirectory_GetDirectory_ReturnsDirectory) {
//     Repositories::InMemoryDirectoryRepository repository;

//     const std::string dir_name = "test_dir";
//     const std::string owner_id = "owner-uuid-123";
//     const std::optional<std::string> parent_id = std::nullopt;

//     auto result = repository.CreateDirectory(dir_name, parent_id, owner_id);
//     EXPECT_TRUE(result.has_value());

//     const auto &dir = result.value();
//     EXPECT_EQ(dir.name, dir_name);
//     EXPECT_EQ(dir.owner_id, owner_id);
//     EXPECT_EQ(dir.is_root, true);

//     auto get_result = repository.GetDirectory(dir.id);
//     EXPECT_TRUE(get_result.has_value());
//     EXPECT_EQ(get_result.value().name, dir_name);
// }

// UTEST(InMemoryDirectoryRepository, CreateDirectory_DuplicateName_ReturnsError) {
//     Repositories::InMemoryDirectoryRepository repository;

//     const std::string dir_name = "duplicate_dir";
//     const std::string owner_id = "owner-uuid-456";

//     // Create parent directory first
//     const std::optional<std::string> grandparent_id = std::nullopt;
//     auto parent_result = repository.CreateDirectory("parent_dir", grandparent_id, owner_id);
//     ASSERT_TRUE(parent_result.has_value());

//     const std::optional<std::string> parent_id = parent_result.value().id;

//     auto result1 = repository.CreateDirectory(dir_name, parent_id, owner_id);
//     EXPECT_TRUE(result1.has_value());

//     auto result2 = repository.CreateDirectory(dir_name, parent_id, owner_id);
//     EXPECT_FALSE(result2.has_value());
//     EXPECT_EQ(result2.error(), Repositories::CreateDirectoryError::DirectoryAlreadyExists);
// }

// UTEST(InMemoryDirectoryRepository, CreateDirectory_SameNameDifferentParent_Success) {
//     Repositories::InMemoryDirectoryRepository repository;

//     const std::string dir_name = "shared_dir";
//     const std::string owner_id = "owner-uuid-789";

//     // Create parent directories first
//     const std::optional<std::string> grandparent_id = std::nullopt;
//     auto parent1_result = repository.CreateDirectory("parent1", grandparent_id, owner_id);
//     auto parent2_result = repository.CreateDirectory("parent2", grandparent_id, owner_id);
//     ASSERT_TRUE(parent1_result.has_value());
//     ASSERT_TRUE(parent2_result.has_value());

//     const std::optional<std::string> parent_id_1 = parent1_result.value().id;
//     const std::optional<std::string> parent_id_2 = parent2_result.value().id;

//     auto result1 = repository.CreateDirectory(dir_name, parent_id_1, owner_id);
//     EXPECT_TRUE(result1.has_value());

//     auto result2 = repository.CreateDirectory(dir_name, parent_id_2, owner_id);
//     EXPECT_TRUE(result2.has_value());

//     EXPECT_NE(result1.value().id, result2.value().id);
// }

// UTEST(InMemoryDirectoryRepository, CreateDirectory_ParentNotFound_ReturnsError) {
//     Repositories::InMemoryDirectoryRepository repository;

//     const std::string dir_name = "child_dir";
//     const std::string owner_id = "owner-uuid-parent";
//     const std::optional<std::string> parent_id = "non-existent-parent";

//     auto result = repository.CreateDirectory(dir_name, parent_id, owner_id);
//     EXPECT_FALSE(result.has_value());
//     EXPECT_EQ(result.error(), Repositories::CreateDirectoryError::ParentNotFound);
// }

// UTEST(InMemoryDirectoryRepository, GetDirectory_NotFound_ReturnsEmpty) {
//     Repositories::InMemoryDirectoryRepository repository;

//     auto result = repository.GetDirectory("non-existent-uuid");
//     EXPECT_FALSE(result.has_value());
// }

// UTEST(InMemoryDirectoryRepository, UpdateDirectory_Success) {
//     Repositories::InMemoryDirectoryRepository repository;

//     const std::string dir_name = "original_dir";
//     const std::string owner_id = "owner-uuid-update";
//     const std::optional<std::string> parent_id = std::nullopt;

//     auto create_result = repository.CreateDirectory(dir_name, parent_id, owner_id);
//     ASSERT_TRUE(create_result.has_value());

//     const std::string new_name = "renamed_dir";
//     auto update_result = repository.UpdateDirectory(create_result.value().id, new_name, owner_id);
//     EXPECT_TRUE(update_result.has_value());
//     EXPECT_EQ(update_result.value().name, new_name);
// }

// UTEST(InMemoryDirectoryRepository, UpdateDirectory_NotFound_ReturnsError) {
//     Repositories::InMemoryDirectoryRepository repository;

//     auto result = repository.UpdateDirectory("non-existent-uuid", "newname", "owner-uuid");
//     EXPECT_FALSE(result.has_value());
//     EXPECT_EQ(result.error(), Repositories::UpdateDirectoryError::DirectoryNotFound);
// }

// UTEST(InMemoryDirectoryRepository, UpdateDirectory_NameConflict_ReturnsError) {
//     Repositories::InMemoryDirectoryRepository repository;

//     const std::string owner_id = "owner-uuid-conflict";
//     const std::optional<std::string> parent_id = std::nullopt;

//     auto result1 = repository.CreateDirectory("dir1", parent_id, owner_id);
//     auto result2 = repository.CreateDirectory("dir2", parent_id, owner_id);
//     ASSERT_TRUE(result1.has_value());
//     ASSERT_TRUE(result2.has_value());

//     auto update_result = repository.UpdateDirectory(result2.value().id, "dir1", owner_id);
//     EXPECT_FALSE(update_result.has_value());
//     EXPECT_EQ(update_result.error(), Repositories::UpdateDirectoryError::NameConflict);
// }

// UTEST(InMemoryDirectoryRepository, DeleteDirectory_Success) {
//     Repositories::InMemoryDirectoryRepository repository;

//     const std::string dir_name = "todelete_dir";
//     const std::string owner_id = "owner-uuid-delete";
//     const std::optional<std::string> parent_id = std::nullopt;

//     auto create_result = repository.CreateDirectory(dir_name, parent_id, owner_id);
//     ASSERT_TRUE(create_result.has_value());

//     auto delete_result = repository.DeleteDirectory(create_result.value().id, false, owner_id);
//     EXPECT_TRUE(delete_result.has_value());
//     EXPECT_TRUE(delete_result.value());

//     auto get_result = repository.GetDirectory(create_result.value().id);
//     EXPECT_FALSE(get_result.has_value());
// }

// UTEST(InMemoryDirectoryRepository, DeleteDirectory_NotFound_ReturnsError) {
//     Repositories::InMemoryDirectoryRepository repository;

//     auto result = repository.DeleteDirectory("non-existent-uuid", false, "owner-uuid");
//     EXPECT_FALSE(result.has_value());
//     EXPECT_EQ(result.error(), Repositories::DeleteDirectoryError::DirectoryNotFound);
// }

// UTEST(InMemoryDirectoryRepository, DeleteDirectory_WithChildren_NotRecursive_ReturnsError) {
//     Repositories::InMemoryDirectoryRepository repository;

//     const std::string owner_id = "owner-uuid-children";
//     const std::optional<std::string> parent_id = std::nullopt;

//     auto parent_result = repository.CreateDirectory("parent_dir", parent_id, owner_id);
//     ASSERT_TRUE(parent_result.has_value());

//     const std::optional<std::string> child_parent_id = parent_result.value().id;
//     auto child_result = repository.CreateDirectory("child_dir", child_parent_id, owner_id);
//     ASSERT_TRUE(child_result.has_value());

//     auto delete_result = repository.DeleteDirectory(parent_result.value().id, false, owner_id);
//     EXPECT_FALSE(delete_result.has_value());
//     EXPECT_EQ(delete_result.error(), Repositories::DeleteDirectoryError::DirectoryNotEmpty);
// }

// UTEST(InMemoryDirectoryRepository, DeleteDirectory_WithChildren_Recursive_Success) {
//     Repositories::InMemoryDirectoryRepository repository;

//     const std::string owner_id = "owner-uuid-recursive";
//     const std::optional<std::string> parent_id = std::nullopt;

//     auto parent_result = repository.CreateDirectory("parent_dir", parent_id, owner_id);
//     ASSERT_TRUE(parent_result.has_value());

//     const std::optional<std::string> child_parent_id = parent_result.value().id;
//     auto child_result = repository.CreateDirectory("child_dir", child_parent_id, owner_id);
//     ASSERT_TRUE(child_result.has_value());

//     auto delete_result = repository.DeleteDirectory(parent_result.value().id, true, owner_id);
//     EXPECT_TRUE(delete_result.has_value());
//     EXPECT_TRUE(delete_result.value());

//     auto get_parent = repository.GetDirectory(parent_result.value().id);
//     EXPECT_FALSE(get_parent.has_value());
//     auto get_child = repository.GetDirectory(child_result.value().id);
//     EXPECT_FALSE(get_child.has_value());
// }

// UTEST(InMemoryDirectoryRepository, MoveDirectory_Success) {
//     Repositories::InMemoryDirectoryRepository repository;

//     const std::string owner_id = "owner-uuid-move";
//     const std::optional<std::string> parent_id = std::nullopt;

//     auto dir_result = repository.CreateDirectory("move_dir", parent_id, owner_id);
//     auto new_parent_result = repository.CreateDirectory("new_parent", parent_id, owner_id);
//     ASSERT_TRUE(dir_result.has_value());
//     ASSERT_TRUE(new_parent_result.has_value());

//     const std::optional<std::string> new_parent_id = new_parent_result.value().id;
//     auto move_result = repository.MoveDirectory(dir_result.value().id, new_parent_id, owner_id);
//     EXPECT_TRUE(move_result.has_value());
//     EXPECT_EQ(move_result.value().parent_id, new_parent_id);
//     EXPECT_EQ(move_result.value().is_root, false);
// }

// UTEST(InMemoryDirectoryRepository, MoveDirectory_NotFound_ReturnsError) {
//     Repositories::InMemoryDirectoryRepository repository;

//     auto result = repository.MoveDirectory("non-existent-uuid", std::nullopt, "owner-uuid");
//     EXPECT_FALSE(result.has_value());
//     EXPECT_EQ(result.error(), Repositories::MoveDirectoryError::DirectoryNotFound);
// }

// UTEST(InMemoryDirectoryRepository, MoveDirectory_WouldCreateCycle_ReturnsError) {
//     Repositories::InMemoryDirectoryRepository repository;

//     const std::string owner_id = "owner-uuid-cycle";
//     const std::optional<std::string> parent_id = std::nullopt;

//     auto parent_result = repository.CreateDirectory("parent_dir", parent_id, owner_id);
//     ASSERT_TRUE(parent_result.has_value());

//     const std::optional<std::string> child_parent_id = parent_result.value().id;
//     auto child_result = repository.CreateDirectory("child_dir", child_parent_id, owner_id);
//     ASSERT_TRUE(child_result.has_value());

//     // Try to move parent into child (would create cycle)
//     const std::optional<std::string> new_parent_id = child_result.value().id;
//     auto move_result = repository.MoveDirectory(parent_result.value().id, new_parent_id, owner_id);
//     EXPECT_FALSE(move_result.has_value());
//     EXPECT_EQ(move_result.error(), Repositories::MoveDirectoryError::WouldCreateCycle);
// }

// UTEST(InMemoryDirectoryRepository, DeleteAll_Success) {
//     Repositories::InMemoryDirectoryRepository repository;

//     const std::string owner_id = "owner-uuid-deleteall";
//     const std::optional<std::string> parent_id = std::nullopt;

//     repository.CreateDirectory("dir1", parent_id, owner_id);
//     repository.CreateDirectory("dir2", parent_id, owner_id);
//     repository.CreateDirectory("dir3", parent_id, owner_id);

//     repository.DeleteAll();

//     auto get_result = repository.GetDirectory("dir1");
//     EXPECT_FALSE(get_result.has_value());
// }

// UTEST(InMemoryDirectoryRepository, ConcurrentCreateDirectories_ThreadSafe) {
//     Repositories::InMemoryDirectoryRepository repository;
//     const int num_threads = 10;
//     std::vector<userver::engine::TaskWithResult<std::string>> tasks;
//     tasks.reserve(num_threads);

//     const std::string owner_id = "owner-uuid-concurrent";
//     const std::optional<std::string> parent_id = std::nullopt;

//     for (int i = 0; i < num_threads; ++i) {
//         tasks.push_back(userver::engine::AsyncNoSpan([&repository, i, owner_id, parent_id]() -> std::string {
//             std::string name = "dir" + std::to_string(i);
//             auto result = repository.CreateDirectory(std::move(name), parent_id, owner_id);
//             return result.has_value() ? result.value().id : "";
//         }));
//     }

//     std::vector<std::string> dir_ids;
//     for (auto &task : tasks) {
//         auto id = task.Get();
//         EXPECT_FALSE(id.empty());
//         dir_ids.push_back(id);
//     }

//     // Verify all directories have unique IDs
//     std::sort(dir_ids.begin(), dir_ids.end());
//     auto unique_end = std::unique(dir_ids.begin(), dir_ids.end());
//     EXPECT_EQ(std::distance(dir_ids.begin(), unique_end), num_threads);
// }
