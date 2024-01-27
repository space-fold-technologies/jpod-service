cmake_minimum_required(VERSION 3.15...3.23)

include_directories(${PROJECT_HDRS_FOLDER})

include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(fakeit REQUIRED CONFIG)
find_package(range-v3 REQUIRED CONFIG)
find_package(sole REQUIRED CONFIG)
find_package(asio REQUIRED CONFIG)

add_executable(address-provider-tests 
               test_address_provider.cc
               ${PROJECT_SRC_FOLDER}/domain/networking/asio_address_provider.cc)
target_link_libraries(address-provider-tests PRIVATE catch_main fakeit::fakeit range-v3::range-v3 sole::sole asio::asio)

# automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
# to whatever you want, or use different for different binaries
catch_discover_tests(
  address-provider-tests
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

