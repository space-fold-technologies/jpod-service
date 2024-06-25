#ifndef __DAEMON_CORE_HTTP_SSL_CONFIGURATION__
#define __DAEMON_CORE_HTTP_SSL_CONFIGURATION__

#include <asio/ssl.hpp>

namespace core::http
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
        ctx.set_options(asio::ssl::context::default_workarounds);
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
            if (!configuration.verification_file.empty() || configuration.verify)
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

}

#endif // __DAEMON_CORE_HTTP_SSL_CONFIGURATION__