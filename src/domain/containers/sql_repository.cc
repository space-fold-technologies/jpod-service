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
    sql_container_repository::sql_container_repository(core::sql::pool::data_source &data_source) : data_source(data_source) {}
    std::optional<domain::images::image_details> sql_container_repository::fetch_image_details(const std::string &registry, const std::string &name, const std::string &tag) {}
    std::optional<container_details> sql_container_repository::fetch(const std::string &identifier) {}
    bool sql_container_repository::save(const container_properties &properties) {}
    sql_container_repository::~sql_container_repository() {}
}