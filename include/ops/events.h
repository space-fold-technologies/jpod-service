#ifndef __JPOD_SERVICE_OPERATIONS_EVENTS__
#define __JPOD_SERVICE_OPERATIONS_EVENTS__

namespace operations
{
    const std::string SHUTDOWN_SESSION = "shutdown-session";
    struct ShutdownEvent
    {
        std::string id;
    };
}

#endif // __JPOD_SERVICE_OPERATIONS_EVENTS__