#include <domain/containers/sql_repository.h>
#include <core/sql/data_source.h>
#include <core/sql/connection_proxy.h>
#include <core/sql/statement.h>
#include <core/sql/result_set.h>
#include <core/sql/transaction.h>
#include <core/sql/error.h>
#include <range/v3/algorithm/for_each.hpp>
#include <fmt/format.h>

namespace domain::containers
{
    sql_container_repository::sql_container_repository(core::sql::pool::data_source &data_source) : data_source(data_source)
    {
    }
    std::optional<domain::images::image_details> sql_container_repository::fetch_image_details(const std::string &registry, const std::string &name, const std::string &tag)
    {
        std::string sql("SELECT "
                        "i.identifier, "
                        "i.name, "
                        "i.tag, "
                        "i.os, "
                        "i.variant, "
                        "i.version, "
                        "i.entry_point, "
                        "i.size, "
                        "i.internals "
                        "FROM image_tb AS i "
                        "INNER JOIN registry_tb AS r ON i.registry_id = r.id "
                        "WHERE r.path = ? "
                        "AND i.name = ? "
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
            domain::images::image_details details{};

            details.identifier = result.fetch<std::string>("identifier");
            details.name = result.fetch<std::string>("name");
            details.tag = result.fetch<std::string>("tag");
            details.os = result.fetch<std::string>("os");
            details.variant = result.fetch<std::string>("variant");
            details.version = result.fetch<std::string>("version");
            details.entry_point = result.fetch<std::string>("entry_point");
            details.size = static_cast<std::size_t>(result.fetch<int64_t>("size"));
            auto internals = domain::images::unpack_image_internals(result.fetch<std::vector<uint8_t>>("internals"));
            details.env_vars.insert(internals.env_vars.begin(), internals.env_vars.end());
            details.labels.insert(internals.labels.begin(), internals.labels.end());
            details.parameters.insert(internals.parameters.begin(), internals.parameters.end());
            details.mount_points.assign(internals.mount_points.begin(), internals.mount_points.end());
            return details;
        }
    }
    std::optional<container_details> sql_container_repository::fetch(const std::string &identifier)
    {
        std::string sql("SELECT "
                        "c.identifier, "
                        "c.internals "
                        "FROM container_tb AS c "
                        "WHERE c.identifier = ?");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        statement.bind(1, identifier);
        if (auto result = statement.execute_query(); !result.has_next())
        {
            return std::nullopt;
        }
        else
        {
            container_details details{};
            details.identifier = result.fetch<std::string>("identifier");
            auto content = result.fetch<std::vector<uint8_t>>("internals");
            auto internals = unpack_container_internals(content);
            fill_container_details(details, internals);
            return details;
        }
    }

    std::optional<container_details> sql_container_repository::first_match(const std::string &query)
    {
        std::string sql("SELECT "
                        "c.identifier, "
                        "c.internals "
                        "FROM container_tb AS c "
                        "WHERE c.identifier LIKE ? "
                        "OR c.name LIKE ? LIMIT 1");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        statement.bind(1, query);
        statement.bind(2, query);
        if (auto result = statement.execute_query(); !result.has_next())
        {
            return std::nullopt;
        }
        else
        {
            container_details details{};
            details.identifier = result.fetch<std::string>("identifier");
            container_internals internals = unpack_container_internals(result.fetch<std::vector<uint8_t>>("internals"));
            fill_container_details(details, internals);
            return details;
        }
    }

