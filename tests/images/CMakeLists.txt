cmake_minimum_required(VERSION 3.15...3.23)

include_directories(${PROJECT_HDRS_FOLDER})

include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(fakeit REQUIRED)
find_package(range-v3 REQUIRED)
find_package(sole REQUIRED)

add_executable(copy-instruction-tests 
               test_copy_instruction.cc
               ${PROJECT_SRC_FOLDER}/images/copy_instruction.cc)
target_link_libraries(copy-instruction-tests PRIVATE catch_main fakeit::fakeit range-v3::range-v3 sole::sole)

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