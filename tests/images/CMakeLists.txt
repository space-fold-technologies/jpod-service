cmake_minimum_required(VERSION 3.15...3.23)

include_directories(${PROJECT_HDRS_FOLDER})

include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(fakeit REQUIRED CONFIG)
find_package(range-v3 REQUIRED CONFIG)
find_package(sole REQUIRED CONFIG)
find_package(asio REQUIRED CONFIG)
find_package(libzip REQUIRED CONFIG)
find_package(zstd REQUIRED CONFIG)
find_package(yaml-cpp REQUIRED CONFIG)
find_package(SQLite3 REQUIRED CONFIG)

add_executable(copy-instruction-tests 
               test_copy_instruction.cc
               ${PROJECT_SRC_FOLDER}/domain/images/instructions/copy_instruction.cc)
target_link_libraries(copy-instruction-tests PRIVATE catch_main fakeit::fakeit range-v3::range-v3 sole::sole asio::asio Msgpack::Msgpack yaml-cpp::yaml-cpp)

# automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
# to whatever you want, or use different for different binaries
catch_discover_tests(
  copy-instruction-tests
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

add_executable(download-instruction-tests 
  test_download_instruction.cc
  ${PROJECT_SRC_FOLDER}/domain/images/instructions/download_instruction.cc
  ${PROJECT_SRC_FOLDER}/domain/images/http/request.cc
  ${PROJECT_SRC_FOLDER}/domain/images/http/request_builder.cc)
target_link_libraries(download-instruction-tests PRIVATE catch_main fakeit::fakeit range-v3::range-v3 sole::sole asio::asio Msgpack::Msgpack yaml-cpp::yaml-cpp)

# automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
# to whatever you want, or use different for different binaries
catch_discover_tests(
download-instruction-tests
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

add_executable(work-dir-instruction-tests 
  test_work_dir_instruction.cc
  ${PROJECT_SRC_FOLDER}/domain/images/instructions/work_dir_instruction.cc)
target_link_libraries(work-dir-instruction-tests PRIVATE catch_main fakeit::fakeit range-v3::range-v3 sole::sole asio::asio Msgpack::Msgpack yaml-cpp::yaml-cpp)

# automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
# to whatever you want, or use different for different binaries
catch_discover_tests(
  work-dir-instruction-tests
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


add_executable(compression-instruction-tests 
  test_compression_instruction.cc
  ${PROJECT_SRC_FOLDER}/domain/images/instructions/compression_instruction.cc)
target_link_libraries(compression-instruction-tests PRIVATE catch_main fakeit::fakeit range-v3::range-v3 sole::sole libzip::zip zstd::libzstd_static asio::asio Msgpack::Msgpack yaml-cpp::yaml-cpp)

# automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
# to whatever you want, or use different for different binaries
catch_discover_tests(
  compression-instruction-tests
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


add_executable(mount-instruction-tests 
  test_mount_instruction.cc
  ${PROJECT_SRC_FOLDER}/domain/images/instructions/mount_instruction.cc)
target_link_libraries(mount-instruction-tests PRIVATE catch_main fakeit::fakeit range-v3::range-v3 sole::sole asio::asio Msgpack::Msgpack yaml-cpp::yaml-cpp)

# automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
# to whatever you want, or use different for different binaries
catch_discover_tests(
  mount-instruction-tests
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


add_executable(unmount-instruction-tests 
  test_unmount_instruction.cc
  ${PROJECT_SRC_FOLDER}/domain/images/instructions/unmount_instruction.cc)
target_link_libraries(unmount-instruction-tests PRIVATE catch_main fakeit::fakeit range-v3::range-v3 sole::sole asio::asio Msgpack::Msgpack yaml-cpp::yaml-cpp)

# automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
# to whatever you want, or use different for different binaries
catch_discover_tests(
  unmount-instruction-tests
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

add_executable(registration-instruction-tests 
  test_registration_instruction.cc
  ${PROJECT_SRC_FOLDER}/domain/images/instructions/registration_instruction.cc)
target_link_libraries(registration-instruction-tests PRIVATE catch_main fakeit::fakeit range-v3::range-v3 sole::sole asio::asio Msgpack::Msgpack yaml-cpp::yaml-cpp)

# automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
# to whatever you want, or use different for different binaries
catch_discover_tests(
  registration-instruction-tests
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


add_executable(cleanup-instruction-tests 
  test_cleanup_instruction.cc
  ${PROJECT_SRC_FOLDER}/domain/images/instructions/cleanup_instruction.cc)
target_link_libraries(cleanup-instruction-tests PRIVATE catch_main fakeit::fakeit range-v3::range-v3 sole::sole asio::asio Msgpack::Msgpack yaml-cpp::yaml-cpp)

# automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
# to whatever you want, or use different for different binaries
catch_discover_tests(
  cleanup-instruction-tests
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

add_executable(extraction-instruction-tests 
  test_extraction_instruction.cc
  ${PROJECT_SRC_FOLDER}/domain/images/instructions/extraction_instruction.cc)
target_link_libraries(extraction-instruction-tests PRIVATE catch_main fakeit::fakeit range-v3::range-v3 libzip::zip zstd::libzstd_static sole::sole asio::asio Msgpack::Msgpack yaml-cpp::yaml-cpp archives::rc)

# automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
# to whatever you want, or use different for different binaries
catch_discover_tests(
  extraction-instruction-tests
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


add_executable(import-instruction-tests 
  test_import_instruction.cc
  ${PROJECT_SRC_FOLDER}/domain/images/instructions/import_instruction.cc)
target_link_libraries(import-instruction-tests PRIVATE catch_main fakeit::fakeit range-v3::range-v3 libzip::zip zstd::libzstd_static sole::sole asio::asio Msgpack::Msgpack yaml-cpp::yaml-cpp archives::rc)

# automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
# to whatever you want, or use different for different binaries
catch_discover_tests(
  import-instruction-tests
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


add_executable(image-store-tests 
  test_image_store.cc
  ${PROJECT_SRC_FOLDER}/core/sql/connection.cc 
  ${PROJECT_SRC_FOLDER}/core/sql/data_source.cc 
  ${PROJECT_SRC_FOLDER}/core/sql/migrations.cc 
  ${PROJECT_SRC_FOLDER}/core/sql/result_set.cc 
  ${PROJECT_SRC_FOLDER}/core/sql/statement.cc 
  ${PROJECT_SRC_FOLDER}/core/sql/transaction.cc
  ${PROJECT_SRC_FOLDER}/domain/images/sql_repository.cc)
target_link_libraries(image-store-tests PRIVATE catch_main fakeit::fakeit range-v3::range-v3 sole::sole Msgpack::Msgpack SQLite::SQLite3)

# automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
# to whatever you want, or use different for different binaries
catch_discover_tests(
  image-store-tests
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