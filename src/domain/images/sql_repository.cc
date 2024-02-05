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
    std::optional<registry> sql_image_repository::fetch_registry_by_uri(const std::string &path)
    {
        std::string sql("SELECT "
                        "r.uri, "
                        "r.token "
                        "FROM registry_tb AS r "
                        "WHERE r.path = ?");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        statement.bind(0, path);
        auto result = statement.execute_query();
        if (!result.has_next())
        {
            return std::nullopt;
        }
        return {registry{result.fetch<std::string>("uri"), result.fetch<std::string>("token")}};
    }
    std::optional<registry> sql_image_repository::fetch_registry_by_name(const std::string &name)
    {
        std::string sql("SELECT "
                        "r.uri, "
                        "r.token "
                        "FROM registry_tb AS r "
                        "WHERE r.name = ?");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        statement.bind(0, name);
        auto result = statement.execute_query();
        if (!result.has_next())
        {
            return std::nullopt;
        }
        return {registry{result.fetch<std::string>("uri"), result.fetch<std::string>("token")}};
    }
    bool sql_image_repository::has_image(const std::string &registry, const std::string &name, const std::string &tag)
    {
        std::string sql("SELECT COUNT(*) AS image_exists "
                        "FROM image_tb AS i"
                        "INNER JOIN registry_tb AS r ON i.registry_id = r.id "
                        "WHERE r.path = ? "
                        "AND i.name = ? "
                        "AND i.tag = ?");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        statement.bind(0, registry);
        statement.bind(1, name);
        statement.bind(2, tag);
        auto result = statement.execute_query();
        return result.has_next() ? result.fetch<bool>("image_exists") : false;
    }
    std::error_code sql_image_repository::save_image_details(const image_details &details)
    {
        std::string sql("INSERT INTO image_tb(identifier, name, tag, os, variant, version, entry_point, size, internals, registry_id) "
                        "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, (SELECT r.id FROM registry_tb AS r WHERE r.uri = ?))");

        auto connection = data_source.connection();
        core::sql::transaction txn(connection);
        image_internals internals{details.labels, details.parameters, details.env_vars, details.mount_points};
        auto statement = connection->statement(sql);
        statement.bind(0, details.identifier);
        statement.bind(1, details.name);
        statement.bind(2, details.tag);
        statement.bind(3, details.os);
        statement.bind(4, details.variant);
        statement.bind(5, details.version);
        statement.bind(6, details.entry_point);
        statement.bind(7, static_cast<int64_t>(details.size));
        statement.bind(8, pack_image_internals(internals));
        statement.bind(8, details.registry_uri);
        if (auto result_code = statement.execute(); result_code < 0)
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
                        "i.name, "
                        "i.tag, "
                        "i.os, "
                        "i.variant, "
                        "i.version, "
                        "i.entry_point, "
                        "i.size, "
                        "i.internals "
                        "FROM image_tb AS m "
                        "INNER JOIN registry_tb AS r ON i.registry_id = r.id "
                        "WHERE r.path = ? "
                        "AND i.name = ? "
                        "AND i.tag = ?");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        statement.bind(0, registry);
        statement.bind(1, name);
        statement.bind(2, tag);
        if (auto result = statement.execute_query(); !result.has_next())
        {
            return std::nullopt;
        }
        else
        {
            image_details details{};

            details.identifier = result.fetch<std::string>("identifier");
            details.name = result.fetch<std::string>("name");
            details.tag = result.fetch<std::string>("tag");
            details.os = result.fetch<std::string>("os");
            details.variant = result.fetch<std::string>("variant");
            details.version = result.fetch<std::string>("version");
            details.entry_point = result.fetch<std::string>("entry_point");
            details.size = static_cast<std::size_t>(result.fetch<int64_t>("size"));
            image_internals internals = unpack_image_internals(result.fetch<std::vector<uint8_t>>("internals"));
            details.env_vars.insert(internals.env_vars.begin(), internals.env_vars.end());
            details.labels.insert(internals.labels.begin(), internals.labels.end());
            details.parameters.insert(internals.parameters.begin(), internals.parameters.end());
            details.mount_points.assign(internals.mount_points.begin(), internals.mount_points.end());
            return details;
        }
    }
    std::vector<image_summary_entry> sql_image_repository::fetch_matching_details(const std::string &query)
    {

        std::string sql = fmt::format("SELECT "
                                      "i.identifier, "
                                      "i.name, "
                                      "i.tag, "
                                      "r.name AS repository, "
                                      "i.size, "
                                      "UNIXEPOCH(i.created_at), AS creation_date, "
                                      "FROM image_tb AS m "
                                      "INNER JOIN registry_tb AS r ON i.registry_id = r.id {}",
                                      !query.empty() ? "WHERE i.name LIKE ? OR i.identifier LIKE ? " : "");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        if (!query.empty())
        {
            statement.bind(0, query);
            statement.bind(1, query);
        }

        auto result = statement.execute_query();
        std::vector<image_summary_entry> entries;
        while (result.has_next())
        {
            image_summary_entry entry{};
            entry.identifier = result.fetch<std::string>("identifier");
            entry.name = result.fetch<std::string>("name");
            entry.tag = result.fetch<std::string>("tag");
            entry.repository = result.fetch<std::string>("repository");
            entry.size = static_cast<std::size_t>(result.fetch<int64_t>("size"));
            entry.created_at = result.fetch<time_point<system_clock, milliseconds>>("creation_date");
            entries.push_back(entry);
        }
        return entries;
    }
    std::optional<std::string> sql_image_repository::fetch_image_identifier(const std::string &registry, const std::string &name, const std::string &tag)
    {
        std::string sql("SELECT i.identifier "
                        "FROM image_tb AS i"
                        "INNER JOIN registry_tb AS r ON i.registry_id = r.id "
                        "WHERE r.path = ? "
                        "AND i.name = ? "
                        "AND i.tag = ?");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        statement.bind(0, registry);
        statement.bind(1, name);
        statement.bind(2, tag);
        auto result = statement.execute_query();
        if (!result.has_next())
        {
            return std::nullopt;
        }
        return {result.fetch<std::string>("identifier")};
    }
    std::vector<mount_point> sql_image_repository::fetch_image_mount_points(const std::string &registry, const std::string &name, const std::string &tag)
    {
        std::string sql("SELECT m.internals "
                        "FROM mount_point_tb AS m "
                        "INNER JOIN image_tb AS i ON m.image_id = i.id "
                        "INNER JOIN registry_tb AS r ON i.registry_id = r.id "
                        "WHERE r.path = ? "
                        "AND i.name = ? "
                        "AND i.tag = ?");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        statement.bind(0, registry);
        statement.bind(1, name);
        statement.bind(2, tag);
        auto result = statement.execute_query();
        std::vector<mount_point> mount_points;
        if (result.has_next())
        {
            image_internals internals = unpack_image_internals(result.fetch<std::vector<uint8_t>>("internals"));
            mount_points.reserve(internals.mount_points.size());
            mount_points.assign(internals.mount_points.begin(), internals.mount_points.end());
        }
        return mount_points;
    }
    bool sql_image_repository::has_containers(const std::string &query)
    {
        std::string sql("SELECT "
                        "COUNT(*) AS has_containers "
                        "FROM container_tb AS c "
                        "INNER JOIN image_tb AS i ON c.image_id = i.id "
                        "WHERE i.name = ? OR i.identifier = ?");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        statement.bind(0, query);
        statement.bind(1, query);
        if (auto result = statement.execute_query(); !result.has_next())
        {
            return false;
        }
        else
        {
            return result.fetch<uint32_t>("has_containers") > 0;
        }
    }
    std::error_code sql_image_repository::remove(const std::string &query)
    {
        std::string sql("DELETE "
                        "FROM image_tb WHERE name = ? OR identifier = ?");
        auto connection = data_source.connection();
        core::sql::transaction txn(connection);
        auto statement = connection->statement(sql);
        statement.bind(0, query);
        statement.bind(1, query);
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