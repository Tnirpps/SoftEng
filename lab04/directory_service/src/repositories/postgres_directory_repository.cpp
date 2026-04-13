#include "postgres_directory_repository.hpp"

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/exceptions.hpp>
#include <userver/storages/postgres/transaction.hpp>
#include <userver/storages/postgres/io/chrono.hpp>
#include <userver/utils/datetime.hpp>
#include <userver/utils/boost_uuid4.hpp>

#include "models/directory.hpp"
#include "models/file.hpp"

namespace Repositories {

namespace {

std::optional<boost::uuids::uuid> StringToOptionalUuid(const std::optional<std::string>& str) {
    if (!str.has_value()) {
        return std::nullopt;
    }
    return userver::utils::BoostUuidFromString(*str);
}

} // namespace

PostgresDirectoryRepository::PostgresDirectoryRepository(userver::storages::postgres::ClusterPtr cluster)
    : cluster_(std::move(cluster)) {}

CreateDirectoryResult PostgresDirectoryRepository::CreateDirectory(
    const std::string &name,
    const std::optional<std::string> &parent_id,
    const std::string &owner_id) {

    try {
        auto transaction = cluster_->Begin(
            "create_directory_transaction",
            userver::storages::postgres::ClusterHostType::kMaster,
            userver::storages::postgres::Transaction::RW
        );

        // Check if directory with same name and parent already exists
        const auto check_result = transaction.Execute(
            DirectoryService::sql::kSelectDirectoryByNameAndParent,
            userver::utils::BoostUuidFromString(owner_id),
            name,
            StringToOptionalUuid(parent_id)
        );

        if (!check_result.IsEmpty()) {
            transaction.Rollback();
            return Common::Utils::unexpected(CreateDirectoryError::DirectoryAlreadyExists);
        }

        // Check parent exists if specified
        if (parent_id.has_value()) {
            const auto parent_result = transaction.Execute(
                DirectoryService::sql::kSelectDirectoryById,
                userver::utils::BoostUuidFromString(parent_id.value())
            );
            if (parent_result.IsEmpty()) {
                transaction.Rollback();
                return Common::Utils::unexpected(CreateDirectoryError::ParentNotFound);
            }
        }

        bool is_root = !parent_id.has_value();

        const auto result = transaction.Execute(
            DirectoryService::sql::kInsertDirectory,
            name,
            parent_id,
            owner_id,
            is_root
        );

        transaction.Commit();

        return result.AsSingleRow<Models::Directory>(userver::storages::postgres::kRowTag);
    } catch (const userver::storages::postgres::UniqueViolation& e) {
        LOG_WARNING() << "Unique violation in CreateDirectory: " << e.what();
        return Common::Utils::unexpected(CreateDirectoryError::DirectoryAlreadyExists);
    } catch (const std::exception& e) {
        LOG_ERROR() << "Database error in CreateDirectory: " << e.what();
        return Common::Utils::unexpected(CreateDirectoryError::ServerError);
    }
}

GetDirectoryResult PostgresDirectoryRepository::GetDirectory(const std::string &directory_id) {
    try {
        const auto result = cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kSlave,
            DirectoryService::sql::kSelectDirectoryById,
            userver::utils::BoostUuidFromString(directory_id)
        );

        if (result.IsEmpty()) {
            return std::nullopt;
        }

        return result.AsSingleRow<Models::Directory>(userver::storages::postgres::kRowTag);
    } catch (const userver::storages::postgres::Error& e) {
        LOG_WARNING() << "Directory not found in database: " << e.what();
        return std::nullopt;
    } catch (const std::exception& e) {
        LOG_ERROR() << "Database error in GetDirectory: " << e.what();
        throw;
    }
}

DirectoryListResult PostgresDirectoryRepository::ListDirectories(
    const DirectoryListParams &params,
    const std::string &owner_id) {

    try {
        int limit = params.limit.value_or(20);
        int offset = params.offset.value_or(0);

        const auto result = cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kSlave,
            DirectoryService::sql::kListDirectories,
            userver::utils::BoostUuidFromString(owner_id),
            StringToOptionalUuid(params.parent_id),
            limit,
            offset
        );

        std::vector<Models::Directory> items;
        for (const auto& row : result) {
            items.push_back(row.As<Models::Directory>(userver::storages::postgres::kRowTag));
        }

        // Get total count
        const auto count_result = cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kSlave,
            DirectoryService::sql::kCountDirectories,
            userver::utils::BoostUuidFromString(owner_id),
            StringToOptionalUuid(params.parent_id)
        );

        int total = count_result.AsSingleRow<int>(userver::storages::postgres::kFieldTag);

        return DirectoryListResult{
            .items = std::move(items),
            .total = total,
            .limit = limit,
            .offset = offset};
    } catch (const std::exception& e) {
        LOG_ERROR() << "Database error in ListDirectories: " << e.what();
        throw;
    }
}

