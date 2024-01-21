#ifndef __DAEMON_DOMAIN_IMAGES_HTTP_ASIO_CLIENT__
#define __DAEMON_DOMAIN_IMAGES_HTTP_ASIO_CLIENT__

#include <string>
#include <domain/images/http/client.h>
#include <domain/images/http/lifetime_listener.h>
#include <memory>
#include <mutex>

namespace asio
{
    class io_context;
};
namespace domain::images::http
{
    class connection;
    class asio_client : public client, public lifetime_listener
    {
    public:
        explicit asio_client(asio::io_context &context, uint32_t pool_size = 2);
        virtual ~asio_client();
        void download(const std::string &path,const std::map<std::string, std::string> &headers, std::shared_ptr<download_destination> sink, report_callback callback) override;
        void upload(
            const std::string &path,
            const std::map<std::string, std::string> &headers,
            const fs::path &file_path,
            upload_callback callback) override;
        void execute(const request &req, response_callback callback) override;
        void available(const std::string &id) override;

    private:
        asio::io_context &context;
        std::map<std::string, std::shared_ptr<connection>> available_secured_connections;
        std::map<std::string, std::shared_ptr<connection>> available_insecured_connections;
        std::map<std::string, std::shared_ptr<connection>> busy_secure_connections;
        std::map<std::string, std::shared_ptr<connection>> busy_insecure_connections;
        std::mutex access_mutex;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_HTTP_ASIO_CLIENT__