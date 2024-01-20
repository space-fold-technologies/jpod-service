#include <catch2/catch.hpp>
#include <fakeit.hpp>
#include <domain/images/instructions/work_dir_instruction.h>
#include <domain/images/instructions/instruction_listener.h>

#include "utilities.h"
#include <sole.hpp>

using namespace fakeit;
using namespace domain::images::instructions;

TEST_CASE("work dir instruction case")
{
    fs::path destination_path = fs::path(create_folder("destination-folder"));

    Mock<instruction_listener> listener;
    Fake(
        Method(listener, on_instruction_initialized),
        Method(listener, on_instruction_data_received),
        Method(listener, on_instruction_complete));
    GIVEN("a valid folder")
    {
        std::string id = sole::uuid4().str();
        WHEN("a folder does not exist")
        {
            std::string order("neza");
            work_dir_instruction instruction(id, order, destination_path, listener.get());
            THEN("changing to it will fail")
            {
                std::error_code error = make_error_code(error_code::no_work_directory);
                instruction.execute();
                Verify(Method(listener, on_instruction_complete).Using(id, Eq(error))).Once();
                VerifyNoOtherInvocations(listener);
            }
        }
        WHEN("a file is specified")
        {
            create_file_in_folder(destination_path.string(), "app/file.txt");
            std::string order("app/file.txt");
            work_dir_instruction instruction(id, order, destination_path, listener.get());
            THEN("changing to it will fail")
            {
                std::error_code error = make_error_code(error_code::no_work_directory);
                instruction.execute();
                Verify(Method(listener, on_instruction_complete).Using(id, Eq(error))).Once();
                VerifyNoOtherInvocations(listener);
            }
        }
        WHEN("a known folder is specified back")
        {
            auto target(destination_path);
            create_sub_folder(destination_path.string(), "recker");
            std::string order("recker");
            work_dir_instruction instruction(id, order, target, listener.get());
            THEN("a change to the folder will be made")
            {
                std::error_code error = {};
                instruction.execute();
                Verify(Method(listener, on_instruction_initialized).Using(id, "WORKDIR")).Once();
                Verify(Method(listener, on_instruction_complete).Using(id, Eq(error)));
                REQUIRE(target == destination_path / fs::path("recker"));
            }
        }
    }
}