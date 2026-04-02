#include "postgres_auth_repository.hpp"

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/exceptions.hpp>
#include <userver/storages/postgres/transaction.hpp>
#include <userver/storages/postgres/io/chrono.hpp>
#include <userver/utils/datetime.hpp>

#include "models/user.hpp"
#include "utils/uuid_generator.hpp"

namespace Auth::Repositories {

PostgresAuthRepository::PostgresAuthRepository(userver::storages::postgres::ClusterPtr cluster)
    : cluster_(std::move(cluster)) {}

CheckUserResult PostgresAuthRepository::CheckUser(const std::string &login, const std::string &password) {
    try {
        const auto result = cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kSlave,
            UserService::sql::kSelectUserByLogin,
            login,
            password
        );

        if (result.IsEmpty()) {
            return std::nullopt;
        }

        return result.AsSingleRow<Models::User>(userver::storages::postgres::kRowTag);
    } catch (const userver::storages::postgres::Error& e) {
        LOG_WARNING() << "User not found in database: " << e.what();
        return std::nullopt;
    } catch (const std::exception& e) {
        LOG_ERROR() << "Database error in CheckUser: " << e.what();
        throw;
    }
}

AddUserResult PostgresAuthRepository::AddUser(
    const std::string &login,
    const std::string &password,
    const std::string &first_name,
    const std::string &last_name) {

    try {
        auto transaction = cluster_->Begin(
            "add_user_transaction",
            userver::storages::postgres::ClusterHostType::kMaster,
            userver::storages::postgres::Transaction::RW
        );

        // First check if user exists
        const auto check_result = transaction.Execute(UserService::sql::kSelectUserByLoginOnly, login);
        if (!check_result.IsEmpty()) {
            transaction.Rollback();
            return AddUserError::UserAlreadyExists;
        }
        // Insert new user
        const auto result = transaction.Execute(
            UserService::sql::kInsertUser,
            login,
            password,
            first_name,
            last_name
        );
        transaction.Commit();

        return result.AsSingleRow<Models::User>(userver::storages::postgres::kRowTag);
    } catch (const userver::storages::postgres::UniqueViolation& e) {
        LOG_WARNING() << "Unique violation in AddUser: " << e.what();
        return AddUserError::UserAlreadyExists;
    } catch (const std::exception& e) {
        LOG_ERROR() << "Database error in AddUser: " << e.what();
        return AddUserError::ServerError;
    }
}

SearchUserResult PostgresAuthRepository::SearchUserByPattern(const std::string &pattern) {
    try {
        const auto result = cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kSlave,
            UserService::sql::kSearchUserByLastNamePattern,
            "%" + pattern + "%"
        );

        if (result.IsEmpty()) {
            return std::nullopt;
        }

        return result.AsSingleRow<Models::User>(userver::storages::postgres::kRowTag);
    } catch (const userver::storages::postgres::Error& e) {
        LOG_WARNING() << "User not found in database during search: " << e.what();
        return std::nullopt;
    } catch (const std::exception& e) {
        LOG_ERROR() << "Database error in SearchUserByPattern: " << e.what();
        throw;
    }
}

void PostgresAuthRepository::DeleteAllUsers() {
    try {
        cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            UserService::sql::kDeleteAllUsers
        );
        LOG_INFO() << "All users deleted from database";
    } catch (const std::exception& e) {
        LOG_ERROR() << "Database error in DeleteAllUsers: " << e.what();
        throw;
    }
}

} // namespace Auth::Repositories
