#include <domain/networking/network_service.h>
#include <domain/networking/details.h>
#include <domain/networking/address_provider.h>
#include <domain/networking/network_handler.h>
#include <domain/networking/repository.h>
#include <domain/networking/errors.h>
#include <domain/networking/payloads.h>
#include <range/v3/view/split.hpp>
// #include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/range/conversion.hpp>
#include <fmt/format.h>

using namespace ranges;

namespace domain::networking
{
    network_service::network_service(std::shared_ptr<network_repository> repository, network_handler_provider provider) : repository(std::move(repository)),
                                                                                                                          provider(std::move(provider)) {}

    std::error_code network_service::add(network_entry &entry)
    {
        if (networks.find(entry.name) != networks.end())
        {
            return make_error_code(error_code::exists);
        }
        std::error_code error;
        auto handler = provider();
        if (auto result = handler->has_bridge(entry.code, error); result)
        {
            if (error = handler->remove_bridge(entry.code); error)
            {
                return error;
            }
        }
        // now go ahead and add the needed network provider
        networks.emplace(entry.name, std::move(std::make_shared<address_provider>(entry.subnet, "")));
        auto network = networks.at(entry.name);
        if (network->initialize(error); error)
        {
            return error;
        }
        if (auto subnet_address = network->fetch_subnet(); !subnet_address.has_value())
        {
            fmt::println("failed to get the subnet for this subnet");
            return make_error_code(error_code::bridge_creation_failed);
        }
        else
        {
            bridge_order order{entry.code, subnet_address->value, subnet_address->broadcast, subnet_address->type, subnet_address->netmask};
            if (error = handler->create_bridge(order); error)
            {
                return error;
            }
            else if (!repository->is_known(entry.name))
            {
                if (error = repository->add(entry); error)
                {
                    return error;
                }
            }

            return repository->update_status(entry.name, "UP");
            // we have created a bridge, now make a record of it
        }

        // first create the bridge and add the subnet details to the bridge
        return {};
    }
    std::error_code network_service::remove(const std::string &name)
    {
        if (networks.find(name) == networks.end())
        {
            return make_error_code(error_code::unknown_bridge);
        }
        std::error_code error{};
        auto handler = provider();
        if (auto code = repository->code(name); !code.has_value())
        {
            return make_error_code(error_code::unknown_bridge);
        }
        else if (handler->has_bridge(code.value(), error) && !error)
        {
            if (error = handler->remove_bridge(code.value()); error)
            {
                return error;
            }
        }
        return repository->remove(name);
    }
    std::error_code network_service::join(const std::string &name, const std::string &container_identifier)
    {
        if (auto pos = networks.find(name); pos == networks.end())
        {
            return make_error_code(error_code::unknown_bridge);
        }
        else
        {
            std::error_code error{};
            auto handler = provider();
            auto network = pos->second;
            if (auto address = network->fetch_next_available(container_identifier, ip_address_type::v4, error); error)
            {
                return error;
            }
            else if (auto code = repository->code(name); !code.has_value())
            {
                return make_error_code(error_code::unknown_bridge);
            }
            else
            {
                auto gateway_address = network->fetch_subnet();
                network_order order{
                    container_identifier,
                    code.value(),
                    fmt::format("jp{}", network->total_taken(address->type) - 1),
                    address->value,
                    address->type,
                    address->broadcast,
                    address->netmask,
                    gateway_address->value};
                // clang-format off
                auto bridging_results_handler = [this](const bridge_result& result) -> std::error_code
                {
                    return repository->join(result.code, result.members, result.container);
                };
                // clang-format on
                return handler->bridge_container(order, bridging_results_handler);
            }
        }
    }
    std::error_code network_service::leave(const std::string &name, const std::string &container_identifier)
    {
        if (auto pos = networks.find(name); pos == networks.end())
        {
            return make_error_code(error_code::unknown_bridge);
        }
        else if (auto result = repository->members(name, container_identifier); !result.has_value())
        {
            return make_error_code(error_code::unknown_bridge);
        }
        else
        {
            auto handler = provider();
            if (result->members.find(","); std::string::npos)
            {
                // this is a bridge network with epair{}a|b
                auto parts = result->members | views::split(',') | to<std::vector<std::string>>();
                if (auto error = handler->leave_bridge(result->code, parts.at(0), parts.at(1)); error)
                {
                    return error;
                }
            }

            pos->second->remove(container_identifier);
            return repository->leave(name, container_identifier);
        }
    }
    std::vector<network_details> network_service::list(const std::string &query)
    {
        return repository->list(query);
    }
    uint32_t network_service::total_networks()
    {
        return repository->total();
    }
    network_service::~network_service() {}
}