#include <catch2/catch.hpp>
#include <fakeit.hpp>
#include <domain/images/instructions/cleanup_instruction.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/directory_resolver.h>
#include <domain/images/repository.h>
#include "utilities.h"
#include <sole.hpp>
#include <fmt/format.h>

using namespace fakeit;
using namespace domain::images;
using namespace domain::images::instructions;

TEST_CASE("cleanup instruction case")
{
    fs::path image_build_path = fs::path(create_folder("image-build-folder"));

    Mock<instruction_listener> listener;
    Fake(
        Method(listener, on_instruction_initialized),
        Method(listener, on_instruction_data_received),
        Method(listener, on_instruction_complete));
    GIVEN("given completed stages")
    {
        std::string first_stage = sole::uuid4().str();
        std::string last_stage = sole::uuid4().str();

        Mock<directory_resolver> resolver;
        Mock<image_repository> repository;
        fs::path first_stage_path = fs::path(create_folder(fmt::format("{}", first_stage)));
        create_file_in_folder(first_stage_path.generic_string(), "trash_0.txt");
        create_file_in_folder(first_stage_path.generic_string(), "trash_1.txt");
        create_file_in_folder(first_stage_path.generic_string(), "trash_2.txt");
        create_file_in_folder(first_stage_path.generic_string(), "trash_3.txt");
        fs::path last_stage_path = fs::path(create_folder(fmt::format("{}", last_stage)));
        create_file_in_folder(last_stage_path.generic_string(), "trash_0.txt");
        create_file_in_folder(last_stage_path.generic_string(), "trash_1.txt");
        create_file_in_folder(last_stage_path.generic_string(), "trash_2.txt");
        create_file_in_folder(last_stage_path.generic_string(), "trash_3.txt");
        When(Method(resolver, destination_path).Using(first_stage, Any())).AlwaysReturn(first_stage_path);
        When(Method(resolver, destination_path).Using(last_stage, Any())).AlwaysReturn(last_stage_path);
       
        WHEN("when the completed stages are passed for clean up")
        {
            std::vector<std::string> targets{first_stage, last_stage};
            
            cleanup_instruction instruction(last_stage, targets, resolver.get(), listener.get());
            THEN("then the folders and there contents are removed")
            {
                std::error_code error = {};
                instruction.execute();
                Verify(Method(listener, on_instruction_initialized).Using(last_stage, "CLEANUP"));
                Verify(Method(listener, on_instruction_complete).Using(last_stage, Eq(error))).Once();
                REQUIRE_FALSE(fs::exists(first_stage_path));
                REQUIRE_FALSE(fs::exists(last_stage_path));
            }
        }
    }
}
