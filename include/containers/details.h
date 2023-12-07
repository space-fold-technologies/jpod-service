#ifndef __JPOD_SERVICE_CONTAINERS_DETAILS__
#define __JPOD_SERVICE_CONTAINERS_DETAILS__

#include <login_cap.h>
#include <pwd.h>

namespace containers
{
    struct UserDetails
    {
        login_cap_t *lcap;
        passwd *pwd;
    };
}
#endif // __JPOD_SERVICE_CONTAINERS_DETAILS__