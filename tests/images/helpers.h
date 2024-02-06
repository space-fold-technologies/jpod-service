#ifndef __JPOD_TESTING_HELPERS__
#define __JPOD_TESTING_HELPERS__

#include <cmrc/cmrc.hpp>
#include <core/sql/data_source.h>
#include <core/sql/migrations.h>

CMRC_DECLARE(resources);

namespace core::sql
{
    // Will need to find a way to start and stop migrations
    inline auto migrate(core::sql::pool::data_source &data_source, const std::string &path) -> void
    {
        migration_handler handler(data_source, path);
        handler.migrate();
    }
}

#endif // __JPOD_TESTING_HELPERS__