    std::optional<std::string> sql_container_repository::first_identifier_match(const std::string &query)
    {
        std::string sql("SELECT "
                        "c.identifier "
                        "FROM container_tb AS c "
                        "WHERE c.identifier LIKE ? "
                        "OR c.name LIKE ? LIMIT 1");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        statement.bind(1, query);
        statement.bind(2, query);
        if (auto result = statement.execute_query(); !result.has_next())
        {
            return std::nullopt;
        }
        else
        {
            return result.fetch<std::string>("identifier");
        }
    }
    std::error_code sql_container_repository::save(const container_properties &properties)
    {
        std::string sql("INSERT INTO container_tb(identifier, name, internals, status, image_id) "
                        "VALUES(?, ?, ?, ?, (SELECT i.id FROM image_tb AS i WHERE i.identifier = ?))");
        auto connection = data_source.connection();
        core::sql::transaction txn(connection);
        auto internals = container_internals{properties.parameters, properties.port_map, properties.env_vars, properties.entry_point, properties.network_properties};
        auto statement = connection->statement(sql);
        statement.bind(1, properties.identifier);
        statement.bind(2, properties.name);
        statement.bind(3, pack_container_internals(internals));
        statement.bind(4, "shutdown");
        statement.bind(5, properties.image_identifier);
        if (auto result_code = statement.execute(); result_code < 0)
        {
            return core::sql::errors::make_error_code(result_code);
        }
        txn.commit();
        return {};
    }
    std::vector<container_summary_entry> sql_container_repository::fetch_match(const std::string &query, const std::string &status)
    {
        return query.empty() ? fetch_match_without_query(status) : fetch_match_with_query(query, status);
    }
    std::vector<container_summary_entry> sql_container_repository::fetch_match_without_query(const std::string &status)
    {
        std::string sql = fmt::format(
            "SELECT "
            "c.identifier, "
            "c.status, "
            "c.name AS container_name, "
            "i.name AS image_name, "
            "c.internals, "
            "c.created_at "
            "FROM container_tb AS c "
            "INNER JOIN image_tb AS i ON c.image_id = i.id"
            "",
            (status != "all" ? " WHERE c.status = ?" : ""));
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        if (status != "all")
        {
            statement.bind(1, status);
        }

        auto result = statement.execute_query();
        std::vector<container_summary_entry> entries;
        while (result.has_next())
        {
            container_summary_entry entry{};
            entry.identifier = result.fetch<std::string>("identifier");
            entry.name = result.fetch<std::string>("container_name");
            entry.status = result.fetch<std::string>("status");
            entry.image = result.fetch<std::string>("image_name");
            entry.created_at = result.fetch<time_point<system_clock, nanoseconds>>("created_at");
            auto internals = unpack_container_internals(result.fetch<std::vector<uint8_t>>("internals"));
            entry.port_map.insert(internals.port_map.begin(), internals.port_map.end());
            entries.push_back(entry);
        }
        return entries;
    }
    std::vector<container_summary_entry> sql_container_repository::fetch_match_with_query(const std::string query, const std::string &status)
    {
        std::string sql = fmt::format(
            "SELECT "
            "c.identifier, "
            "c.status, "
            "c.name AS container_name, "
            "i.name AS image_name, "
            "c.internals, "
            "c.created_at "
            "FROM container_tb AS c "
            "INNER JOIN image_tb AS i ON c.image_id = i.id "
            "WHERE c.name LIKE ? OR c.identifier LIKE ?{}",
            (status != "all" ? " AND c.status = ?" : ""));
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        if (!query.empty())
        {
            statement.bind(1, fmt::format("%{}%", query));
            statement.bind(2, fmt::format("%{}%", query));
            if (status != "all")
            {
                statement.bind(3, status);
            }
        }
        auto result = statement.execute_query();
        std::vector<container_summary_entry> entries;
        while (result.has_next())
        {
            container_summary_entry entry{};
            entry.identifier = result.fetch<std::string>("identifier");
            entry.name = result.fetch<std::string>("container_name");
            entry.status = result.fetch<std::string>("status");
            entry.image = result.fetch<std::string>("image_name");
            entry.created_at = result.fetch<time_point<system_clock, nanoseconds>>("created_at");
            auto internals = unpack_container_internals(result.fetch<std::vector<uint8_t>>("internals"));
            entry.port_map.insert(internals.port_map.begin(), internals.port_map.end());
            entries.push_back(entry);
        }
        return entries;
    }
    bool sql_container_repository::is_running(const std::string &query)
    {
        std::string sql("SELECT "
                        "COUNT(*) AS is_running "
                        "FROM container_tb AS c "
                        "WHERE c.status = ? "
                        "AND c.name = ? "
                        "OR c.identifier = ?");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        statement.bind(1, "active");
        statement.bind(2, query);
        statement.bind(3, query);

        if (auto result = statement.execute_query(); !result.has_next())
        {
            return false;
        }
        else
        {
            return result.fetch<int32_t>("is_running") > 0;
        }
    }
    std::error_code sql_container_repository::register_status(const std::string &identifier, const std::string &status)
    {
        std::string sql("UPDATE "
                        "container_tb "
                        "SET "
                        "status = ? "
                        "WHERE "
                        "identifier = ?");
        auto connection = data_source.connection();
        core::sql::transaction txn(connection);
        auto statement = connection->statement(sql);
        statement.bind(1, status);
        statement.bind(2, identifier);
        if (auto result_code = statement.execute(); result_code < 0)
        {
            return core::sql::errors::make_error_code(result_code);
        }
        txn.commit();
        return {};
    }
    std::error_code sql_container_repository::remove(const std::string &query)
    {

        std::string sql("DELETE "
                        "FROM container_tb "
                        "WHERE identifier = ? OR name = ?");
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
    sql_container_repository::~sql_container_repository() {}
}