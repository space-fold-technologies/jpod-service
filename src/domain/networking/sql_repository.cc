#include <domain/networking/sql_repository.h>
#include <domain/networking/details.h>
#include <domain/networking/payloads.h>
#include <core/sql/data_source.h>
#include <core/sql/connection_proxy.h>
#include <core/sql/statement.h>
#include <core/sql/result_set.h>
#include <core/sql/transaction.h>
#include <core/sql/error.h>
#include <range/v3/algorithm/for_each.hpp>
#include <fmt/format.h>
#include <sole.hpp>

namespace domain::networking
{
    sql_network_repository::sql_network_repository(core::sql::pool::data_source &data_source) : data_source(data_source) {}
    bool sql_network_repository::is_known(const std::string &name)
    {
        std::string sql("SELECT COUNT(*) AS is_known "
                        "FROM network_tb "
                        "WHERE name = ?");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        statement.bind(1, name);
        auto result = statement.execute_query();
        return result.has_next() ? result.fetch<bool>("is_known") : false;
    }
    std::error_code sql_network_repository::add(const network_entry &entry)
    {
        std::string sql("INSERT INTO network_tb(identifier, name, code, driver, scope, subnet) "
                        "VALUES(?, ?, ?, ?, ?, ?)");
        auto connection = data_source.connection();
        core::sql::transaction txn(connection);
        auto statement = connection->statement(sql);
        statement.bind(1, sole::uuid4().str());
        statement.bind(2, entry.name);
        statement.bind(3, entry.code);
        statement.bind(4, entry.driver);
        statement.bind(5, entry.scope);
        statement.bind(6, entry.subnet);
        if (auto result_code = statement.execute(); result_code < 0)
        {
            return core::sql::errors::make_error_code(result_code);
        }
        txn.commit();
        return {};
    }
    std::error_code sql_network_repository::update_status(const std::string &name, const std::string &status)
    {
        std::string sql("UPDATE network_tb "
                        "SET "
                        "status = ? "
                        "WHERE name = ?");
        auto connection = data_source.connection();
        core::sql::transaction txn(connection);
        auto statement = connection->statement(sql);
        statement.bind(1, status);
        statement.bind(2, name);
        if (auto result_code = statement.execute(); result_code < 0)
        {
            return core::sql::errors::make_error_code(result_code);
        }
        txn.commit();
        return {};
    }
    bool sql_network_repository::is_a_member(const std::string &network_name, const std::string &container_identifier)
    {
        std::string sql("SELECT COUNT(*) AS is_member "
                        "FROM network_member_tb AS nm "
                        "INNER JOIN network_tb AS n ON nm.network_id = n.id "
                        "INNER JOIN container_tb AS c ON nm.container_id = c.id "
                        "WHERE c.identifier = ? AND n.name = ?");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        statement.bind(1, container_identifier);
        statement.bind(2, network_name);
        auto result = statement.execute_query();
        return result.has_next() ? result.fetch<bool>("is_member") : false;
    }
    std::error_code sql_network_repository::join(const std::string &code, const std::string &members, const std::string &container_identifier)
    {
        std::string sql("INSERT INTO network_member_tb(network_id, members, container_id) "
                        "VALUES("
                        "(SELECT n.id FROM network_tb AS n WHERE n.code = ?), "
                        "?, "
                        "(SELECT c.id FROM container_tb AS c WHERE c.identifier = ?))");
        auto connection = data_source.connection();
        core::sql::transaction txn(connection);
        auto statement = connection->statement(sql);
        statement.bind(1, code);
        statement.bind(2, members);
        statement.bind(3, container_identifier);
        if (auto result_code = statement.execute(); result_code < 0)
        {
            return core::sql::errors::make_error_code(result_code);
        }
        txn.commit();
        return {};
    }
    std::optional<network_membership> sql_network_repository::members(const std::string &network_name, const std::string &container_identifier)
    {
        std::string sql("SELECT "
                        "n.code, "
                        "n.driver, "
                        "nm.members "
                        "FROM network_tb AS n "
                        "INNER JOIN network_member_tb AS nm ON nm.network_id = n.id "
                        "INNER JOIN container_tb AS c ON nm.container_id = c.id "
                        "WHERE n.name = ? AND c.identifier = ?");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        statement.bind(1, network_name);
        statement.bind(2, container_identifier);

        if (auto result = statement.execute_query(); !result.has_next())
        {
            return std::nullopt;
        }
        else
        {
            return std::make_optional(network_membership{
                result.fetch<std::string>("code"),
                result.fetch<std::string>("driver"),
                result.fetch<std::string>("members")});
        }
    }
    std::error_code sql_network_repository::leave(const std::string &network_name, const std::string &container_identifier)
    {
        std::string sql("DELETE FROM network_member_tb "
                        "WHERE "
                        "network_id = (SELECT id FROM network_tb WHERE name = ?) "
                        "AND "
                        "container_id = (SELECT id FROM container_tb WHERE identifier = ?)");
        auto connection = data_source.connection();
        core::sql::transaction txn(connection);
        auto statement = connection->statement(sql);
        statement.bind(1, network_name);
        statement.bind(1, container_identifier);
        if (auto result_code = statement.execute(); result_code < 0)
        {
            return core::sql::errors::make_error_code(result_code);
        }
        txn.commit();
        return {};
    }
    std::error_code sql_network_repository::remove(const std::string &name)
    {
        std::string sql("DELETE FROM network_tb "
                        "WHERE "
                        "name = ?");
        auto connection = data_source.connection();
        core::sql::transaction txn(connection);
        auto statement = connection->statement(sql);
        statement.bind(1, name);
        if (auto result_code = statement.execute(); result_code < 0)
        {
            return core::sql::errors::make_error_code(result_code);
        }
        txn.commit();
        return {};
    }
    std::optional<std::string> sql_network_repository::code(const std::string &name)
    {
        std::string sql("SELECT n.code FROM network_tb AS n WHERE n.name = ?");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        statement.bind(1, name);
        auto result = statement.execute_query();
        if (!result.has_next())
        {
            return std::nullopt;
        }
        return std::make_optional(result.fetch<std::string>("code"));
    }
    std::vector<network_details> sql_network_repository::list(const std::string &query)
    {
        std::string sql("SELECT "
                        "n.name, "
                        "n.driver, "
                        "n.scope, "
                        "n.subnet, "
                        "n.status "
                        "FROM network_tb AS n");
        sql += !query.empty()
                   ? "WHERE name LIKE ?"
                   : "";
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        if (!query.empty())
        {
            statement.bind(1, fmt::format("%{}%", query));
        }

        auto result = statement.execute_query();
        std::vector<network_details> entries;
        while (result.has_next())
        {
            entries.push_back(network_details{
                result.fetch<std::string>("name"),
                result.fetch<std::string>("driver"),
                result.fetch<std::string>("subnet"),
                result.fetch<std::string>("scope"),
                result.fetch<std::string>("status")});
        }
        return entries;
    }
    int sql_network_repository::total()
    {
        std::string sql("SELECT COUNT(*) AS total FROM network_tb");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        auto result = statement.execute_query();
        return result.has_next() ? result.fetch<int>("total") : 0;
    }
    sql_network_repository::~sql_network_repository() {}
}