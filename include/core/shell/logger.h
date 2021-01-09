#ifndef __JPOD__SHELL_LOGGER__
#define __JPOD__SHELL_LOGGER__

#include <definitions.h>
#include <spdlog/spdlog.h>
#include <websocketpp/common/cpp11.hpp>
#include <websocketpp/logger/basic.hpp>
#include <websocketpp/logger/levels.hpp>

namespace shell {
  template <typename concurrency, typename names>
  class Logger : public websocketpp::log::basic<concurrency, names> {
  public:
    typedef websocketpp::log::basic<concurrency, names> base;

    Logger<concurrency, names>(websocketpp::log::channel_type_hint::value hint =
                                   websocketpp::log::channel_type_hint::access)
        : websocketpp::log::basic<concurrency, names>(hint), m_channel_type_hint(hint) {
    }

    Logger<concurrency, names>(websocketpp::log::level channels, websocketpp::log::channel_type_hint::value hint =
                                                                     websocketpp::log::channel_type_hint::access)
        : websocketpp::log::basic<concurrency, names>(channels, hint), m_channel_type_hint(hint) {
    }

    void write(websocketpp::log::level channel, std::string const &msg) {
      write(channel, msg.c_str());
    }

    void write(websocketpp::log::level channel, char const *msg) {
      scoped_lock_type lock(base::m_lock);
      if (!this->dynamic_test(channel)) {
        return;
      }

      auto logger = spdlog::get(LOGGER);

      if (m_channel_type_hint == websocketpp::log::channel_type_hint::access) {
        logger->info(msg);
      } else {
        if (channel == websocketpp::log::elevel::devel) {
          logger->debug(msg);
        } else if (channel == websocketpp::log::elevel::library) {
          logger->debug(msg);
        } else if (channel == websocketpp::log::elevel::info) {
          logger->info(msg);
        } else if (channel == websocketpp::log::elevel::warn) {
          logger->warn(msg);
        } else if (channel == websocketpp::log::elevel::rerror) {
          logger->error(msg);
        } else if (channel == websocketpp::log::elevel::fatal) {
          logger->critical(msg);
        }
      }
    }

  private:
  private:
    typedef typename base::scoped_lock_type scoped_lock_type;
    websocketpp::log::channel_type_hint::value m_channel_type_hint;
  };
};     // namespace shell
#endif // __JPOD__SHELL_LOGGER__