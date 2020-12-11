#ifndef __JPOD_SQL_JAIL_REPOSITORY__
#define __JPOD_SQL_JAIL_REPOSITORY__

#include <jails/jail_repository.h>

namespace database {
  class DataSource;
}

namespace prison {

  class SqlPrisonRepository : public PrisonRepository {
  public:
    SqlPrisonRepository(database::DataSource &dataSource);
    ~SqlPrisonRepository();
    void save(const Details &details) override;
    Details fetchByReference(const std::string &reference) override;
    Details fetchByNameOrReference(const std::string &value) override;
    std::vector<Details> fetchAll() override;
    void remove(const std::string &reference) override;

  private:
    database::DataSource &dataSource;
  };
}; // namespace prison

#endif // __JPOD_SQL_JAIL_REPOSITORY__
