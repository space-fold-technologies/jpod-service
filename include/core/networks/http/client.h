#ifndef __JPOD_SERVICE_CORE_NETWORKS_HTTP_CLIENT__
#define __JPOD_SERVICE_CORE_NETWORKS_HTTP_CLIENT__

#include <functional>
#include <core/networks/http/request.h>
#include <core/networks/http/response.h>
#include <core/networks/http/download_components.h>

namespace core::networks::http
{
    class Client
    {
    public:
        virtual void execute(const Request &req, std::function<void(const Response &)> cbr) = 0;
        virtual void download(const std::string &path, std::shared_ptr<Destination> destination, std::function<void(const Status &)> cbr) = 0;
    };
}

#endif // __JPOD_SERVICE_CORE_NETWORKS_HTTP_CLIENT__