#ifndef __DAEMON_DOMAIN_CONTAINERS_VIRTUAL_TERMINAL_DETAILS__
#define __DAEMON_DOMAIN_CONTAINERS_VIRTUAL_TERMINAL_DETAILS__

#include <cstdint>
#include <vector>
#include <string>
#include <map>

namespace domain::containers
{
    struct terminal_properties
    {
        std::string identifier;
        std::vector<std::string> commands;
        bool interactive;
        std::string user;
        uint16_t columns;
        uint16_t rows;
        std::map<std::string, std::string> env_vars;
    };
}

#endif // __DAEMON_DOMAIN_CONTAINERS_VIRTUAL_TERMINAL_DETAILS__