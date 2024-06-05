#include <domain/networking/freebsd/freebsd_network_handler.h>
#include <domain/containers/freebsd/freebsd_utils.h>
#include <domain/networking/freebsd/internal.h>
#include <domain/networking/details.h>
#include <domain/networking/errors.h>
#include <core/utilities/defer.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <net/if.h>
#include <net/vnet.h>
#include <net/if_vlan_var.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <jail.h>
#include <sys/jail.h>
#include <net/ethernet.h>
#include <net/if_bridgevar.h>
#include <unistd.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <array>
using namespace core::utilities;
namespace domain::networking::freebsd
{
    freebsd_network_handler::freebsd_network_handler() : logger(spdlog::get("jpod")) {}
    bool freebsd_network_handler::has_bridge(const std::string &name, std::error_code &error)
    {
        if (auto fd = socket(AF_LOCAL, SOCK_DGRAM, 0); fd == -1)
        {
            error = std::error_code(errno, std::system_category());
            return false;
        }
        else
        {
            // clang-format off
            defer close_fd([fd] { close(fd); });
            // clang-format on

            ifreq request{};
            strlcpy(request.ifr_name, name.c_str(), sizeof(request.ifr_name));
            return ioctl(fd, SIOCGIFCAP, &request) >= 0;
        }
    }
    std::error_code freebsd_network_handler::create_bridge(const bridge_order &order)
    {
        if (auto fd = socket(AF_INET, SOCK_DGRAM, 0); fd == -1)
        {
            return std::error_code(errno, std::system_category());
        }
        else
        {
            // clang-format off
            defer close_fd([fd] { close(fd); });
            // clang-format on
            ifreq request{};
            std::string type("bridge");
            request.ifr_addr.sa_family = AF_INET;
            strlcpy(request.ifr_name, type.c_str(), sizeof(request.ifr_name));
            logger->info("CREATING INTERFACE: {}", type);
            if (ioctl(fd, SIOCIFCREATE2, &request) != 0)
            {
                logger->error("failed to create interface");
                return std::error_code(errno, std::system_category());
            }
            std::string name(order.name);
            request.ifr_ifru.ifru_data = name.data();
            logger->info("RENAMING INTERFACE: {}", name);
            if (ioctl(fd, SIOCSIFNAME, &request) == -1)
            {
                logger->error("failed to rename an interface");
                return std::error_code(errno, std::system_category());
            }
            address_details details{order.name, order.ip, order.broadcast, order.netmask};
            logger->info("adding address details");
            return add_address(fd, details);
        }
    }
    std::error_code freebsd_network_handler::remove_bridge(const std::string &name)
    {
        if (auto fd = socket(AF_LOCAL, SOCK_DGRAM, 0); fd == -1)
        {
            return std::error_code(errno, std::system_category());
        }
        else
        {
            // clang-format off
            defer close_fd([fd] { close(fd); });
            // clang-format on
            return remove_interface(fd, name);
        }
    }
    std::error_code freebsd_network_handler::bridge_container(const network_order &order, details_callback callback)
    {
        if (auto fd = socket(AF_INET, SOCK_DGRAM, 0); fd == -1)
        {
            return std::error_code(errno, std::system_category());
        }
        else
        {
            // clang-format off
            defer close_fd([fd] { close(fd); });
            // clang-format on
            ifreq first{};
            ifreq second{};
            std::string first_name = fmt::format("{}a", order.code);
            std::string second_name = fmt::format("{}b", order.code);
            if (auto error = create_pair(fd, first, second, first_name, second_name); error)
            {
                logger->error("failed to create epair interface");
                return error;
            }
            else if (auto error = activate_interface(fd, first); error)
            {
                remove_interface(fd, second_name);
                remove_interface(fd, first_name);
                logger->error("failed to activate first epair interface");
                return error;
            }
            // the Samuel Karp way
            else if (auto error = send_bridge_order(fd, order.bridge, first_name, BRDGADD); error)
            {
                logger->error("failed to bridge the first interface");
                remove_interface(fd, second_name);
                remove_interface(fd, first_name);
                return error;
            }
            else if (auto error = move_interface_to_jail(fd, second, order.container); error)
            {
                send_bridge_order(fd, order.bridge, first_name, BRDGDEL);
                remove_interface(fd, second_name);
                remove_interface(fd, first_name);
                return error;
            }
            container_interface_order details{order.container, second_name, order.ip, order.broadcast, order.netmask, order.gateway};
            if (auto error = add_interface_to_jail(details); error)
            {
                send_bridge_order(fd, order.bridge, first_name, BRDGDEL);
                remove_interface(fd, second_name);
                remove_interface(fd, first_name);
                return error;
            }
            bridge_result result{order.bridge, order.container, fmt::format("{},{}", first_name, second_name)};
            return callback(result);
        }
    }
    std::error_code freebsd_network_handler::add_interface_to_jail(const container_interface_order &order)
    {
        int pid = fork();
        if (pid < 0)
        {
            return std::error_code(errno, std::system_category());
        }
        else if (pid == 0)
        {
            if (int jail_id = jail_getid(order.identifier.c_str()); jail_id > 0)
            {
                if (jail_attach(jail_id) == -1)
                {
                    _exit(-errno);
                }
                else if (auto fd = socket(AF_INET, SOCK_DGRAM, 0); fd == -1)
                {
                    logger->error("error: {}", strerror(errno));
                    _exit(-errno);
                }
                else
                {
                    // clang-format off
                    defer close_fd([fd] { close(fd); });
                    // clang-format on
                    ifreq request{};
                    strlcpy(request.ifr_name, order.name.c_str(), sizeof(request.ifr_name));
                    std::string assigned_name(order.name);
                    request.ifr_ifru.ifru_data = assigned_name.data();
                    if (ioctl(fd, SIOCSIFNAME, &request) == -1)
                    {
                        remove_interface(fd, order.name);
                        _exit(errno);
                    }
                    logger->info("ADDING ADDRESS TO JAIL INTERFACE: {}", order.name);
                    address_details details{order.name, order.ip, order.broadcast, order.netmask};
                    if (auto error = add_address(fd, details); error)
                    {
                        remove_interface(fd, order.name);
                        logger->error("failed to add address to interface: {}", strerror(errno));
                        _exit(errno);
                    }
                    else if (auto error = activate_interface(fd, request); error)
                    {
                        remove_interface(fd, order.name);
                        logger->error("failed to create interface: {}", strerror(errno));
                        _exit(errno); // will need a better clean up mechanism for this, perhaps channels could work for this :|
                    }
                    else if (auto error = add_route(order.gateway); error)
                    {
                        logger->error("failed to add default route: {}", error.message());
                        _exit(errno);
                    }
                    _exit(0);
                }
            }
        }
        int status;
        if (auto result = waitpid(pid, &status, 0); result < 0)
        {
            return std::error_code(result, std::system_category());
        }
        else if (WIFEXITED(status))
        {
            return std::error_code(WEXITSTATUS(status), std::system_category());
        }
        return {};
    }
    std::error_code freebsd_network_handler::leave_bridge(const std::string &name, const std::string &first, std::string &second)
    {
        if (auto fd = socket(AF_LOCAL, SOCK_DGRAM, 0); fd == -1)
        {
            return std::error_code(errno, std::system_category());
        }
        else
        {
            // clang-format off
            defer close_fd([fd] { close(fd); });
            // clang-format on
            if (auto error = send_bridge_order(fd, name, first, BRDGDEL); error)
            {
                return error;
            }
            else if (auto error = remove_interface(fd, second); error)
            {
                return error;
            }
            return remove_interface(fd, first);
        }
    }
    void freebsd_network_handler::convert_address(sockaddr_in *in, size_t size, const std::string &address)
    {
        in->sin_len = size;
        in->sin_family = AF_INET;
        inet_pton(AF_INET, address.c_str(), &in->sin_addr);
        char _ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &in->sin_addr, _ip, INET_ADDRSTRLEN);
        logger->trace("CONVERTED : {}", _ip);
    }
    std::error_code freebsd_network_handler::add_address(int socket, const address_details &details)
    {
        ifaliasreq alias_request{};
        convert_address((struct sockaddr_in *)&alias_request.ifra_addr, sizeof(struct sockaddr_in), details.ip);
        convert_address((struct sockaddr_in *)&alias_request.ifra_broadaddr, sizeof(struct sockaddr_in), details.broadcast);
        convert_address((struct sockaddr_in *)&alias_request.ifra_mask, sizeof(struct sockaddr_in), details.netmask);
        strncpy(alias_request.ifra_name, details.name.c_str(), IFNAMSIZ);
        logger->trace("SETTING ADDRESS FOR DEVICE: {}", alias_request.ifra_name);
        if (ioctl(socket, SIOCAIFADDR, &alias_request) != 0)
        {
            return std::error_code(errno, std::system_category());
        }
        logger->trace("ADDED FOR DEVICE: {}", alias_request.ifra_name);
        return {};
    }
    std::error_code freebsd_network_handler::create_pair(int socket, ifreq &first, ifreq &second, const std::string &first_name, const std::string &second_name)
    {
        std::string type("epair");
        first.ifr_addr.sa_family = AF_INET;
        strlcpy(first.ifr_name, type.c_str(), sizeof(first.ifr_name));
        if (ioctl(socket, SIOCIFCREATE2, &first) == -1)
        {
            return std::error_code(errno, std::system_category());
        }
        // fetch the first pairs name
        std::string generated_first_name = fmt::format("{}", first.ifr_name);
        // create the second pairs name
        std::string generated_second_name = fmt::format("{}b", generated_first_name.substr(0, generated_first_name.size() - 1));
        std::string assigned_first_name(first_name);
        first.ifr_ifru.ifru_data = assigned_first_name.data();
        if (ioctl(socket, SIOCSIFNAME, &first) == -1)
        {
            remove_interface(socket, generated_second_name);
            remove_interface(socket, generated_first_name);
            return std::error_code(errno, std::system_category());
        }
        // updating the name of the first interface
        strlcpy(first.ifr_name, assigned_first_name.c_str(), sizeof(first.ifr_name));

        strlcpy(second.ifr_name, generated_second_name.c_str(), sizeof(second.ifr_name));
        std::string assigned_second_name(second_name);
        second.ifr_ifru.ifru_data = assigned_second_name.data();
        if (ioctl(socket, SIOCSIFNAME, &second) == -1)
        {
            remove_interface(socket, generated_second_name);
            remove_interface(socket, assigned_first_name);
            return std::error_code(errno, std::system_category());
        }
        strlcpy(second.ifr_name, assigned_second_name.c_str(), sizeof(second.ifr_name));
        return {};
    }
    std::error_code freebsd_network_handler::move_interface_to_jail(int socket, ifreq &request, const std::string &container)
    {
        // get jail id
        if (int jail_id = jail_getid(container.c_str()); jail_id <= 0)
        {
            logger->error("failed to fetch jail id: {}", jail_errmsg);
            return std::error_code(errno, std::system_category());
        }
        else
        {
            // std::vector<jailparam> parameters;
            // defer parameter_free([&parameters](){jailparam_free(&parameters[0], parameters.size());});
            // domain::containers::freebsd::add_parameter(parameters, "name", container);
            // domain::containers::freebsd::add_parameter(parameters, "i4p.addr", std::string(request.ifr_name));
            // if (int jail_id = jailparam_set(&parameters[0], parameters.size(), JAIL_UPDATE); jail_id == -1)
            // {
            //     logger->error("failure in creating jail: JAIL ERR: {} SYS-ERR: {}", jail_errmsg, errno);
            //     return std::error_code{errno, std::system_category()};
            // }
            // jailparam_free(&parameters[0], parameters.size());
            logger->info("moving jail pair to jail");
            // request.ifr_jid = jail_id;
            ifreq jail_order{};
            memcpy(&jail_order, &request, sizeof(jail_order));
            jail_order.ifr_ifru.ifru_jid = jail_id;
            // SIOCSIFRVNET
            if (ioctl(socket, SIOCSIFVNET, &jail_order) < 0)
            {
                if (errno == EEXIST && ioctl(socket, SIOCSIFRVNET, &jail_order) < 0)
                {
                    logger->error("internal jail failure: {}", strerror(errno));
                    return std::error_code(errno, std::system_category());
                }
                else if (ioctl(socket, SIOCSIFVNET, &jail_order) < 0)
                {
                    logger->error("internal jail failure: {}", strerror(errno));
                    return std::error_code(errno, std::system_category());
                }
            }
            return {};
        }
    }
    std::error_code freebsd_network_handler::activate_interface(int socket, ifreq &request)
    {
        int flags;
        flags = IFF_UP;
        request.ifr_flags |= flags & 0xffff;
        request.ifr_flagshigh |= flags >> 16;
        if (ioctl(socket, SIOCSIFFLAGS, &request) < 0)
        {
            return std::error_code(errno, std::system_category());
        }
        return {};
    }
    std::error_code freebsd_network_handler::send_bridge_order(int socket, const std::string &name, const std::string &member, uint64_t order)
    {
        ifbreq membership_request{};
        strlcpy(membership_request.ifbr_ifsname, member.c_str(), sizeof(membership_request.ifbr_ifsname));
        ifdrv bridge_request{};
        strlcpy(bridge_request.ifd_name, name.c_str(), IF_NAMESIZE);
#if 0
	/* Get bridge priority */
	
	bridge_request.ifd_cmd = BRDGGPRI;
	bridge_request.ifd_len = sizeof(membership_request);
	bridge_request.ifd_data = &membership_request;

	if (ioctl(socket, SIOCGDRVSPEC, &bridge_request) < 0) {
		if (errno != EEXIST) {
			perror("SIOCGDRVSPEC");
			return std::error_code(errno, std::system_category());
		}
	}
#endif
        bridge_request.ifd_cmd = order;
        bridge_request.ifd_len = sizeof(ifbreq);
        bridge_request.ifd_data = &membership_request;

        if (ioctl(socket, SIOCSDRVSPEC, &bridge_request) < 0)
        {
            return std::error_code(errno, std::system_category());
        }
        return {};
    }

    std::error_code freebsd_network_handler::remove_interface(int socket, const std::string &name)
    {
        ifreq request;
        memset(&request, 0, sizeof(request));
        strlcpy(request.ifr_name, name.c_str(), sizeof(request.ifr_name));
        if (ioctl(socket, SIOCIFDESTROY, &request) < 0)
        {
            return std::error_code(errno, std::system_category());
        }
        return {};
    }

    std::error_code freebsd_network_handler::add_route(const std::string &address)
    {
        // well time to eat a book, one last feature to implement

        rt_msg_header header{};
        header.rtm_type = RTM_ADD;
        header.rtm_flags = RTF_UP | RTF_GATEWAY | RTF_STATIC | RTF_PINNED;
        header.rtm_version = RTM_VERSION;
        header.rtm_addrs = RTA_DST | RTA_GATEWAY | RTA_NETMASK;
        header.rtm_seq = 1;
        header.rtm_msglen = sizeof(rt_message<std::array<sockaddr_in, 3>>);

        sockaddr_in first{};
        sockaddr_in second{};
        sockaddr_in last{};
        first.sin_len = sizeof(sockaddr_in);
        first.sin_family = AF_INET;
        second.sin_len = sizeof(sockaddr_in);
        second.sin_family = AF_INET;

        if (auto result = inet_pton(AF_INET, address.c_str(), &second.sin_addr); result == -1)
        {
            return std::error_code(errno, std::system_category());
        }
        else if (result == 0)
        {
            return make_error_code(error_code::invalid_address_format);
        }
        last.sin_len = sizeof(sockaddr_in);
        last.sin_family = AF_INET;
        std::array<sockaddr_in, 3> payload{first, second, last};
        rt_message<std::array<sockaddr_in, 3>> msg{header, payload};
        if (auto fd = socket(PF_ROUTE, SOCK_RAW, 0); fd == -1)
        {
            return std::error_code(errno, std::system_category());
        }
        else
        {
            // clang-format off
            defer close_fd([fd] { close(fd); });
            // clang-format on
            if (write(fd, &msg, header.rtm_msglen) < 0)
            {
                logger->error("route addition failed: {}", strerror(errno));
                return std::error_code(errno, std::system_category());
            }
            return {};
        }
    }
    freebsd_network_handler::~freebsd_network_handler() {}
}