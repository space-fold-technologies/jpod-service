#ifndef __JPOD_SQLITE3_RESULT_SET__
#define __JPOD_SQLITE3_RESULT_SET__
#include <string>

namespace database {
  class Statement;
  class ResultSet {
  public:
    ResultSet(const Statement *statement);
    ~ResultSet();
    template <typename T>
    T fetch(const std::string &columnName) const;

    template <>
    inline std::string fetch(const std::string &columnName) const {
      auto columnIndex = getColumnIndex(columnName);
      return getString(columnIndex);
    }

    template <>
    inline int fetch(const std::string &columnName) const {
      auto columnIndex = getColumnIndex(columnName);
      return getInt(columnIndex);
    }

    template <>
    inline int64_t fetch(const std::string &columnName) const {
      auto columnIndex = getColumnIndex(columnName);
      return getInt64(columnIndex);
    }

    template <>
    inline double fetch(const std::string &columnName) const {
      auto columnIndex = getColumnIndex(columnName);
      return getDouble(columnIndex);
    }
    bool hasNext();

  private:
    const Statement *statement;
    int getColumnIndex(const std::string &columnName) const;
    int getInt(const int columnIndex) const;
    int64_t getInt64(const int columnIndex) const;
    double getDouble(const int columnIndex) const;
    std::string getString(const int columnIndex) const;
  };
};     // namespace database
#endif // __JPOD_SQLITE3_RESULT_SET__