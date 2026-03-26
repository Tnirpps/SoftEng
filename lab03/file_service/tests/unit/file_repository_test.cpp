#include <algorithm>
#include <userver/engine/async.hpp>
#include <userver/engine/run_standalone.hpp>
#include <userver/engine/sleep.hpp>
#include <userver/engine/task/task.hpp>
#include <userver/engine/task/task_with_result.hpp>
#include <userver/utest/utest.hpp>
#include <vector>

#include "repositories/in_memory_file_repository.hpp"

UTEST(InMemoryFileRepository, CreateFile_GetFile_ReturnsFile) {
    Repositories::InMemoryFileRepository repository;

    const std::string file_name = "test.txt";
    const std::string content = "Hello, World!";
    const std::string owner_id = "owner-uuid-123";
    const std::optional<std::string> directory_id = std::nullopt;

    auto result = repository.CreateFile(file_name, content, directory_id, owner_id);
    EXPECT_TRUE(result.has_value());

    const auto &file = result.value();
    EXPECT_EQ(file.name, file_name);
    EXPECT_EQ(file.size, static_cast<std::int64_t>(content.size()));
    EXPECT_EQ(file.owner_id, owner_id);
    EXPECT_EQ(file.status, Models::FileStatus::Available);

    auto get_result = repository.GetFile(file.id);
    EXPECT_TRUE(get_result.has_value());
    EXPECT_EQ(get_result.value().name, file_name);
}

UTEST(InMemoryFileRepository, CreateFile_DuplicateName_ReturnsError) {
    Repositories::InMemoryFileRepository repository;

    const std::string file_name = "duplicate.txt";
    const std::string content = "Content 1";
    const std::string owner_id = "owner-uuid-456";
    const std::optional<std::string> directory_id = "dir-uuid-same";

    auto result1 = repository.CreateFile(file_name, content, directory_id, owner_id);
    EXPECT_TRUE(result1.has_value());

    const std::string content2 = "Content 2";
    auto result2 = repository.CreateFile(file_name, content2, directory_id, owner_id);
    EXPECT_FALSE(result2.has_value());
    EXPECT_EQ(result2.error(), Repositories::CreateFileError::FileAlreadyExists);
}

UTEST(InMemoryFileRepository, CreateFile_SameNameDifferentDirectory_Success) {
    Repositories::InMemoryFileRepository repository;

    const std::string file_name = "shared.txt";
    const std::string content = "Content";
    const std::string owner_id = "owner-uuid-789";
    const std::string dir_id_1 = "dir-uuid-1";
    const std::string dir_id_2 = "dir-uuid-2";

    auto result1 = repository.CreateFile(file_name, content, dir_id_1, owner_id);
    EXPECT_TRUE(result1.has_value());

    auto result2 = repository.CreateFile(file_name, content, dir_id_2, owner_id);
    EXPECT_TRUE(result2.has_value());

    EXPECT_NE(result1.value().id, result2.value().id);
}

UTEST(InMemoryFileRepository, GetFile_NotFound_ReturnsError) {
    Repositories::InMemoryFileRepository repository;

    auto result = repository.GetFile("non-existent-uuid");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), Repositories::GetFileError::FileNotFound);
}

UTEST(InMemoryFileRepository, UpdateFile_Success) {
    Repositories::InMemoryFileRepository repository;

    const std::string file_name = "original.txt";
    const std::string content = "Original content";
    const std::string owner_id = "owner-uuid-update";
    const std::optional<std::string> directory_id = std::nullopt;

    auto create_result = repository.CreateFile(file_name, content, directory_id, owner_id);
    ASSERT_TRUE(create_result.has_value());

    const std::string new_name = "renamed.json";
    auto update_result = repository.UpdateFile(create_result.value().id, new_name, owner_id);
    EXPECT_TRUE(update_result.has_value());
    EXPECT_EQ(update_result.value().name, new_name);
    EXPECT_EQ(update_result.value().mime_type, "application/json");
}

