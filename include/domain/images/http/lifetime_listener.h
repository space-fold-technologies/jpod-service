#ifndef __DAEMON_DOMAIN_IMAGES_HTTP_LIFETIME_LISTENER__
#define __DAEMON_DOMAIN_IMAGES_HTTP_LIFETIME_LISTENER__
#include <domain/images/http/request.h>
#include <domain/images/http/response.h>
#include <domain/images/http/contracts.h>
#include <memory>
namespace domain::images::http
{
    class lifetime_listener
    {
    public:
        virtual void available(const std::string& id) = 0;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_HTTP_LIFETIME_LISTENER__