#ifndef __DAEMON_BOOTSTRAP__
#define __DAEMON_BOOTSTRAP__
#include <memory>

namespace asio
{
    class io_context;
};

namespace core::commands
{
    class command_handler_registry;
};
namespace core::connections
{
    class connection_acceptor;
};

namespace core::sql::pool
{
    class data_source;
};
namespace domain::images 
{
    class image_repository;
};
namespace domain::images::http 
{
    class client;
};
using namespace core::commands;
using namespace core::connections;

class bootstrap
{
public:
    explicit bootstrap(asio::io_context &context);
    virtual ~bootstrap();
    void setup();
    void start();
    void stop();

private:
    std::shared_ptr<command_handler_registry> registry;
    std::unique_ptr<connection_acceptor> acceptor;
    std::unique_ptr<core::sql::pool::data_source> data_source;
    std::shared_ptr<domain::images::image_repository> image_repository;
    std::shared_ptr<domain::images::http::client> client;
    asio::io_context &context;
};

#endif // __DAEMON_BOOTSTRAP__