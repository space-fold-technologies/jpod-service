#include <algorithm>
#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>
#include <sqlite3.h>
#include <sstream>
#include <storage/connection_proxy.h>
#include <storage/data_source.h>
#include <storage/migrations.h>

namespace database {

  MigrationHandler::MigrationHandler(DataSource &dataSource, const std::string &migration_folder) : dataSource(dataSource),
                                                                                                    migration_folder(migration_folder) {
  }

  void MigrationHandler::migrate() {

    auto logger = spdlog::get("app");
    auto file_iterator = std::filesystem::directory_iterator(this->migration_folder);
    auto migration_files = std::find_if(begin(file_iterator), end(file_iterator), [](const std::filesystem::directory_entry &file) -> bool {
      return file.is_regular_file() && file.path().extension() == ".sql";
    });

    std::for_each(begin(migration_files), end(migration_files), [&](const std::filesystem::directory_entry &file) {
      logger->info("MIGRATION {}", file.path().filename().string());
      std::ifstream in(file.path(), std::ios::in | std::ios::binary);
      if (!in) {
        throw std::runtime_error("Failed to open configuration");
      }
      std::ostringstream content;
      content << in.rdbuf();
      in.close();

      apply(content.str(), file.path().filename().string());
    });
  }

  void MigrationHandler::apply(const std::string &content, const std::string &migration) {
    if (auto connectionProxy = dataSource.fetchConnection(); connectionProxy.isValid()) {
      if (connectionProxy->execute(content) < 0) {
        spdlog::get("app")->error("MIGRATION {} FAILED", migration);
      }
    }
  }

  MigrationHandler::~MigrationHandler() {
  }
} // namespace database
