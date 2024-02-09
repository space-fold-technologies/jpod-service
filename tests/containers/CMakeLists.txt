cmake_minimum_required(VERSION 3.15...3.23)

include_directories(${PROJECT_HDRS_FOLDER})

include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(range-v3 REQUIRED CONFIG)
find_package(sole REQUIRED CONFIG)
find_package(SQLite3 REQUIRED CONFIG)


add_executable(container-store-tests 
  test_container_store.cc
  ${PROJECT_SRC_FOLDER}/core/sql/connection.cc 
  ${PROJECT_SRC_FOLDER}/core/sql/data_source.cc 
  ${PROJECT_SRC_FOLDER}/core/sql/migrations.cc 
  ${PROJECT_SRC_FOLDER}/core/sql/result_set.cc 
  ${PROJECT_SRC_FOLDER}/core/sql/statement.cc 
  ${PROJECT_SRC_FOLDER}/core/sql/transaction.cc
  ${PROJECT_SRC_FOLDER}/domain/images/sql_repository.cc
  ${PROJECT_SRC_FOLDER}/domain/containers/sql_repository.cc)
target_link_libraries(container-store-tests PRIVATE catch_main range-v3::range-v3 sole::sole Msgpack::Msgpack SQLite::SQLite3 resource::rc)

# automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
# to whatever you want, or use different for different binaries
catch_discover_tests(
  container-store-tests
TEST_PREFIX
"unittests."
REPORTER
xml
OUTPUT_DIR
.
OUTPUT_PREFIX
"unittests."
OUTPUT_SUFFIX
.xml)