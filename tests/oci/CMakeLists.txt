cmake_minimum_required(VERSION 3.15...3.23)

include_directories(${PROJECT_HDRS_FOLDER})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(fakeit REQUIRED CONFIG)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog REQUIRED CONFIG)
find_package(OpenSSL REQUIRED CONFIG)
find_package(sole REQUIRED CONFIG)
find_package(range-v3 REQUIRED CONFIG)
find_package(nlohmann_json REQUIRED CONFIG)
find_package(LibArchive REQUIRED CONFIG)  
find_package(tl-expected REQUIRED CONFIG)  


add_executable(layer-composition-tests 
               layer_composition_tests.cc
               ${PROJECT_SRC_FOLDER}/core/sha/utilities.cc
               ${PROJECT_SRC_FOLDER}/core/archives/helper.cc
               ${PROJECT_SRC_FOLDER}/core/oci/layer_composer.cc
              )

target_link_libraries(layer-composition-tests 
                      PRIVATE 
                      catch_main 
                      fakeit::fakeit 
                      fmt::fmt 
                      spdlog::spdlog_header_only
                      range-v3::range-v3
                      sole::sole
                      LibArchive::LibArchive
                      tl::expected
                      nlohmann_json::nlohmann_json
                      PUBLIC 
                      utilities
                      OpenSSL::Crypto
                      OpenSSL::SSL)
   
# automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
# to whatever you want, or use different for different binaries
catch_discover_tests(
  layer-composition-tests
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