UpdateDirectoryResult PostgresDirectoryRepository::UpdateDirectory(
    const std::string &directory_id,
    const std::string &new_name,
    const std::string &owner_id) {

    try {
        auto transaction = cluster_->Begin(
            "update_directory_transaction",
            userver::storages::postgres::ClusterHostType::kMaster,
            userver::storages::postgres::Transaction::RW
        );

        // Get current directory to check parent
        const auto current_result = transaction.Execute(
            DirectoryService::sql::kSelectDirectoryByIdAndOwner,
            userver::utils::BoostUuidFromString(directory_id),
            userver::utils::BoostUuidFromString(owner_id)
        );

        if (current_result.IsEmpty()) {
            transaction.Rollback();
            return Common::Utils::unexpected(UpdateDirectoryError::DirectoryNotFound);
        }

        auto current_dir = current_result.AsSingleRow<Models::Directory>(userver::storages::postgres::kRowTag);

        // Check for name conflict with sibling
        const auto conflict_result = transaction.Execute(
            DirectoryService::sql::kSelectDirectoryByNameAndParent,
            userver::utils::BoostUuidFromString(owner_id),
            new_name,
            current_dir.parent_uuid
        );

        if (!conflict_result.IsEmpty()) {
            auto existing_dir = conflict_result.AsSingleRow<Models::Directory>(userver::storages::postgres::kRowTag);
            if (existing_dir.uuid != current_dir.uuid) {
                transaction.Rollback();
                return Common::Utils::unexpected(UpdateDirectoryError::NameConflict);
            }
        }

        const auto result = transaction.Execute(
            DirectoryService::sql::kUpdateDirectory,
            new_name,
            userver::utils::BoostUuidFromString(directory_id),
            userver::utils::BoostUuidFromString(owner_id)
        );

        transaction.Commit();

        return result.AsSingleRow<Models::Directory>(userver::storages::postgres::kRowTag);
    } catch (const userver::storages::postgres::UniqueViolation& e) {
        LOG_WARNING() << "Unique violation in UpdateDirectory: " << e.what();
        return Common::Utils::unexpected(UpdateDirectoryError::NameConflict);
    } catch (const std::exception& e) {
        LOG_ERROR() << "Database error in UpdateDirectory: " << e.what();
        return Common::Utils::unexpected(UpdateDirectoryError::ServerError);
    }
}

DeleteDirectoryResult PostgresDirectoryRepository::DeleteDirectory(
    const std::string &directory_id,
    bool recursive,
    const std::string &owner_id) {

    try {
        auto transaction = cluster_->Begin(
            "delete_directory_transaction",
            userver::storages::postgres::ClusterHostType::kMaster,
            userver::storages::postgres::Transaction::RW
        );

        // Get current directory
        const auto current_result = transaction.Execute(
            DirectoryService::sql::kSelectDirectoryByIdAndOwner,
            userver::utils::BoostUuidFromString(directory_id),
            userver::utils::BoostUuidFromString(owner_id)
        );

        if (current_result.IsEmpty()) {
            transaction.Rollback();
            return Common::Utils::unexpected(DeleteDirectoryError::DirectoryNotFound);
        }

        if (!recursive) {
            // Check if directory has children
            const auto children_result = transaction.Execute(
                DirectoryService::sql::kCheckDirectoryHasChildren,
                userver::utils::BoostUuidFromString(directory_id)
            );

            bool has_children = children_result.AsSingleRow<bool>(userver::storages::postgres::kFieldTag);
            if (has_children) {
                transaction.Rollback();
                return Common::Utils::unexpected(DeleteDirectoryError::DirectoryNotEmpty);
            }
        }

        if (recursive) {
            // Delete with CASCADE will handle descendants
            transaction.Execute(
                DirectoryService::sql::kDeleteDirectory,
                userver::utils::BoostUuidFromString(directory_id),
                userver::utils::BoostUuidFromString(owner_id)
            );
        } else {
            transaction.Execute(
                DirectoryService::sql::kDeleteDirectory,
                userver::utils::BoostUuidFromString(directory_id),
                userver::utils::BoostUuidFromString(owner_id)
            );
        }

        transaction.Commit();

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR() << "Database error in DeleteDirectory: " << e.what();
        return Common::Utils::unexpected(DeleteDirectoryError::ServerError);
    }
}

