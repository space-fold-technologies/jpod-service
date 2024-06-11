#ifndef __DAEMON_CORE_OCI_DOWNLOAD_TASK_LISTENER__
#define __DAEMON_CORE_OCI_DOWNLOAD_TASK_LISTENER__

#include <string>

namespace core::oci
{
    struct update_details
    {
        std::string image_digest;
        std::string layer_digest;
        uint16_t current;
        uint16_t total;
    };
    class download_task_listener
    {
    public:
        virtual void on_download_started(const std::string &image_digest, const std::string &layer_digest) = 0;
        virtual void on_download_complete(const std::string &image_digest, const std::string &layer_digest) = 0;
        virtual void on_download_update(const update_details &details) = 0;
        virtual void on_download_failure(const std::string &image_digest, const std::string &layer_digest, const std::error_code &error) = 0;
    };
}
#endif // __DAEMON_CORE_OCI_DOWNLOAD_TASK_LISTENER__