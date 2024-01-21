cmake_minimum_required(VERSION 3.15...3.23)

include_directories(${PROJECT_HDRS_FOLDER})

include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(fakeit REQUIRED CONFIG)
find_package(range-v3 REQUIRED CONFIG)
find_package(sole REQUIRED CONFIG)
find_package(asio REQUIRED CONFIG)
find_package(libzip REQUIRED CONFIG)
find_package(zstd REQUIRED CONFIG)

add_executable(copy-instruction-tests 
               test_copy_instruction.cc
               ${PROJECT_SRC_FOLDER}/domain/images/instructions/copy_instruction.cc)
target_link_libraries(copy-instruction-tests PRIVATE catch_main fakeit::fakeit range-v3::range-v3 sole::sole asio::asio Msgpack::Msgpack)

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
target_link_libraries(download-instruction-tests PRIVATE catch_main fakeit::fakeit range-v3::range-v3 sole::sole asio::asio Msgpack::Msgpack)

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
target_link_libraries(work-dir-instruction-tests PRIVATE catch_main fakeit::fakeit range-v3::range-v3 sole::sole asio::asio Msgpack::Msgpack)

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
target_link_libraries(compression-instruction-tests PRIVATE catch_main fakeit::fakeit range-v3::range-v3 sole::sole libzip::zip zstd::libzstd_static asio::asio Msgpack::Msgpack)

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
target_link_libraries(mount-instruction-tests PRIVATE catch_main fakeit::fakeit range-v3::range-v3 sole::sole asio::asio Msgpack::Msgpack)

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
target_link_libraries(unmount-instruction-tests PRIVATE catch_main fakeit::fakeit range-v3::range-v3 sole::sole asio::asio Msgpack::Msgpack)

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
target_link_libraries(registration-instruction-tests PRIVATE catch_main fakeit::fakeit range-v3::range-v3 sole::sole asio::asio Msgpack::Msgpack)

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
target_link_libraries(cleanup-instruction-tests PRIVATE catch_main fakeit::fakeit range-v3::range-v3 sole::sole asio::asio Msgpack::Msgpack)

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