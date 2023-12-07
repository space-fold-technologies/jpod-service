#ifndef __REMOTE_SHELL_SHELL_DETAILS__
#define __REMOTE_SHELL_SHELL_DETAILS__

#include <login_cap.h>
#include <pwd.h>

namespace shell {
    struct UserDetails {
        login_cap_t *lcap;
        passwd *pwd;
    };
};

#endif // __REMOTE_SHELL_SHELL_DETAILS__