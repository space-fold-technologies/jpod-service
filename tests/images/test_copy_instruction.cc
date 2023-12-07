#include <catch2/catch.hpp>
#include <fakeit.hpp>
#include <images/copy_instruction.h>
#include "utilities.h"
#include <sole.hpp>

using namespace fakeit;
using namespace images;

TEST_CASE("copy instruction case")
{
    std::string local_path = create_folder("local-folder");
    std::string destination_path = create_folder("destination-folder");
    Mock<InstructionListener> mock;
    Fake(
        Method(mock, on_instruction_runner_initialized),
        Method(mock, on_instruction_runner_data_received),
        Method(mock, on_instruction_runner_completion));
    GIVEN("a valid origin folder")
    {
        std::string id = sole::uuid4().str();
        WHEN("a real target destination folder specified")
        {

            std::string order(". /");
            create_file_in_folder(local_path, "crimes.txt");
            create_file_in_folder(local_path, "stupidity.txt");
            THEN("all folder content will be copied over")
            {
                CopyInstruction instruction(id, order, local_path, destination_path, mock.get());
                auto err = instruction.parse();
                Verify(Method(mock, on_instruction_runner_initialized).Using(id)).Once();
                REQUIRE(!err);
                instruction.execute();

                // cannot match std::error_code as an invoked argument
                Verify(Method(mock, on_instruction_runner_completion).Using(id, _)).Once();
            }
        }
        WHEN("a non existent destination folder is specified")
        {
            std::string order(". /apps");
            THEN("an error will be returned")
            {
                CopyInstruction instruction(id, order, local_path, destination_path, mock.get());
                auto err = instruction.parse();
                REQUIRE(err);
                VerifyNoOtherInvocations(mock);
            }
        }

        WHEN("a real target destination is specified")
        {
            create_file_in_folder(local_path, "crimes-20.txt");
            THEN("file will be transferred")
            {
                std::string order("./crimes-20.txt .");
                CopyInstruction instruction(id, order, local_path, destination_path, mock.get());
                auto err = instruction.parse();
                REQUIRE(!err);
                instruction.execute();
                Verify(Method(mock, on_instruction_runner_initialized).Using(id)).Once();
                Verify(Method(mock, on_instruction_runner_completion).Using(id, _)).Once();
            }
        }

        WHEN("a non existent origin file is specified")
        {
            std::string order("./lies.txt .");
            THEN("an error will be returned")
            {
                CopyInstruction instruction(id, order, local_path, destination_path, mock.get());
                auto err = instruction.parse();
                REQUIRE(err);
                VerifyNoOtherInvocations(mock);
            }
        }
    }
}