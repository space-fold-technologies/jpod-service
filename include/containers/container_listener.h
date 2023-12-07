#ifndef __JPOD_SERVICE_CONTAINERS_CONTAINER_LISTENER__
#define __JPOD_SERVICE_CONTAINERS_CONTAINER_LISTENER__

namespace containers
{
    class ContainerListener
    {
    public:
        virtual void on_container_initialized(const std::string &id) = 0;
        virtual void on_container_data_received(const std::string &id, const std::vector<uint8_t> &content) = 0;
        virtual void on_container_error(const std::string &id, const std::error_code &err) = 0;
        virtual void on_container_shutdown(const std::string &id) = 0;
    };
}

#endif // __JPOD_SERVICE_CONTAINERS_CONTAINER_LISTENER__