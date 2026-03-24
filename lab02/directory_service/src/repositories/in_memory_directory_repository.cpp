#include "in_memory_directory_repository.hpp"

#include <userver/logging/log.hpp>
#include <userver/utils/boost_uuid4.hpp>
#include <userver/utils/datetime.hpp>
#include <userver/utils/uuid4.hpp>

namespace Repositories {

namespace {

std::string GenerateUuid() {
    return userver::utils::ToString(userver::utils::BoostUuidFromString(userver::utils::generators::GenerateUuid()));;
}

bool IsDescendant(const std::map<std::string, Models::Directory> &directories,
                  const std::string &potential_descendant_id,
                  const std::string &potential_ancestor_id) {
    std::string current_id = potential_descendant_id;
    while (true) {
        auto it = directories.find(current_id);
        if (it == directories.end()) {
            return false;
        }
        if (!it->second.parent_id.has_value()) {
            return false;
        }
        if (it->second.parent_id.value() == potential_ancestor_id) {
            return true;
        }
        current_id = it->second.parent_id.value();
    }
}

} // namespace

CreateDirectoryResult InMemoryDirectoryRepository::CreateDirectory(
    const std::string &name,
    const std::optional<std::string> &parent_id,
    const std::string &owner_id) {

    auto data_ptr = directories_.Lock();

    // Check if directory with same name and parent already exists
    for (const auto &[id, dir] : *data_ptr) {
        if (dir.owner_id == owner_id &&
            dir.name == name &&
            dir.parent_id == parent_id) {
            return std::unexpected(CreateDirectoryError::DirectoryAlreadyExists);
        }
    }

    // Check parent exists if specified
    if (parent_id.has_value()) {
        if (data_ptr->find(parent_id.value()) == data_ptr->end()) {
            return std::unexpected(CreateDirectoryError::ParentNotFound);
        }
    }

    std::string new_id = GenerateUuid();
    auto now = userver::utils::datetime::Now();

    Models::Directory new_dir{
        .id = new_id,
        .name = name,
        .parent_id = parent_id,
        .owner_id = owner_id,
        .created_at = userver::utils::datetime::TimePointTz{now},
        .updated_at = userver::utils::datetime::TimePointTz{now},
        .is_root = !parent_id.has_value()};

    data_ptr->emplace(new_id, new_dir);

    return new_dir;
}

GetDirectoryResult InMemoryDirectoryRepository::GetDirectory(const std::string &directory_id) {
    auto data_ptr = directories_.Lock();
    auto it = data_ptr->find(directory_id);
    if (it == data_ptr->end()) {
        return std::nullopt;
    }
    return it->second;
}

DirectoryListResult InMemoryDirectoryRepository::ListDirectories(
    const DirectoryListParams &params,
    const std::string &owner_id) {

    auto data_ptr = directories_.Lock();

    std::vector<Models::Directory> filtered;
    for (const auto &[id, dir] : *data_ptr) {
        if (dir.owner_id != owner_id) {
            continue;
        }
        if (params.parent_id.has_value()) {
            if (!dir.parent_id.has_value() || dir.parent_id.value() != params.parent_id.value()) {
                continue;
            }
        } else {
            if (dir.parent_id.has_value()) {
                continue;
            }
        }
        filtered.push_back(dir);
    }

    int total = static_cast<int>(filtered.size());

    int limit = params.limit.value_or(20);
    int offset = params.offset.value_or(0);

    if (offset >= static_cast<int>(filtered.size())) {
        return DirectoryListResult{
            .items = {},
            .total = total,
            .limit = limit,
            .offset = offset};
    }

    auto begin = filtered.begin() + offset;
    auto end = (offset + limit >= static_cast<int>(filtered.size()))
                   ? filtered.end()
                   : filtered.begin() + offset + limit;

    std::vector<Models::Directory> paginated(begin, end);

    return DirectoryListResult{
        .items = std::move(paginated),
        .total = total,
        .limit = limit,
        .offset = offset};
}

UpdateDirectoryResult InMemoryDirectoryRepository::UpdateDirectory(
    const std::string &directory_id,
    const std::string &new_name,
    const std::string &owner_id) {

    auto data_ptr = directories_.Lock();
    auto it = data_ptr->find(directory_id);
    if (it == data_ptr->end()) {
        return std::unexpected(UpdateDirectoryError::DirectoryNotFound);
    }

    if (it->second.owner_id != owner_id) {
        return std::unexpected(UpdateDirectoryError::DirectoryNotFound);
    }

    // Check for name conflict with sibling
    for (const auto &[id, dir] : *data_ptr) {
        if (id != directory_id &&
            dir.owner_id == owner_id &&
            dir.name == new_name &&
            dir.parent_id == it->second.parent_id) {
            return std::unexpected(UpdateDirectoryError::NameConflict);
        }
    }

    it->second.name = new_name;
    it->second.updated_at = userver::utils::datetime::TimePointTz{userver::utils::datetime::Now()};

    return it->second;
}

DeleteDirectoryResult InMemoryDirectoryRepository::DeleteDirectory(
    const std::string &directory_id,
    bool recursive,
    const std::string &owner_id) {

    auto data_ptr = directories_.Lock();
    auto it = data_ptr->find(directory_id);
    if (it == data_ptr->end()) {
        return std::unexpected(DeleteDirectoryError::DirectoryNotFound);
    }

    if (it->second.owner_id != owner_id) {
        return std::unexpected(DeleteDirectoryError::DirectoryNotFound);
    }

    // Check if directory has children
    bool has_children = false;
    for (const auto &[id, dir] : *data_ptr) {
        if (dir.parent_id.has_value() && dir.parent_id.value() == directory_id) {
            has_children = true;
            break;
        }
    }

    if (has_children && !recursive) {
        return std::unexpected(DeleteDirectoryError::DirectoryNotEmpty);
    }

    if (recursive) {
        // Collect all descendants
        std::vector<std::string> to_delete;
        to_delete.push_back(directory_id);

        size_t processed = 0;
        while (processed < to_delete.size()) {
            std::string current_id = to_delete[processed++];
            for (const auto &[id, dir] : *data_ptr) {
                if (dir.parent_id.has_value() && dir.parent_id.value() == current_id) {
                    to_delete.push_back(id);
                }
            }
        }

        // Delete all collected
        for (const auto &id : to_delete) {
            data_ptr->erase(id);
        }
    } else {
        data_ptr->erase(it);
    }

    return true;
}

MoveDirectoryResult InMemoryDirectoryRepository::MoveDirectory(
    const std::string &directory_id,
    const std::optional<std::string> &new_parent_id,
    const std::string &owner_id) {

    auto data_ptr = directories_.Lock();
    auto it = data_ptr->find(directory_id);
    if (it == data_ptr->end()) {
        return std::unexpected(MoveDirectoryError::DirectoryNotFound);
    }

    if (it->second.owner_id != owner_id) {
        return std::unexpected(MoveDirectoryError::DirectoryNotFound);
    }

    // Check if moving to itself
    if (new_parent_id.has_value() && new_parent_id.value() == directory_id) {
        return std::unexpected(MoveDirectoryError::WouldCreateCycle);
    }

    // Check if new parent exists
    if (new_parent_id.has_value()) {
        auto parent_it = data_ptr->find(new_parent_id.value());
        if (parent_it == data_ptr->end()) {
            return std::unexpected(MoveDirectoryError::ParentNotFound);
        }
        if (parent_it->second.owner_id != owner_id) {
            return std::unexpected(MoveDirectoryError::ParentNotFound);
        }

        // Check if would create cycle
        if (IsDescendant(*data_ptr, new_parent_id.value(), directory_id)) {
            return std::unexpected(MoveDirectoryError::WouldCreateCycle);
        }

        // Check for name conflict in new parent
        for (const auto &[id, dir] : *data_ptr) {
            if (id != directory_id &&
                dir.owner_id == owner_id &&
                dir.name == it->second.name &&
                dir.parent_id == new_parent_id) {
                return std::unexpected(MoveDirectoryError::NameConflict);
            }
        }
    }

    it->second.parent_id = new_parent_id;
    it->second.is_root = !new_parent_id.has_value();
    it->second.updated_at = userver::utils::datetime::TimePointTz{userver::utils::datetime::Now()};

    return it->second;
}

FileListResult InMemoryDirectoryRepository::ListFiles(
    const FileListParams &params,
    const std::string &owner_id) {

    auto data_ptr = files_.Lock();

    std::vector<Models::File> filtered;
    for (const auto &[id, file] : *data_ptr) {
        if (file.owner_id != owner_id) {
            continue;
        }
        if (file.directory_id != params.directory_id) {
            continue;
        }
        filtered.push_back(file);
    }

    int total = static_cast<int>(filtered.size());

    int limit = params.limit.value_or(20);
    int offset = params.offset.value_or(0);

    if (offset >= static_cast<int>(filtered.size())) {
        return FileListResult{
            .items = {},
            .total = total,
            .limit = limit,
            .offset = offset};
    }

    auto begin = filtered.begin() + offset;
    auto end = (offset + limit >= static_cast<int>(filtered.size()))
                   ? filtered.end()
                   : filtered.begin() + offset + limit;

    std::vector<Models::File> paginated(begin, end);

    return FileListResult{
        .items = std::move(paginated),
        .total = total,
        .limit = limit,
        .offset = offset};
}

void InMemoryDirectoryRepository::DeleteAll() {
    auto dir_ptr = directories_.Lock();
    auto file_ptr = files_.Lock();
    dir_ptr->clear();
    file_ptr->clear();
}

} // namespace Repositories
