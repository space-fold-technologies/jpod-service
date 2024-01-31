#include <domain/containers/freebsd/freebsd_container.h>
#include <domain/containers/freebsd/freebsd_utils.h>
#include <asio/io_context.hpp>
#include <asio/post.hpp>
#include <asio/read.hpp>
#include <asio/write.hpp>
#include <array>
#include <libutil.h>
#include <login_cap.h>
#include <paths.h>
#include <poll.h>
#include <pwd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/jail.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/uio.h>
#include <jail.h>
#include <termios.h>
#include <thread>
#include <unistd.h>
#include <filesystem>
#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/algorithm/count_if.hpp>
#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

namespace domain::containers::freebsd
{
    freebsd_container::freebsd_container(asio::io_context &context, container_details details) : container(std::move(details)),
                                                                                           context(context),
                                                                                           file_descriptor(-1),
                                                                                           process_identifier(-1),
                                                                                           buffer(WRITE_BUFFER_SIZE),
                                                                                           stream(nullptr),
                                                                                           logger(spdlog::get("jpod"))
    {
    }
    void freebsd_container::initialize()
    {
    }
    void freebsd_container::start()
    {
    }
    void freebsd_container::resize(int columns, int rows)
    {
        // we will make the call to window re-size
    }
    void freebsd_container::register_listener(std::shared_ptr<container_listener> listener)
    {
        if (listener)
        {
            listener.reset();
            this->listener = listener;
        }
    }

    std::error_code freebsd_container::mount_file_systems()
    {
        return {};
    }
    std::error_code freebsd_container::unmount_file_systems()
    {
        return {};
    }
    freebsd_container::~freebsd_container()
    {
    }
}