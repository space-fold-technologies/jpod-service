#include <catch.hpp>
#include <core/containers/data_types.h>
#include <core/containers/manager.h>
#include <spdlog/fmt/fmt.h>

TEST_CASE("Container Builder test case") {
  const std::string ALPINE_LINUX_SNAPSHOT = "";
  const std::string FREEBSD_BASE_SNAPSHOT = "";

  SECTION("will construct jail given valid parameters and a correct freebsd-base snapshot, a valid freebsd jail", "[ROOT][FREE-BSD]") {
    using namespace containers;
    Composition composition;
    composition.hostname = "free";
    composition.ip_v4_address = "127.0.0.1";
    composition.ip_v6_address = "";
    composition.snapshot_path = FREEBSD_BASE_SNAPSHOT;

    ContainerManager manager;
    ContainerReport report = manager.create(BaseOS::FREE_BSD, composition);
    REQUIRE(report.isCreated() == true);
    // How to check if the actual jail exists is if a system call is made with the jail ID
    std::system(fmt::format("jail -r {}", report.getIdentifier()).c_str());
  }

  SECTION("will construct jail given valid parameters and a correct debian-base snapshot, a valid debian linux jail", "[ROOT][OTHER-BSD]") {
    using namespace containers;
    Composition composition;
    composition.hostname = "free";
    composition.ip_v4_address = "127.0.0.1";
    composition.ip_v6_address = "";
    composition.snapshot_path = ALPINE_LINUX_SNAPSHOT;

    ContainerManager manager;
    ContainerReport report = manager.create(BaseOS::LINUX, composition);
    REQUIRE(report.isCreated() == true);
    std::system(fmt::format("jail -r {}", report.getIdentifier()).c_str());
  }

  SECTION("will construct jail given valid parameters a base jail [environment] will be created", "[ROOT][ANY-BSD]") {
    using namespace containers;
    Composition composition;
    composition.hostname = "free";
    composition.ip_v4_address = "127.0.0.1";
    composition.ip_v6_address = "";

    ContainerManager manager;
    ContainerReport report = manager.create(BaseOS::FREE_BSD, composition);
    REQUIRE(report.isCreated() == true);
    std::system(fmt::format("jail -r {}", report.getIdentifier()).c_str());
  }

  SECTION("will fail to construct jail given no parameters", "[ROOT]") {
    using namespace containers;
    Composition composition;

    ContainerManager manager;
    ContainerReport report = manager.create(BaseOS::FREE_BSD, composition);
    REQUIRE(report.isCreated() == false);
  }

  SECTION("will fail to construct jail if a jail with an existing name exists", "[ROOT]") {
    using namespace containers;
    Composition composition;
    composition.hostname = "free";
    composition.ip_v4_address = "127.0.0.1";
    composition.ip_v6_address = "";
    composition.snapshot_path = ALPINE_LINUX_SNAPSHOT;

    ContainerManager manager;
    ContainerReport report = manager.create(BaseOS::LINUX, composition);
    Composition composition2;
    composition2.hostname = "free";
    composition2.ip_v4_address = "127.0.0.1";
    composition2.ip_v6_address = "";
    composition2.identifier = "linux";
    composition2.snapshot_path = ALPINE_LINUX_SNAPSHOT;
    ContainerReport report2 = manager.create(BaseOS::LINUX, composition2);
    REQUIRE_FALSE(report2.isCreated());
    std::system(fmt::format("jail -r {}", report.getIdentifier()).c_str());
  }

  SECTION("will be able to stop an existing container that is running", "[ROOT]") {
    using namespace containers;
    Composition composition;
    composition.hostname = "free";
    composition.ip_v4_address = "127.0.0.1";
    composition.ip_v6_address = "";
    composition.snapshot_path = ALPINE_LINUX_SNAPSHOT;

    ContainerManager manager;
    ContainerReport report = manager.create(BaseOS::LINUX, composition);
    report = manager.stop(report.getIdentifier());
    REQUIRE(report.isStopped());
  }

  SECTION("will be able to update an existing container given that it is running", "[ROOT]") {
    using namespace containers;
    Composition composition;
    composition.hostname = "free";
    composition.ip_v4_address = "127.0.0.1";
    composition.snapshot_path = ALPINE_LINUX_SNAPSHOT;
    ContainerManager manager;
    manager.create(BaseOS::LINUX, composition);
    ContainerReport report = manager.update(composition.identifier, {CommandType::ENV, "FOO=BAR"});
    REQUIRE(report.isUpdated() == true);
  }

  SECTION("will fail to update a container that is not running", "[ROOT]") {
    using namespace containers;
    Composition composition;
    composition.hostname = "free";
    composition.ip_v4_address = "127.0.0.1";
    composition.snapshot_path = ALPINE_LINUX_SNAPSHOT;
    ContainerManager manager;
    manager.create(BaseOS::LINUX, composition);
    ContainerReport report = manager.update(composition.identifier, {CommandType::ENV, "FOO=BAR"});
    REQUIRE(report.isUpdated() == true);
  }

  SECTION("will fail to update a container that does not exist", "[ROOT]") {
  }
}
