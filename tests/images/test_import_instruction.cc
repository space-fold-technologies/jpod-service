#include <catch2/catch.hpp>
#include <fakeit.hpp>
#include <domain/images/instructions/import_instruction.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/import_resolver.h>
#include <domain/images/repository.h>
#include "utilities.h"
#include "embedded_fs.h"
#include <sole.hpp>
#include <fmt/format.h>

using namespace fakeit;
using namespace domain::images;
using namespace domain::images::instructions;

TEST_CASE("import instruction case")
{
    Mock<instruction_listener> listener;
    Fake(
        Method(listener, on_instruction_initialized),
        Method(listener, on_instruction_data_received),
        Method(listener, on_instruction_complete));
    GIVEN("an order with a valid path to an archive")
    {
        Mock<import_resolver> resolver;
        Mock<image_repository> repository;
        WHEN("the path is given to the import instruction")
        {
            std::string id = sole::uuid4().str();
            fs::path image_target_path = fs::path(create_folder(fmt::format("{}", id)));
            When(Method(resolver, archive_file_path)).Return(from_archives("archives/dummy.zip"));
            When(Method(repository, save_image_details).Using(Any())).Return(std::error_code{});
            import_instruction instruction(id, repository.get(), resolver.get(), listener.get());
            THEN("the image details will be imported")
            {
                std::error_code error = {};
                instruction.execute();
                Verify(Method(listener, on_instruction_initialized).Using(id, "IMPORT"));
                Verify(Method(listener, on_instruction_data_received).Using(id, Any())).AtLeastOnce();
                Verify(Method(listener, on_instruction_complete).Using(id, Eq(error))).Once();
            }
        }
    }
}