#ifndef __JPOD_CORE_DATABASES_MIGRATIONS__
#define __JPOD_CORE_DATABASES_MIGRATIONS__

#include <string>
#include <memory>
#include <system_error>

namespace spdlog
{
    class logger;
};

namespace core::databases
{
    class DataSource;
    class MigrationHandler
    {
        MigrationHandler(DataSource &data_source, const std::string &folder);
        virtual ~MigrationHandler();
        void migrate();

    private:
        void apply(const std::string &content, const std::string &migration);

    private:
        DataSource &data_source;
        const std::string &folder;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif //__JPOD_CORE_DATABASES_MIGRATIONS__