#ifndef __JPOD_SERVICE_CORE_NETWORKS_MESSAGES_CONTROLLER__
#define __JPOD_SERVICE_CORE_NETWORKS_MESSAGES_CONTROLLER__

#include <core/networks/messages/registry.h>

namespace core::networks::messages
{
    class Controller
    {
    public:
        virtual void on_registration(HandlerRegistry &registry) = 0;
    };
}

#endif // __JPOD_SERVICE_CORE_NETWORKS_MESSAGES_CONTROLLER__