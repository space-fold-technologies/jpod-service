#ifndef __JPOD_SERVICE_SHELL_EVENTS__
#define __JPOD_SERVICE_SHELL_EVENTS__
#include <string>

namespace shell
{
    const std::string END_SESSION = "end-shell-session";
    struct ShellEvent
    {
        std::string id;
    };

}
#endif // __JPOD_SERVICE_SHELL_EVENTS__