MoveDirectoryResult PostgresDirectoryRepository::MoveDirectory(
    const std::string &directory_id,
    const std::optional<std::string> &new_parent_id,
    const std::string &owner_id) {

    try {
        auto transaction = cluster_->Begin(
            "move_directory_transaction",
            userver::storages::postgres::ClusterHostType::kMaster,
            userver::storages::postgres::Transaction::RW
        );

        // Get current directory
        const auto current_result = transaction.Execute(
            DirectoryService::sql::kSelectDirectoryByIdAndOwner,
            userver::utils::BoostUuidFromString(directory_id),
            userver::utils::BoostUuidFromString(owner_id)
        );

        if (current_result.IsEmpty()) {
            transaction.Rollback();
            return Common::Utils::unexpected(MoveDirectoryError::DirectoryNotFound);
        }

        auto current_dir = current_result.AsSingleRow<Models::Directory>(userver::storages::postgres::kRowTag);

        // Check if moving to itself
        if (new_parent_id.has_value() && new_parent_id.value() == directory_id) {
            transaction.Rollback();
            return Common::Utils::unexpected(MoveDirectoryError::WouldCreateCycle);
        }

        // Check if new parent exists
        if (new_parent_id.has_value()) {
            const auto parent_result = transaction.Execute(
                DirectoryService::sql::kSelectDirectoryByIdAndOwner,
                userver::utils::BoostUuidFromString(new_parent_id.value()),
                userver::utils::BoostUuidFromString(owner_id)
            );

            if (parent_result.IsEmpty()) {
                transaction.Rollback();
                return Common::Utils::unexpected(MoveDirectoryError::ParentNotFound);
            }

            // Check if would create cycle
            const auto cycle_result = transaction.Execute(
                DirectoryService::sql::kCheckIsDescendant,
                userver::utils::BoostUuidFromString(new_parent_id.value()),
                userver::utils::BoostUuidFromString(directory_id)
            );

            bool is_descendant = cycle_result.AsSingleRow<bool>(userver::storages::postgres::kFieldTag);
            if (is_descendant) {
                transaction.Rollback();
                return Common::Utils::unexpected(MoveDirectoryError::WouldCreateCycle);
            }
        }

        // Check for name conflict in new parent
        const auto conflict_result = transaction.Execute(
            DirectoryService::sql::kSelectDirectoryByNameAndParent,
            userver::utils::BoostUuidFromString(owner_id),
            current_dir.name,
            StringToOptionalUuid(new_parent_id)
        );

        if (!conflict_result.IsEmpty()) {
            auto existing_dir = conflict_result.AsSingleRow<Models::Directory>(userver::storages::postgres::kRowTag);
            if (existing_dir.uuid != current_dir.uuid) {
                transaction.Rollback();
                return Common::Utils::unexpected(MoveDirectoryError::NameConflict);
            }
        }

        bool is_root = !new_parent_id.has_value();

        const auto result = transaction.Execute(
            DirectoryService::sql::kMoveDirectory,
            StringToOptionalUuid(new_parent_id),
            is_root,
            userver::utils::BoostUuidFromString(directory_id),
            userver::utils::BoostUuidFromString(owner_id)
        );

        transaction.Commit();

        return result.AsSingleRow<Models::Directory>(userver::storages::postgres::kRowTag);
    } catch (const userver::storages::postgres::UniqueViolation& e) {
        LOG_WARNING() << "Unique violation in MoveDirectory: " << e.what();
        return Common::Utils::unexpected(MoveDirectoryError::NameConflict);
    } catch (const std::exception& e) {
        LOG_ERROR() << "Database error in MoveDirectory: " << e.what();
        return Common::Utils::unexpected(MoveDirectoryError::ServerError);
    }
}

FileListResult PostgresDirectoryRepository::ListFiles(
    const FileListParams &params,
    const std::string &owner_id) {

    try {
        int limit = params.limit.value_or(20);
        int offset = params.offset.value_or(0);

        const auto result = cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kSlave,
            DirectoryService::sql::kListFiles,
            userver::utils::BoostUuidFromString(params.directory_id),
            userver::utils::BoostUuidFromString(owner_id),
            limit,
            offset
        );

        std::vector<Models::File> items;
        for (const auto& row : result) {
            items.push_back(row.As<Models::File>(userver::storages::postgres::kRowTag));
        }

        // Get total count
        const auto count_result = cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kSlave,
            DirectoryService::sql::kCountFiles,
            userver::utils::BoostUuidFromString(params.directory_id),
            userver::utils::BoostUuidFromString(owner_id)
        );

        int total = count_result.AsSingleRow<int>(userver::storages::postgres::kFieldTag);

        return FileListResult{
            .items = std::move(items),
            .total = total,
            .limit = limit,
            .offset = offset};
    } catch (const std::exception& e) {
        LOG_ERROR() << "Database error in ListFiles: " << e.what();
        throw;
    }
}

void PostgresDirectoryRepository::DeleteAll() {
    try {
        // Delete all directories (files will be deleted by CASCADE)
        cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            "TRUNCATE TABLE directories CASCADE"
        );
        LOG_INFO() << "All directories deleted from database";
    } catch (const std::exception& e) {
        LOG_ERROR() << "Database error in DeleteAll: " << e.what();
        throw;
    }
}

} // namespace Repositories
