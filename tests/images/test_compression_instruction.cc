#include <catch2/catch.hpp>
#include <fakeit.hpp>
#include <domain/images/instructions/compression_instruction.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/directory_resolver.h>
#include "utilities.h"
#include <sole.hpp>
#include <fmt/format.h>

using namespace fakeit;
using namespace domain::images;
using namespace domain::images::instructions;

TEST_CASE("compression instruction case")
{
    fs::path image_build_path = fs::path(create_folder("image-build-folder"));

    Mock<instruction_listener> listener;
    Fake(
        Method(listener, on_instruction_initialized),
        Method(listener, on_instruction_data_received),
        Method(listener, on_instruction_complete));
    GIVEN("a valid folder")
    {
        std::string id = sole::uuid4().str();

        Mock<directory_resolver> resolver;
        create_file_in_folder(image_build_path, "file1.txt");
        create_file_in_folder(image_build_path, "file2.txt");
        create_file_in_folder(image_build_path, "folder_1/file1_1.txt");
        create_file_in_folder(image_build_path, "folder_2/file2_2.txt");
        create_file_in_folder(image_build_path, "folder_1/sub_folder_1/file1_1.txt");
        create_file_in_folder(image_build_path, "folder_2/sub_folder_2/file2_2.txt");
        fs::path image_target_path = fs::path(create_folder(fmt::format("{}", id)));
        When(Method(resolver, destination_path).Using(id, Any())).AlwaysReturn(image_build_path);
        When(Method(resolver, generate_image_path).Using(id, Any())).AlwaysReturn(image_target_path);
        WHEN("a folder contains files")
        {

            compression_instruction instruction(id, resolver.get(), listener.get());
            THEN("the files will be packed into a compressed archive")
            {
                std::error_code error = {};
                instruction.execute();
                Verify(Method(listener, on_instruction_initialized).Using(id, "ARCHIVE"));
                Verify(Method(listener, on_instruction_data_received)).AtLeastOnce();
                Verify(Method(listener, on_instruction_complete).Using(id, Eq(error))).Once();
                REQUIRE(fs::exists(image_target_path / fs::path("fs.tar.gz")));
            }
        }
    }
}
