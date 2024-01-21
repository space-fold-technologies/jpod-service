#include <catch2/catch.hpp>
#include <fakeit.hpp>
#include <domain/images/instructions/registration_instruction.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/directory_resolver.h>
#include <domain/images/repository.h>
#include "utilities.h"
#include <sole.hpp>
#include <fmt/format.h>

using namespace fakeit;
using namespace domain::images;
using namespace domain::images::instructions;

TEST_CASE("registration instruction case")
{
    Mock<instruction_listener> listener;
    Fake(
        Method(listener, on_instruction_initialized),
        Method(listener, on_instruction_data_received),
        Method(listener, on_instruction_complete));
    GIVEN("given an image properties")
    {
        std::string id = sole::uuid4().str();

        Mock<directory_resolver> resolver;
        Mock<image_repository> repository;
        image_properties properties;
        fs::path image_target_path = fs::path(create_folder(fmt::format("{}", id)));
        When(Method(resolver, destination_path).Using(id, Any())).AlwaysReturn(image_target_path);
        When(Method(repository, fetch_image_details).Using(Any(), Any(), Any())).Return(dummy_image_details());
        When(Method(repository, save_image_details).Using(Any())).Return(std::error_code{});
        WHEN("when a stage completes its build from a previous base")
        {
            std::string order("saber.pods.com/freebsd:14");
            properties.entry_point = "java -jar app.jar";
            properties.name = "ram-hoger-fx";
            properties.parent_image_order = "saber.pods.com/freebsd:14";
            properties.labels.emplace("EMAIL", "morty@spg.com");
            properties.tag = "latest";
            properties.env_vars.emplace("ACCESS_KEY", "default");
            registration_instruction instruction(id, properties, repository.get(), listener.get());
            THEN("then the aggregate of both the base image and the added user properties are persisted")
            {
                std::error_code error = {};
                instruction.execute();
                Verify(Method(listener, on_instruction_initialized).Using(id, "REGISTRATION"));
                Verify(Method(repository, save_image_details)).Once();
                Verify(Method(listener, on_instruction_complete).Using(id, Eq(error))).Once();
        
            }
        }
    }
}
