#include <catch2/catch.hpp>
#include <fakeit.hpp>
#include <domain/images/instructions/copy_instruction.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/errors.h>
#include <domain/images/instructions/directory_resolver.h>
#include "utilities.h"
#include <sole.hpp>

using namespace fakeit;
using namespace domain::images::instructions;

TEST_CASE("copy instruction case")
{
    std::string local_path = create_folder("local-folder");
    std::string destination_path = create_folder("destination-folder");

    Mock<instruction_listener> listener;
    Fake(
        Method(listener, on_instruction_initialized),
        Method(listener, on_instruction_data_received),
        Method(listener, on_instruction_complete));
    GIVEN("a valid origin folder")
    {
        std::string id = sole::uuid4().str();
        Mock<directory_resolver> resolver;
        When(Method(resolver, local_folder)).AlwaysReturn(std::filesystem::path(local_path));
        When(Method(resolver, destination_path).Using(id, Any())).AlwaysReturn(std::filesystem::path(destination_path));

        WHEN("a real target destination folder specified")
        {

            std::string order(". /");
            create_file_in_folder(local_path, "crimes.txt");
            create_file_in_folder(local_path, "stupidity.txt");
            THEN("all folder content will be copied over")
            {
                copy_instruction instruction(id, order, resolver.get(), listener.get());
                instruction.execute();
                std::error_code error{};
                Verify(Method(listener, on_instruction_initialized).Using(id, "COPY")).Once();
                Verify(Method(listener, on_instruction_complete).Using(id, Eq(error))).Once();
            }
        }
        WHEN("a non existent destination folder is specified")
        {
            std::string order(". /apps");
            THEN("an error will be returned")
            {
                copy_instruction instruction(id, order, resolver.get(), listener.get());
                std::error_code error = make_error_code(error_code::invalid_destination);
                instruction.execute();
                Verify(Method(listener, on_instruction_complete).Using(id, Eq(error))).Once();
                VerifyNoOtherInvocations(listener);
            }
        }

        WHEN("a real target destination is specified")
        {
            create_file_in_folder(local_path, "crimes-20.txt");
            THEN("file will be transferred")
            {
                std::string order("./crimes-20.txt .");
                copy_instruction instruction(id, order, resolver.get(), listener.get());
                instruction.execute();
                std::error_code error{};
                Verify(Method(listener, on_instruction_initialized).Using(id, "COPY")).Once();
                Verify(Method(listener, on_instruction_complete).Using(id, Eq(error))).Once();
            }
        }

        WHEN("a non existent origin file is specified")
        {
            std::string order("./lies.txt .");
            THEN("an error will be returned")
            {
                copy_instruction instruction(id, order, resolver.get(), listener.get());
                std::error_code error = make_error_code(error_code::invalid_origin);
                instruction.execute();
                Verify(Method(listener, on_instruction_complete).Using(id, Eq(error))).Once();
                VerifyNoOtherInvocations(listener);
            }
        }
    }
    GIVEN("an existing external image folder")
    {
        std::string id = sole::uuid4().str();
        std::string previous_stage_path = create_folder("previous-stage-folder");
        create_file_in_folder(previous_stage_path, "crimes-20-1.txt");
        create_file_in_folder(previous_stage_path, "crimes-20-2.txt");
        create_file_in_folder(previous_stage_path, "crimes-20-3.txt");
        Mock<directory_resolver> resolver;
        When(Method(resolver, local_folder)).AlwaysReturn(std::filesystem::path(local_path));
        When(Method(resolver, destination_path).Using(id, Any())).AlwaysReturn(std::filesystem::path(destination_path));
        When(Method(resolver, stage_path).Using("0", Any())).AlwaysReturn(std::filesystem::path(previous_stage_path));
        When(Method(resolver, stage_path).Using("BASE", Any())).AlwaysReturn(std::filesystem::path(previous_stage_path));
        WHEN("a file exists on another image stage")
        {
            THEN("a file can be copied to the current image stage by number")
            {
                std::string order("--from=0 ./crimes-20-1.txt .");
                copy_instruction instruction(id, order, resolver.get(), listener.get());
                std::error_code error{};
                instruction.execute();
                Verify(Method(listener, on_instruction_initialized).Using(id, "COPY")).Once();
                Verify(Method(listener, on_instruction_complete).Using(id, Eq(error))).Once();
            }
            THEN("a file can be copied to the current image stage by alias")
            {
                std::string order("--from=BASE ./crimes-20-1.txt .");
                copy_instruction instruction(id, order, resolver.get(), listener.get());
                std::error_code error{};
                instruction.execute();
                Verify(Method(listener, on_instruction_initialized).Using(id, "COPY")).Once();
                Verify(Method(listener, on_instruction_complete).Using(id, Eq(error))).Once();
            }
        }
        WHEN("a file does not exist on another image stage")
        {
            THEN("copying the target will result in an error")
            {
                std::string order("--from=0 ./crimes-unknown.txt .");
                copy_instruction instruction(id, order, resolver.get(), listener.get());
                std::error_code error = make_error_code(error_code::invalid_origin);
                instruction.execute();
                Verify(Method(listener, on_instruction_complete).Using(id, Eq(error))).Once();
                VerifyNoOtherInvocations(listener);
            }
        }
    }
}