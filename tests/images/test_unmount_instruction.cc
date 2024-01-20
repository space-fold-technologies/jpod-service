#include <catch2/catch.hpp>
#include <fakeit.hpp>
#include <domain/images/instructions/unmount_instruction.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/directory_resolver.h>
#include <domain/images/repository.h>
#include "utilities.h"
#include <sole.hpp>
#include <fmt/format.h>

using namespace fakeit;
using namespace domain::images;
using namespace domain::images::instructions;

TEST_CASE("unmount instruction case")
{
    fs::path image_build_path = fs::path(create_folder("image-build-folder"));

    Mock<instruction_listener> listener;
    Fake(
        Method(listener, on_instruction_initialized),
        Method(listener, on_instruction_data_received),
        Method(listener, on_instruction_complete));
    GIVEN("given an image build from a known registry")
    {
        std::string id = sole::uuid4().str();

        Mock<directory_resolver> resolver;
        Mock<image_repository> repository;
        fs::path image_target_path = fs::path(create_folder(fmt::format("{}", id)));
        create_sub_folder(image_target_path.generic_string(), "dev");
        create_sub_folder(image_target_path.generic_string(), "sys");
        create_sub_folder(image_target_path.generic_string(), "proc");
        create_sub_folder(image_target_path.generic_string(), "dev/shm");
        When(Method(resolver, destination_path).Using(id, Any())).AlwaysReturn(image_target_path);
        When(Method(repository, fetch_image_mount_points).Using(Any(), Any(), Any())).Return(mount_points());
        WHEN("when the base image mount points are resolved")
        {   
            std::string order("saber.pods.com/freebsd:14");
            unmount_instruction instruction(id, order, repository.get(), resolver.get(), listener.get());
            THEN("the mounted folders will be unmounted")
            {
                std::error_code error = {};
                instruction.execute();
                Verify(Method(listener, on_instruction_initialized).Using(id, "UNMOUNT"));
                Verify(Method(listener, on_instruction_complete).Using(id, Eq(error))).Once();
            }
        }
    }
}