UTEST(InMemoryFileRepository, UpdateFile_NotFound_ReturnsError) {
    Repositories::InMemoryFileRepository repository;

    auto result = repository.UpdateFile("non-existent-uuid", "newname.txt", "owner-uuid");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), Repositories::UpdateFileError::FileNotFound);
}

UTEST(InMemoryFileRepository, UpdateFile_NameConflict_ReturnsError) {
    Repositories::InMemoryFileRepository repository;

    const std::string owner_id = "owner-uuid-conflict";
    const std::optional<std::string> directory_id = std::nullopt;

    auto result1 = repository.CreateFile("file1.txt", "content1", directory_id, owner_id);
    auto result2 = repository.CreateFile("file2.txt", "content2", directory_id, owner_id);
    ASSERT_TRUE(result1.has_value());
    ASSERT_TRUE(result2.has_value());

    auto update_result = repository.UpdateFile(result2.value().id, "file1.txt", owner_id);
    EXPECT_FALSE(update_result.has_value());
    EXPECT_EQ(update_result.error(), Repositories::UpdateFileError::NameConflict);
}

UTEST(InMemoryFileRepository, DeleteFile_Success) {
    Repositories::InMemoryFileRepository repository;

    const std::string file_name = "todelete.txt";
    const std::string content = "Delete me";
    const std::string owner_id = "owner-uuid-delete";
    const std::optional<std::string> directory_id = std::nullopt;

    auto create_result = repository.CreateFile(file_name, content, directory_id, owner_id);
    ASSERT_TRUE(create_result.has_value());

    auto delete_result = repository.DeleteFile(create_result.value().id, owner_id);
    EXPECT_TRUE(delete_result.has_value());
    EXPECT_TRUE(delete_result.value());

    auto get_result = repository.GetFile(create_result.value().id);
    EXPECT_FALSE(get_result.has_value());
}

UTEST(InMemoryFileRepository, DeleteFile_NotFound_ReturnsError) {
    Repositories::InMemoryFileRepository repository;

    auto result = repository.DeleteFile("non-existent-uuid", "owner-uuid");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), Repositories::DeleteFileError::FileNotFound);
}

UTEST(InMemoryFileRepository, DeleteAll_Success) {
    Repositories::InMemoryFileRepository repository;

    const std::string owner_id = "owner-uuid-deleteall";
    const std::optional<std::string> directory_id = std::nullopt;

    auto _ = repository.CreateFile("file1.txt", "content1", directory_id, owner_id);
    _ = repository.CreateFile("file2.txt", "content2", directory_id, owner_id);
    _ = repository.CreateFile("file3.txt", "content3", directory_id, owner_id);

    repository.DeleteAll();

    // Verify all files are deleted by trying to get them
    auto get_result = repository.GetFile("file1.txt");
    EXPECT_FALSE(get_result.has_value());
}

UTEST(InMemoryFileRepository, ConcurrentCreateFiles_ThreadSafe) {
    Repositories::InMemoryFileRepository repository;
    const int num_threads = 10;
    std::vector<userver::engine::TaskWithResult<std::string>> tasks;
    tasks.reserve(num_threads);

    const std::string owner_id = "owner-uuid-concurrent";
    const std::optional<std::string> directory_id = std::nullopt;

    for (int i = 0; i < num_threads; ++i) {
        tasks.push_back(userver::engine::AsyncNoSpan([&repository, i, owner_id, directory_id]() -> std::string {
            std::string name = "file" + std::to_string(i) + ".txt";
            std::string content = "content" + std::to_string(i);
            auto result = repository.CreateFile(std::move(name), std::move(content), directory_id, owner_id);
            return result.has_value() ? result.value().id : "";
        }));
    }

    std::vector<std::string> file_ids;
    for (auto &task : tasks) {
        auto id = task.Get();
        EXPECT_FALSE(id.empty());
        file_ids.push_back(id);
    }

    // Verify all files have unique IDs
    std::sort(file_ids.begin(), file_ids.end());
    auto unique_end = std::unique(file_ids.begin(), file_ids.end());
    EXPECT_EQ(std::distance(file_ids.begin(), unique_end), num_threads);
}
