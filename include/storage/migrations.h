#ifndef __JPOD_MIGRATION_HANDLER__
#define __JPOD_MIGRATION_HANDLER__

#include <string>

namespace database {
  class DataSource;
  class MigrationHandler {
  public:
    MigrationHandler(DataSource &dataSource, const std::string &migration_folder);
    ~MigrationHandler();
    void migrate();

  private:
    DataSource &dataSource;
    void apply(const std::string &content, const std::string &migration);
    const std::string &migration_folder;
  };
};     // namespace database
#endif // __JPOD_MIGRATION_HANDLER__
