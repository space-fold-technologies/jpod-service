#ifndef __JPOD_CORE_DATABASES_CONNECTION__
#define __JPOD_CORE_DATABASES_CONNECTION__

#include <functional>
#include <memory>
#include <string>

struct sqlite3;

namespace spdlog
{
    class logger;
};

namespace core::databases
{
    class Statement;
    template <typename T>
    using database_ptr = std::unique_ptr<T, std::function<void(T *)>>;
    class Connection
    {
        friend class Statement;

    public:
        explicit Connection(const std::string &path);
        virtual ~Connection();
        bool has_table(const std::string &name);
        bool is_ok();
        int execute(const std::string &query);
        void open();

    private:
        inline sqlite3 *handle()
        {
            return instance.get();
        }

    private:
        const std::string &path;
        database_ptr<sqlite3> instance;
        std::shared_ptr<spdlog::logger> logger;
    };

}

#endif // __JPOD_CORE_DATABASES_CONNECTION__