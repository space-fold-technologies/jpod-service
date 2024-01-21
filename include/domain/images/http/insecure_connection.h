#ifndef __DAEMON_DOMAIN_IMAGES_HTTP_INSECURE_CONNECTION__
#define __DAEMON_DOMAIN_IMAGES_HTTP_INSECURE_CONNECTION__

#include <domain/images/http/connection.h>
#include <domain/images/http/connection_base.h>

namespace domain::images::http
{

    class insecure_connection : public connection, public connection_base
    {
    public:
        explicit insecure_connection(asio::io_context &context, const std::string &identifier, lifetime_listener &listener);
        virtual ~insecure_connection();
        void download(const internal::uri &uri, const std::map<std::string, std::string> &headers, std::shared_ptr<download_destination> sink, report_callback callback) override;
        void execute(const request &req, response_callback callback) override;
        void upload(
            const internal::uri &uri,
            const std::map<std::string, std::string> &headers,
            const fs::path &file_path,
            upload_callback callback) override;

    protected:
        asio::ip::tcp::socket &socket() override;

    private:
        asio::ip::tcp::socket _socket;
    };

}

#endif // __DAEMON_DOMAIN_IMAGES_HTTP_INSECURE_CONNECTION__