#include <catch2/catch.hpp>
#include <core/oci/layer_composer.h>
#include <nlohmann/json.hpp>
#include <utilities/embedded_fs.h>
#include <fstream>

using json = nlohmann::json;

// https://github.com/opencontainers/image-spec/blob/main/layer.md
TEST_CASE("layer composition test case")
{
    SECTION("files are added to a root filesystem")
    {
        auto root = test::utilities::from_archives("root.tar.gz");
        fs::path destination(fs::current_path() / fs::path("layer-0"));
        test::utilities::clean(destination);
        fs::path layer_file(fs::current_path() / fs::path("layer.tar.gz"));
        auto result = core::oci::initialize(root, destination, layer_file)
                          .and_then(core::oci::copy_root)
                          .and_then(core::oci::snapshot_target);
        test::utilities::create_to_filesystem(destination, "/home/app/README.md");
        REQUIRE(result.has_value());
        result = core::oci::diff_to_target(result.value());
        REQUIRE(result.has_value());
        auto target = destination / fs::path("home/app/README.md");
        REQUIRE_FALSE(result->changes.empty());
        REQUIRE(result->changes.find(target.string()) != result->changes.end());
        auto entry = result->changes.at(target.string());
        REQUIRE(entry.type == core::oci::change_type::added);
        REQUIRE(target == fs::path(entry.path));
    }
    SECTION("files are removed from a copy of the root filesystem")
    {
        auto root = test::utilities::from_archives("root-with-documentation.tar.gz");
        fs::path destination(fs::current_path() / fs::path("layer-0"));
        test::utilities::clean(destination);
        fs::path layer_file(fs::current_path() / fs::path("layer.tar.gz"));
        auto result = core::oci::initialize(root, destination, layer_file)
                          .and_then(core::oci::copy_root)
                          .and_then(core::oci::snapshot_target);
        test::utilities::remove_from_filesystem(destination, "/root-with-documentation/home/app/README.md");
        result = core::oci::diff_to_target(result.value());
        REQUIRE(result.has_value());
        REQUIRE_FALSE(result->changes.empty());
        auto target = destination / fs::path("root-with-documentation/home/app/README.md");
        REQUIRE(result->changes.find(target.string()) != result->changes.end());
        auto entry = result->changes.at(target.string());
        REQUIRE(entry.type == core::oci::change_type::deleted);
        REQUIRE(fs::path(fs::path(destination) / fs::path("root-with-documentation/home/app/.wh.README.md")) == fs::path(entry.path));
    }
    SECTION("files are modified from a copy of the root file-system")
    {
        auto root = test::utilities::from_archives("root-with-documentation.tar.gz");
        fs::path destination(fs::current_path() / fs::path("layer-0"));
        test::utilities::clean(destination);
        fs::path layer_file(fs::current_path() / fs::path("layer.tar.gz"));
        auto result = core::oci::initialize(root, destination, layer_file)
                          .and_then(core::oci::copy_root)
                          .and_then(core::oci::snapshot_target);
        test::utilities::update_on_filesystem(destination, "/root-with-documentation/home/app/README.md", "Nothing to see here");
        result = core::oci::diff_to_target(result.value());
        REQUIRE(result.has_value());
        REQUIRE_FALSE(result->changes.empty());
        auto target = destination / fs::path("root-with-documentation/home/app/README.md");
        REQUIRE(result->changes.find(target.string()) != result->changes.end());
        auto entry = result->changes.at(target.string());
        REQUIRE(entry.type == core::oci::change_type::modified);
        REQUIRE(fs::path(fs::path(destination) / fs::path("root-with-documentation/home/app/README.md")) == fs::path(entry.path));
    }
    SECTION("multiple folders are added")
    {
        auto root = test::utilities::from_archives("root.tar.gz");
        fs::path destination(fs::current_path() / fs::path("layer-0"));
        test::utilities::clean(destination);
        fs::path layer_file(fs::current_path() / fs::path("layer.tar.gz"));
        auto result = core::oci::initialize(root, destination, layer_file)
                          .and_then(core::oci::copy_root)
                          .and_then(core::oci::snapshot_target);
        test::utilities::create_to_filesystem(destination, "/root/home/core/README.md");
        test::utilities::create_to_filesystem(destination, "/root/home/legal/LICENSE.txt");
        test::utilities::create_to_filesystem(destination, "/root/home/doc/INSTALL.txt");
        result = core::oci::diff_to_target(result.value());
        REQUIRE(result.has_value());
        REQUIRE_FALSE(result->changes.empty());

        auto target = destination / fs::path("root/home/core/README.md");
        REQUIRE(result->changes.find(target.string()) != result->changes.end());
        auto entry = result->changes.at(target.string());
        REQUIRE(entry.type == core::oci::change_type::added);
        REQUIRE(fs::path(fs::path(destination) / fs::path("root/home/core/README.md")) == fs::path(entry.path));

        target = destination / fs::path("root/home/legal/LICENSE.txt");
        REQUIRE(result->changes.find(target.string()) != result->changes.end());
        entry = result->changes.at(target.string());
        REQUIRE(entry.type == core::oci::change_type::added);
        REQUIRE(fs::path(fs::path(destination) / fs::path("root/home/legal/LICENSE.txt")) == fs::path(entry.path));

        target = destination / fs::path("root/home/doc/INSTALL.txt");
        REQUIRE(result->changes.find(target.string()) != result->changes.end());
        entry = result->changes.at(target.string());
        REQUIRE(entry.type == core::oci::change_type::added);
        REQUIRE(fs::path(fs::path(destination) / fs::path("root/home/doc/INSTALL.txt")) == fs::path(entry.path));
    }
    SECTION("multiple folders are deleted")
    {
        auto root = test::utilities::from_archives("root-documented-multi-folder.tar.gz");
        fs::path destination(fs::current_path() / fs::path("layer-0"));
        test::utilities::clean(destination);
        fs::path layer_file(fs::current_path() / fs::path("layer.tar.gz"));
        auto result = core::oci::initialize(root, destination, layer_file)
                          .and_then(core::oci::copy_root)
                          .and_then(core::oci::snapshot_target);
        test::utilities::remove_from_filesystem(destination, "root-documented-multi-folder/home/core/README.md");
        test::utilities::remove_from_filesystem(destination, "root-documented-multi-folder/home/legal/LICENSE.txt");
        test::utilities::remove_from_filesystem(destination, "root-documented-multi-folder/home/doc/INSTALL.txt");
        result = core::oci::diff_to_target(result.value());
        REQUIRE(result.has_value());
        REQUIRE_FALSE(result->changes.empty());
        auto target = destination / fs::path("root-documented-multi-folder/home/core/README.md");

        REQUIRE(result->changes.find(target.string()) != result->changes.end());
        auto entry = result->changes.at(target.string());
        REQUIRE(entry.type == core::oci::change_type::deleted);
        REQUIRE(fs::path(fs::path(destination) / fs::path("root-documented-multi-folder/home/core/.wh.README.md")) == fs::path(entry.path));

        target = destination / fs::path("root-documented-multi-folder/home/legal/LICENSE.txt");
        REQUIRE(result->changes.find(target.string()) != result->changes.end());
        entry = result->changes.at(target.string());
        REQUIRE(entry.type == core::oci::change_type::deleted);
        REQUIRE(fs::path(fs::path(destination) / fs::path("root-documented-multi-folder/home/legal/.wh.LICENSE.txt")) == fs::path(entry.path));

        target = destination / fs::path("root-documented-multi-folder/home/doc/INSTALL.txt");
        REQUIRE(result->changes.find(target.string()) != result->changes.end());
        entry = result->changes.at(target.string());
        REQUIRE(entry.type == core::oci::change_type::deleted);
        REQUIRE(fs::path(fs::path(destination) / fs::path("root-documented-multi-folder/home/doc/.wh.INSTALL.txt")) == fs::path(entry.path));
    }
    SECTION("multiple folders are updated")
    {
        auto root = test::utilities::from_archives("root-documented-multi-folder.tar.gz");
        fs::path destination(fs::current_path() / fs::path("layer-0"));
        test::utilities::clean(destination);
        fs::path layer_file(fs::current_path() / fs::path("layer.tar.gz"));
        auto result = core::oci::initialize(root, destination, layer_file)
                          .and_then(core::oci::copy_root)
                          .and_then(core::oci::snapshot_target);
        test::utilities::update_on_filesystem(destination, "root-documented-multi-folder/home/core/README.md", "forget about it");
        test::utilities::update_on_filesystem(destination, "root-documented-multi-folder/home/legal/LICENSE.txt", "forget about it");
        test::utilities::update_on_filesystem(destination, "root-documented-multi-folder/home/doc/INSTALL.txt", "forget about it");
        result = core::oci::diff_to_target(result.value());
        REQUIRE(result.has_value());
        REQUIRE_FALSE(result->changes.empty());

        auto target = destination / fs::path("root-documented-multi-folder/home/core/README.md");
        REQUIRE(result->changes.find(target.string()) != result->changes.end());
        auto entry = result->changes.at(target.string());
        REQUIRE(entry.type == core::oci::change_type::modified);
        REQUIRE(fs::path(fs::path(destination) / fs::path("root-documented-multi-folder/home/core/README.md")) == fs::path(entry.path));

        target = destination / fs::path("root-documented-multi-folder/home/legal/LICENSE.txt");
        REQUIRE(result->changes.find(target.string()) != result->changes.end());
        entry = result->changes.at(target.string());
        REQUIRE(entry.type == core::oci::change_type::modified);
        REQUIRE(fs::path(fs::path(destination) / fs::path("root-documented-multi-folder/home/legal/LICENSE.txt")) == fs::path(entry.path));

        target = destination / fs::path("root-documented-multi-folder/home/doc/INSTALL.txt");
        REQUIRE(result->changes.find(target.string()) != result->changes.end());
        entry = result->changes.at(target.string());
        REQUIRE(entry.type == core::oci::change_type::modified);
        REQUIRE(fs::path(fs::path(destination) / fs::path("root-documented-multi-folder/home/doc/INSTALL.txt")) == fs::path(entry.path));
    }
    SECTION("nested folders are added")
    {
        auto root = test::utilities::from_archives("root.tar.gz");
        fs::path destination(fs::current_path() / fs::path("layer-0"));
        test::utilities::clean(destination);
        test::utilities::clean(destination);
        fs::path layer_file(fs::current_path() / fs::path("layer.tar.gz"));
        auto result = core::oci::initialize(root, destination, layer_file)
                          .and_then(core::oci::copy_root)
                          .and_then(core::oci::snapshot_target);
        test::utilities::create_to_filesystem(destination, "root/home/docs/READMEx.md");
        test::utilities::create_to_filesystem(destination, "root/home/docs/legal/LICENSE.txt");
        test::utilities::create_to_filesystem(destination, "root/home/docs/setup/INSTALL.md");
        test::utilities::create_to_filesystem(destination, "root/home/docs/setup/locale/INSTALL-en.md");
        test::utilities::create_to_filesystem(destination, "root/home/docs/setup/locale/en/flag.png");
        result = core::oci::diff_to_target(result.value());
        REQUIRE(result.has_value());
        REQUIRE_FALSE(result->changes.empty());

        auto target = destination / fs::path("root/home/docs/READMEx.md");
        REQUIRE(result->changes.find(target.string()) != result->changes.end());
        auto entry = result->changes.at(target.string());
        REQUIRE(entry.type == core::oci::change_type::added);
        REQUIRE(fs::path(fs::path(destination) / fs::path("root/home/docs/READMEx.md")) == fs::path(entry.path));

        target = destination / fs::path("root/home/docs/legal/LICENSE.txt");
        REQUIRE(result->changes.find(target.string()) != result->changes.end());
        entry = result->changes.at(target.string());
        REQUIRE(entry.type == core::oci::change_type::added);
        REQUIRE(fs::path(fs::path(destination) / fs::path("root/home/docs/legal/LICENSE.txt")) == fs::path(entry.path));

        target = destination / fs::path("root/home/docs/setup/INSTALL.md");
        REQUIRE(result->changes.find(target.string()) != result->changes.end());
        entry = result->changes.at(target.string());
        REQUIRE(entry.type == core::oci::change_type::added);
        REQUIRE(fs::path(fs::path(destination) / fs::path("root/home/docs/setup/INSTALL.md")) == fs::path(entry.path));

        target = destination / fs::path("root/home/docs/setup/locale/INSTALL-en.md");
        REQUIRE(result->changes.find(target.string()) != result->changes.end());
        entry = result->changes.at(target.string());
        REQUIRE(entry.type == core::oci::change_type::added);
        REQUIRE(fs::path(fs::path(destination) / fs::path("root/home/docs/setup/locale/INSTALL-en.md")) == fs::path(entry.path));

        target = destination / fs::path("root/home/docs/setup/locale/en/flag.png");
        REQUIRE(result->changes.find(target.string()) != result->changes.end());
        entry = result->changes.at(target.string());
        REQUIRE(entry.type == core::oci::change_type::added);
        REQUIRE(fs::path(fs::path(destination) / fs::path("root/home/docs/setup/locale/en/flag.png")) == fs::path(entry.path));
    }
    SECTION("nested folders are deleted")
    {
        auto root = test::utilities::from_archives("root-documented-nested-folder.tar.gz");
        fs::path destination(fs::current_path() / fs::path("layer-0"));
        test::utilities::clean(destination);
        test::utilities::clean(destination);
        fs::path layer_file(fs::current_path() / fs::path("layer.tar.gz"));
        auto result = core::oci::initialize(root, destination, layer_file)
                          .and_then(core::oci::copy_root)
                          .and_then(core::oci::snapshot_target);
        test::utilities::remove_from_filesystem(destination, "root-documented-nested-folder/home/docs/README.md");
        test::utilities::remove_from_filesystem(destination, "root-documented-nested-folder/home/docs/legal/LICENSE.txt");
        test::utilities::remove_from_filesystem(destination, "root-documented-nested-folder/home/docs/setup/INSTALL.md");
        test::utilities::remove_from_filesystem(destination, "root-documented-nested-folder/home/docs/setup/locale/INSTALL-en.md");
        test::utilities::remove_from_filesystem(destination, "root-documented-nested-folder/home/docs/setup/locale/en/flag.png");
        result = core::oci::diff_to_target(result.value());
        REQUIRE(result.has_value());
        REQUIRE_FALSE(result->changes.empty());
        auto target = destination / fs::path("root-documented-nested-folder/home/docs/README.md");
        REQUIRE(result->changes.find(target.string()) != result->changes.end());
        auto entry = result->changes.at(target.string());
        REQUIRE(entry.type == core::oci::change_type::deleted);
        REQUIRE(fs::path(fs::path(destination) / fs::path("root-documented-nested-folder/home/docs/.wh.README.md")) == fs::path(entry.path));

        target = destination / fs::path("root-documented-nested-folder/home/docs/legal/LICENSE.txt");
        REQUIRE(result->changes.find(target.string()) != result->changes.end());
        entry = result->changes.at(target.string());
        REQUIRE(entry.type == core::oci::change_type::deleted);
        REQUIRE(fs::path(fs::path(destination) / fs::path("root-documented-nested-folder/home/docs/legal/.wh.LICENSE.txt")) == fs::path(entry.path));

        target = destination / fs::path("root-documented-nested-folder/home/docs/setup/INSTALL.md");
        REQUIRE(result->changes.find(target.string()) != result->changes.end());
        entry = result->changes.at(target.string());
        REQUIRE(entry.type == core::oci::change_type::deleted);
        REQUIRE(fs::path(fs::path(destination) / fs::path("root-documented-nested-folder/home/docs/setup/.wh.INSTALL.md")) == fs::path(entry.path));

        target = destination / fs::path("root-documented-nested-folder/home/docs/setup/locale/INSTALL-en.md");
        REQUIRE(result->changes.find(target.string()) != result->changes.end());
        entry = result->changes.at(target.string());
        REQUIRE(entry.type == core::oci::change_type::deleted);
        REQUIRE(fs::path(fs::path(destination) / fs::path("root-documented-nested-folder/home/docs/setup/locale/.wh.INSTALL-en.md")) == fs::path(entry.path));

        target = destination / fs::path("root-documented-nested-folder/home/docs/setup/locale/en/flag.png");
        REQUIRE(result->changes.find(target.string()) != result->changes.end());
        entry = result->changes.at(target.string());
        REQUIRE(entry.type == core::oci::change_type::deleted);
        REQUIRE(fs::path(fs::path(destination) / fs::path("root-documented-nested-folder/home/docs/setup/locale/en/.wh.flag.png")) == fs::path(entry.path));
    }

    SECTION("package changes to layer")
    {
        auto root = test::utilities::from_archives("root.tar.gz");
        fs::path destination(fs::current_path() / fs::path("layer-0"));
        test::utilities::clean(destination);
        fs::path layer_file(fs::current_path() / fs::path("layer.tar.gz"));
        auto result = core::oci::initialize(root, destination, layer_file)
                          .and_then(core::oci::copy_root)
                          .and_then(core::oci::snapshot_target);
        test::utilities::create_to_filesystem(destination, "root/home/docs/README.md");
        test::utilities::create_to_filesystem(destination, "root/home/docs/legal/LICENSE.txt");
        test::utilities::create_to_filesystem(destination, "root/home/docs/setup/INSTALL.md");
        test::utilities::create_to_filesystem(destination, "root/home/docs/setup/locale/INSTALL-en.md");
        test::utilities::create_to_filesystem(destination, "root/home/docs/setup/locale/en/flag.png");
        auto report = core::oci::diff_to_target(result.value())
                          .and_then(core::oci::package_layer);
        REQUIRE(report.has_value());

        REQUIRE_FALSE(report->hash_sum.empty());
        REQUIRE(fs::exists(report->path));
        REQUIRE(fs::file_size(report->path) > 0);
    }

    SECTION("generate an oci compliant image configuration")
    {
        core::oci::configuration_order order{};
        order.destination = fs::current_path();
        order.arch = "amd64";
        order.os = "freebsd";
        order.version = "13.4";
        order.variant = "v3";
        order.entrypoint = std::vector<std::string>{"jpod-entrypoint.sh"};
        order.command = std::vector<std::string>{"nginx", "-g", "daemon off"};
        order.env_vars = {{"PATH", "/usr/local/bin:/usr/bin:/bin"}};
        order.layers = {
            {"sha256:d4fc045c9e3a848011de66f34b81f052d4f2c15a17bb196d637e526349601820", fs::current_path()},
            {"sha256:d58589892f802d8854a1579bbe4926bc4256d9fa70596393ff6c2085c3d97978", fs::current_path()},
            {"sha256:1938f2eecb3765a1782ca1e30dcec1bf73b610b30be868b1b38e4b66ca3532d0", fs::current_path()},
            {"sha256:ac0684fc92d457ac1d713b19566f489ed65b3729e4d0dc389d9ceaa07e00360b", fs::current_path()}};
        order.ports = {{9000, "tcp"}};
        order.volumes = {{"/var/lib/postgresql/data", ""}};
        order.labels = {
            {"org.opencontainers.image.created", "2024-02-08T19:52:58Z"},
            {"org.opencontainers.image.version", "1.0.0"},
            {"org.opencontainers.image.title", "orca"}};
        order.work_dir = "/usr/share/nginx/html";
        auto report = core::oci::generate_configuration(order);
        REQUIRE(report.has_value());
        REQUIRE(fs::exists(report.value()));
        REQUIRE(fs::is_regular_file(report.value()));
        // check to make sure the json file
        std::fstream file(report.value());
        auto json = json::parse(file);
        REQUIRE(json.contains("os"));
        REQUIRE(json["os"].template get<std::string>() == "freebsd");
        REQUIRE(json["os.version"].template get<std::string>() == "13.4");
        REQUIRE(json.contains("architecture"));
        REQUIRE(json["architecture"].template get<std::string>() == "amd64");
        REQUIRE(json["variant"].template get<std::string>() == "v3");
        REQUIRE(json.contains("created"));
        REQUIRE(json.contains("rootfs"));
        REQUIRE(json["rootfs"].contains("type"));
        REQUIRE(json["rootfs"]["type"].template get<std::string>() == "layers");
        REQUIRE(json["rootfs"].contains("diff_ids"));
        REQUIRE(json["rootfs"]["diff_ids"].is_array());
        REQUIRE(json["rootfs"]["diff_ids"].size() == 4);
        REQUIRE(json.contains("config"));
        auto configuration = json["config"];
        auto ports = configuration["ExposedPorts"];
        auto labels = configuration["Labels"];
        auto env_vars = configuration["Env"];
        auto entrypoint = configuration["Entrypoint"];
        auto workdir = configuration["WorkDir"];
        auto volumes = configuration["Volumes"];
        auto command = configuration["Cmd"];
        auto stop_signal = configuration["StopSignal"];
        REQUIRE(ports.contains("9000/tcp"));
        REQUIRE(labels.contains("org.opencontainers.image.created"));
        REQUIRE(labels.contains("org.opencontainers.image.version"));
        REQUIRE(labels.contains("org.opencontainers.image.title"));
        REQUIRE(env_vars.at(0).template get<std::string>() == "PATH=/usr/local/bin:/usr/bin:/bin");
        REQUIRE(volumes.contains("/var/lib/postgresql/data"));
        REQUIRE(workdir.template get<std::string>() == "/usr/share/nginx/html");
        REQUIRE(entrypoint.at(0).template get<std::string>() == "jpod-entrypoint.sh");
        REQUIRE(command.at(0).template get<std::string>() == "nginx");
        REQUIRE(command.at(1).template get<std::string>() == "-g");
        REQUIRE(command.at(2).template get<std::string>() == "daemon off");
    }

    SECTION("generate an oci image compliant manifest") {
        
    }
}
