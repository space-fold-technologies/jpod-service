#ifndef __DAEMON_CORE_OCI_UPLOAD_TASK_LISTENER__
#define __DAEMON_CORE_OCI_UPLOAD_TASK_LISTENER__

#include <system_error>
#include <string_view>
#include <string>

namespace core::oci
{
    class upload_task_listener
    {
    public:
        virtual void on_upload_started(const std::string& image_identifier, const std::string &digest) = 0;
        virtual void on_upload_complete(const std::string& image_identifier, const std::string &digest, const std::string& location) = 0;
        virtual void on_upload_update(const std::string& image_identifier, std::string_view digest, uint16_t current, uint16_t total) = 0;
        virtual void on_upload_failure(const std::string& image_identifier, const std::string &digest, const std::error_code &error) = 0;
    };
}
#endif // __DAEMON_CORE_OCI_UPLOAD_TASK_LISTENER__
