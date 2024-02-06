#include <catch2/catch.hpp>
#include <fakeit.hpp>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/errors.h>
#include <domain/images/instructions/directory_resolver.h>
#include <domain/images/instructions/download_instruction.h>
#include <domain/images/repository.h>
#include <domain/images/http/client.h>
#include <domain/images/http/contracts.h>
#include "utilities.h"
#include <sole.hpp>

using namespace fakeit;
using namespace domain::images::instructions;
using namespace domain::images::http;
using namespace domain::images;

TEST_CASE("download instruction case")
{

    std::string destination_path = create_folder("destination-folder");

    Mock<instruction_listener> listener;
    Mock<client> client;

    Fake(
        Method(listener, on_instruction_initialized),
        Method(listener, on_instruction_data_received),
        Method(listener, on_instruction_complete));

    GIVEN("a valid image identifier")
    {
        std::string id = sole::uuid4().str();
        Mock<directory_resolver> resolver;
        When(Method(resolver, destination_path).Using(id, Any())).AlwaysReturn(std::filesystem::path(destination_path));
        WHEN("no valid registries available")
        {
            Mock<image_repository> repository;
            When(Method(repository, fetch_registry_by_path).Using(Eq("funky.pods.com"))).Return(std::nullopt);
            When(Method(repository, has_image).Using("funky.pods.com", "freebsd", "14")).Return(false);
            std::string order("funky.pods.com/freebsd:14");
            download_instruction instruction(id, order, client.get(), repository.get(), resolver.get(), listener.get());

            THEN("download will fail with no valid registries")
            {
                std::error_code error = make_error_code(error_code::no_registry_entries_found);
                instruction.execute();
                Verify(Method(listener, on_instruction_complete).Using(id, Eq(error))).Once();
                VerifyNoOtherInvocations(listener);
            }
        }
        WHEN("no repository with target image")
        {
            Mock<image_repository> repository;
            When(Method(repository, fetch_registry_by_path).Using(Eq("fox-soft.pods.com"))).Return(fox_soft_registry);
            When(Method(repository, has_image).Using("fox-soft.pods.com", "freebsd", "14")).Return(false);
            When(Method(client, execute).Using(Any(), Any())).Do(no_image_found);
            std::string order("fox-soft.pods.com/freebsd:14");
            download_instruction instruction(id, order, client.get(), repository.get(), resolver.get(), listener.get());
            THEN("download will fail no registry found with image")
            {
                std::error_code error = make_error_code(error_code::no_matching_image_found);
                instruction.execute();
                Verify(Method(listener, on_instruction_complete).Using(id, Eq(error))).Once();
                VerifyNoOtherInvocations(listener);
            }
        }
        WHEN("credentials to a target repository are not valid expired")
        {
            Mock<image_repository> repository;
            When(Method(repository, fetch_registry_by_path).Using(Eq("fox-soft.pods.com"))).Return(fox_soft_registry);
            When(Method(repository, has_image).Using("fox-soft.pods.com", "freebsd", "14")).Return(false);
            When(Method(client, execute).Using(Any(), Any())).Do(no_registry_access);
            std::string order("fox-soft.pods.com/freebsd:14");
            download_instruction instruction(id, order, client.get(), repository.get(), resolver.get(), listener.get());
            THEN("download will fail with an http authorization error")
            {
                std::error_code error = make_error_code(error_code::no_registry_access);
                instruction.execute();
                Verify(Method(listener, on_instruction_complete).Using(id, Eq(error))).Once();
                VerifyNoOtherInvocations(listener);
            }
        }
        WHEN("valid credentials are available for the target repository")
        {
            Mock<image_repository> repository;
            When(Method(repository, fetch_registry_by_path).Using(Eq("wikin.pods.com"))).Return(wikin_registry);
            When(Method(repository, has_image).Using("wikin.pods.com", "freebsd", "14")).Return(false);
            When(Method(client, execute).Using(Any(), Any())).Do(access_permitted);
            std::map<std::string, std::string> headers;
            When(Method(client, download).Using(Any(), Any(), Any(), Any())).Do(download_invoked);
            When(Method(resolver, extract_image).Using(Any(), Any(), Any())).Do(extraction_invoked);
            When(Method(repository, save_image_details).Using(Any())).Return({});
            When(Method(resolver, generate_image_path).Using(Any(), Any())).Return(destination_path);
            std::string order("wikin.pods.com/freebsd:14");
            auto instruction = std::make_shared<download_instruction>(id, order, client.get(), repository.get(), resolver.get(), listener.get());
            THEN("download will be performed and image will be extracted to target space")
            {
                std::error_code error = {};
                instruction->execute();
                Verify(Method(listener, on_instruction_initialized).Using(id, "FROM"));
                Verify(Method(listener, on_instruction_data_received).Using(id, _)).AtLeastOnce();
                Verify(Method(resolver, generate_image_path).Using(_, error)).Once();
                Verify(Method(repository, save_image_details)).Once();
                Verify(Method(resolver, extract_image).Using(id, _, _)).Once();
                Verify(Method(listener, on_instruction_complete).Using(id, Eq(error))).Once();
            }
        }
        WHEN("the image identifier matches an already downloaded image")
        {
            Mock<image_repository> repository;
            When(Method(repository, has_image).Using("saber.pods.com", "freebsd", "14")).Return(true);
            When(Method(resolver, generate_image_path).Using(Any(), Any())).Return(destination_path);
            When(Method(resolver, extract_image).Using(Any(), Any(), Any())).Do(extraction_invoked);
            When(Method(repository, fetch_image_identifier).Using(Any(), Any(), Any())).Return(std::optional<std::string>("71dbec89-cad4-4f60-a73f-9be9a7ba6aca"));
            std::string order("saber.pods.com/freebsd:14");
            auto instruction = std::make_shared<download_instruction>(id, order, client.get(), repository.get(), resolver.get(), listener.get());
            THEN("no download will be performed and the image will be extracted to target space")
            {
                std::error_code error = {};
                instruction->execute();
                Verify(Method(listener, on_instruction_initialized).Using(id, "FROM"));
                Verify(Method(listener, on_instruction_data_received).Using(id, _)).AtLeastOnce();
                Verify(Method(repository, fetch_image_identifier)).Once();
                Verify(Method(resolver, extract_image).Using(id, Any(), Any())).Once();
                Verify(Method(listener, on_instruction_complete).Using(id, Eq(error))).Once();
            }
        }
    }
}