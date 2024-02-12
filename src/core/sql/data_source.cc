#include <core/sql/data_source.h>
#include <core/sql/connection_proxy.h>
#include <spdlog/spdlog.h>
#include <sqlite3.h>

namespace core::sql::pool
{
    data_source::data_source(std::string path, std::size_t pool_size) : path(path),
                                                                        pool_size(pool_size),
                                                                        connection_mutex(),
                                                                        logger(spdlog::get("jpod"))
    {
        logger->trace("SQLITE {}", sqlite3_libversion());
        logger->trace("IS THREAD SAFE {}", sqlite3_threadsafe() ? "ON" : "OFF");
    }

    core::sql::connection_proxy data_source::connection()
    {
        std::unique_lock<std::mutex> lock(connection_mutex, std::try_to_lock);
        for (auto &[key, value] : available)
        {
            core::sql::connection_proxy proxy{key, this};
            auto node = available.extract(key);
            busy.insert(std::move(node));
            return proxy;
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
        std::unique_lock<std::mutex> lock(connection_mutex, std::try_to_lock);
        if (auto result = busy.find(instance); result != busy.end())
        {
            auto node = busy.extract(result);
            available.insert(std::move(node));
        }
    }

    data_source::~data_source() {}
}
