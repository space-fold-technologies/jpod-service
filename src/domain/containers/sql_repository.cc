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
            domain::images::image_details details{};

            details.identifier = result.fetch<std::string>("identifier");
            details.name = result.fetch<std::string>("name");
            details.tag = result.fetch<std::string>("tag");
            details.os = result.fetch<std::string>("os");
            details.variant = result.fetch<std::string>("variant");
            details.version = result.fetch<std::string>("version");
            details.entry_point = result.fetch<std::string>("entry_point");
            details.size = static_cast<std::size_t>(result.fetch<int64_t>("size"));
            domain::images::image_internals internals = domain::images::unpack_image_internals(result.fetch<std::vector<uint8_t>>("internals"));
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
        statement.bind(0, identifier);
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
        statement.bind(0, query);
        statement.bind(1, query);
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
        statement.bind(0, query);
        statement.bind(1, query);
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
        std::string sql("INSERT INTO container_tb(identifier, name, internals, image_id) "
                        "VALUES(?, ?, ?, SELECT i.id FROM image_tb AS i WHERE i.identifier = ?)");
        auto connection = data_source.connection();
        core::sql::transaction txn(connection);
        container_internals internals{properties.parameters, properties.port_map, properties.env_vars, properties.network_properties};
        auto statement = connection->statement(sql);
        statement.bind(0, properties.identifier);
        statement.bind(1, properties.name);
        statement.bind(2, pack_container_internals(internals));
        statement.bind(3, properties.image_identifier);
        if (auto result_code = statement.execute(); result_code < 0)
        {
            return core::sql::errors::make_error_code(result_code);
        }
        txn.commit();
        return {};
    }
    sql_container_repository::~sql_container_repository() {}
}