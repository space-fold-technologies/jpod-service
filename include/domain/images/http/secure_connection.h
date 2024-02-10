#ifndef __DAEMON_DOMAIN_IMAGES_HTTP_SECURE_CONNECTION__
#define __DAEMON_DOMAIN_IMAGES_HTTP_SECURE_CONNECTION__

#include <domain/images/http/connection.h>
#include <domain/images/http/connection_base.h>
#include <asio/ssl.hpp>
namespace domain::images::http
{
    struct ssl_configuration
    {
        std::string certificate_file;
        std::string private_key_file;
        std::string verification_file;
        bool verify;
    };

    inline asio::ssl::context create_context(const ssl_configuration &configuration)
    {
        asio::ssl::context ctx(asio::ssl::context::tls_client);
        if (!configuration.certificate_file.empty() && !configuration.private_key_file.empty())
        {
            ctx.use_certificate_chain_file(configuration.certificate_file);
            ctx.use_private_key_file(configuration.private_key_file, asio::ssl::context::pem);
            if (!configuration.verification_file.empty())
            {
                ctx.load_verify_file(configuration.verification_file);
            }
            else
            {
                ctx.set_default_verify_paths();
            }
            if (configuration.verify)
            {
                ctx.set_verify_mode(asio::ssl::verify_peer);
            }
            else
            {
                ctx.set_verify_mode(asio::ssl::verify_none);
            }
                }
        return ctx;
    };
    class secure_connection : public connection, public connection_base
    {
    public:
        explicit secure_connection(asio::io_context &context, asio::ssl::context &ssl_ctx, const std::string &identifier, lifetime_listener &listener);
        virtual ~secure_connection();
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
        asio::ssl::stream<asio::ip::tcp::socket> stream;
    };

}

#endif // __DAEMON_DOMAIN_IMAGES_HTTP_SECURE_CONNECTION__