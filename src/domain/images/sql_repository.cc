#include <domain/images/sql_repository.h>
#include <core/sql/data_source.h>
#include <core/sql/connection_proxy.h>
#include <core/sql/statement.h>
#include <core/sql/result_set.h>
#include <core/sql/transaction.h>
#include <core/sql/error.h>
#include <range/v3/algorithm/for_each.hpp>
#include <fmt/format.h>

namespace domain::images
{
    sql_image_repository::sql_image_repository(core::sql::pool::data_source &data_source) : data_source(data_source)
    {
    }
    std::error_code sql_image_repository::add_registry(const registry_details &details)
    {
        std::string sql("INSERT INTO registry_tb(name, uri, path) "
                        "VALUES(?, ?, ?)");

        auto connection = data_source.connection();
        core::sql::transaction txn(connection);
        auto statement = connection->statement(sql);
        statement.bind(1, details.name);
        statement.bind(2, details.uri);
        statement.bind(3, details.path);
        if (auto result_code = statement.execute(); result_code < 0)
        {
            return core::sql::errors::make_error_code(result_code);
        }
        txn.commit();
        return {};
    }
    std::error_code sql_image_repository::update_token(const authorization_update &update)
    {
        std::string sql("UPDATE registry_tb "
                        "SET "
                        "token = ? "
                        "WHERE "
                        "path = ?");

        auto connection = data_source.connection();
        core::sql::transaction txn(connection);
        auto statement = connection->statement(sql);
        statement.bind(1, update.token);
        statement.bind(2, update.path);
        if (auto result_code = statement.execute(); result_code < 0)
        {
            return core::sql::errors::make_error_code(result_code);
        }
        txn.commit();
        return {};
    }
    std::optional<registry_access_details> sql_image_repository::fetch_registry_by_path(const std::string &path)
    {
        std::string sql("SELECT "
                        "r.uri, "
                        "r.authorization_type, "
                        "r.authorization_url "
                        "FROM registry_tb AS r "
                        "WHERE r.path = ?");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        statement.bind(1, path);
        auto result = statement.execute_query();
        if (!result.has_next())
        {
            return std::nullopt;
        }
        registry_access_details details{};
        details.uri = result.fetch<std::string>("uri");
        details.authorization_type = result.fetch<std::string>("authorization_type");
        details.authorization_url = result.fetch<std::string>("authorization_url");
        return std::make_optional(details);
    }
    std::optional<registry_access_details> sql_image_repository::fetch_registry_by_name(const std::string &name)
    {
        std::string sql("SELECT "
                        "r.uri, "
                        "r.authorization_type, "
                        "r.authorization_url "
                        "FROM registry_tb AS r "
                        "WHERE r.name = ?");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        statement.bind(1, name);
        auto result = statement.execute_query();
        if (!result.has_next())
        {
            return std::nullopt;
        }
        registry_access_details details{};
        details.uri = result.fetch<std::string>("uri");
        details.authorization_type = result.fetch<std::string>("authorization_type");
        details.authorization_url = result.fetch<std::string>("authorization_url");
        return std::make_optional(details);
    }
    bool sql_image_repository::has_image(const std::string &registry, const std::string &name, const std::string &tag)
    {
        std::string sql("SELECT COUNT(*) AS image_exists "
                        "FROM image_tb AS i "
                        "INNER JOIN registry_tb AS r ON i.registry_id = r.id "
                        "WHERE r.path = ? "
                        "AND i.repository = ? "
                        "AND i.tag = ?");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        statement.bind(1, registry);
        statement.bind(2, name);
        statement.bind(3, tag);
        auto result = statement.execute_query();
        return result.has_next() ? result.fetch<bool>("image_exists") : false;
    }
    std::error_code sql_image_repository::save_image_details(const image_details &details)
    {
        std::string sql("INSERT INTO image_tb(identifier, repository, tag, tag_reference, os, variant, version, size, registry_id) "
                        "VALUES(?, ?, ?, ?, ?, ?, ?, ?, (SELECT r.id FROM registry_tb AS r WHERE r.uri = ?))");

        auto connection = data_source.connection();
        core::sql::transaction txn(connection);
        auto statement = connection->statement(sql);
        statement.bind(1, details.identifier);
        statement.bind(2, details.repository);
        statement.bind(3, details.tag);
        statement.bind(4, details.tag_reference);
        statement.bind(5, details.os);
        statement.bind(6, details.variant);
        statement.bind(7, details.version);
        statement.bind(8, static_cast<int64_t>(details.size));
        statement.bind(9, details.registry);
        if (auto result_code = statement.execute(); result_code != SQLITE_OK)
        {
            return core::sql::errors::make_error_code(result_code);
        }
        txn.commit();
        return {};
    }
    std::optional<image_details> sql_image_repository::fetch_image_details(const std::string &registry, const std::string &name, const std::string &tag)
    {
        std::string sql("SELECT "
                        "i.identifier, "
                        "i.repository, "
                        "i.tag, "
                        "i.os, "
                        "i.variant, "
                        "i.version, "
                        "i.size "
                        "FROM image_tb AS i "
                        "INNER JOIN registry_tb AS r ON i.registry_id = r.id "
                        "WHERE r.path = ? "
                        "AND i.repository = ? "
                        "AND i.tag = ?");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        statement.bind(1, registry);
        statement.bind(2, name);
        statement.bind(3, tag);
        if (auto result = statement.execute_query(); !result.has_next())
        {
            return std::nullopt;
        }
        else
        {
            image_details details{};

            details.identifier = result.fetch<std::string>("identifier");
            details.repository = result.fetch<std::string>("repository");
            details.tag = result.fetch<std::string>("tag");
            details.os = result.fetch<std::string>("os");
            details.variant = result.fetch<std::string>("variant");
            details.version = result.fetch<std::string>("version");
            details.size = static_cast<std::size_t>(result.fetch<int64_t>("size"));
            return details;
        }
    }
    std::vector<image_summary_entry> sql_image_repository::fetch_matching_details(const std::string &query)
    {

        std::string sql = fmt::format("SELECT "
                                      "i.identifier, "
                                      "i.repository, "
                                      "i.tag, "
                                      "r.name AS registry, "
                                      "i.size, "
                                      "UNIXEPOCH(i.created_at) AS creation_date "
                                      "FROM image_tb AS i "
                                      "INNER JOIN registry_tb AS r ON i.registry_id = r.id{}",
                                      !query.empty() ? " WHERE i.name LIKE ? OR i.identifier LIKE ?" : "");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        if (!query.empty())
        {
            statement.bind(1, fmt::format("%{}%", query));
            statement.bind(2, fmt::format("%{}%", query));
        }

        auto result = statement.execute_query();
        std::vector<image_summary_entry> entries;
        while (result.has_next())
        {
            image_summary_entry entry{};
            entry.identifier = result.fetch<std::string>("identifier");
            entry.repository = result.fetch<std::string>("repository");
            entry.tag = result.fetch<std::string>("tag");
            entry.registry = result.fetch<std::string>("registry");
            entry.size = static_cast<std::size_t>(result.fetch<int64_t>("size"));
            entry.created_at = result.fetch<time_point<system_clock, nanoseconds>>("creation_date");
            entries.push_back(entry);
        }
        return entries;
    }
    std::optional<std::string> sql_image_repository::fetch_image_identifier(const std::string &registry, const std::string &name, const std::string &tag)
    {
        std::string sql("SELECT i.identifier "
                        "FROM image_tb AS i "
                        "INNER JOIN registry_tb AS r ON i.registry_id = r.id "
                        "WHERE r.path = ? "
                        "AND i.repository = ? "
                        "AND i.tag = ?");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        statement.bind(1, registry);
        statement.bind(2, name);
        statement.bind(3, tag);
        auto result = statement.execute_query();
        if (!result.has_next())
        {
            return std::nullopt;
        }
        return {result.fetch<std::string>("identifier")};
    }
    bool sql_image_repository::has_containers(const std::string &query)
    {
        std::string sql("SELECT "
                        "COUNT(*) AS has_containers "
                        "FROM container_tb AS c "
                        "INNER JOIN image_tb AS i ON c.image_id = i.id "
                        "WHERE i.repository = ? OR i.identifier = ?");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        statement.bind(1, query);
        statement.bind(2, query);
        if (auto result = statement.execute_query(); !result.has_next())
        {
            return false;
        }
        else
        {
            return result.fetch<bool>("has_containers");
        }
    }
    std::error_code sql_image_repository::remove(const std::string &query)
    {
        std::string sql("DELETE "
                        "FROM image_tb WHERE repository = ? OR identifier = ?");
        auto connection = data_source.connection();
        core::sql::transaction txn(connection);
        auto statement = connection->statement(sql);
        statement.bind(1, query);
        statement.bind(2, query);
        if (auto result_code = statement.execute(); result_code < 0)
        {
            return core::sql::errors::make_error_code(result_code);
        }
        txn.commit();
        return {};
    }
    sql_image_repository::~sql_image_repository()
    {
    }
}