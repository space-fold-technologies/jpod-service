#include <core/sql/data_source.h>
#include <core/sql/connection_proxy.h>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <sqlite3.h>

namespace core::sql::pool
{
    data_source::data_source(std::string path, std::size_t pool_size) : path(path),
                                                                        pool_size(pool_size),
                                                                        logger(spdlog::get("jpod"))
    {
        logger->info("SQLITE {}", sqlite3_libversion());
        logger->info("IS THREAD SAFE {}", sqlite3_threadsafe() ? "ON" : "OFF");
    }

    core::sql::connection_proxy data_source::connection()
    {
        if (connection_mutex.try_lock())
        {
            for (auto &[key, value] : available)
            {
                core::sql::connection_proxy proxy{key, this};
                auto node = available.extract(key);
                busy.insert(std::move(node));
                connection_mutex.unlock();
                return proxy;
            }
        }
        return core::sql::connection_proxy{nullptr, nullptr};
    }

    void data_source::initialize()
    {
        while (pool_size > available.size())
        {
            auto connection = std::unique_ptr<in::connection>(new in::connection(path));
            connection->open();
            auto key = connection.get();
            available.try_emplace(key, std::move(connection));
        }
    }

    void data_source::back_to_pool(in::connection *instance)
    {
        if (connection_mutex.try_lock())
        {
            if (auto result = busy.find(instance); result != busy.end())
            {
                auto node = busy.extract(result);
                available.insert(std::move(node));
            }
            connection_mutex.unlock();
        }
    }

    data_source::~data_source() {}
}
