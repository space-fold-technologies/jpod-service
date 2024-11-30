#include <domain/images/helpers.h>
#include <domain/images/instructions/errors.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/registration_instruction.h>
#include <domain/images/mappings.h>
#include <domain/images/repository.h>
#include <core/oci/manifest_composer.h>
#include <range/v3/view/split.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/range/conversion.hpp>
#include <spdlog/spdlog.h>
#include <sys/utsname.h>

namespace domain::images::instructions {
registration_instruction::registration_instruction(
  image_properties properties,
  image_repository &repository,
  const fs::path& image_folder,
  const std::vector<core::oci::layer_details> layers,
  instruction_listener &listener)
  : instruction("REGISTRATION", listener), 
    properties(std::move(properties)), 
    repository(repository),
    image_folder(image_folder), 
    layers(layers), 
    logger(spdlog::get("jpod"))
{}

void registration_instruction::execute() {
  logger->info("STAGE ID: {}", properties.identifier);
  auto image_path = image_folder / fs::path(properties.identifier);
  /*
   - iterate over all the generated layers
   - put up the information into the configuration files
   - create the config and manifest files along side the layers
   */

  /*
   NB:
   - Need to gather the version of FreeBSD, architecture, variant if any as well as the last work-dir
  */
  listener.on_instruction_initialized(properties.identifier, name);
  core::oci::configuration_order configuration;
  configuration.labels = properties.labels;
  configuration.entrypoint = properties.entry_point | ranges::views::split(' ') | ranges::to<std::vector<std::string>>();
  configuration.command = properties.command | ranges::views::split(' ') | ranges::to<std::vector<std::string>>();
  configuration.env_vars = properties.env_vars;
  configuration.destination = image_path;
  
  utsname machine_details;
  /*
    char *sysname;
    The name of the implementation of the operating system.
    char *nodename;
    The node name of this particular machine. The node name is set by the SYSNAME sysparm (specified at IPL), and usually differentiates machines running at a single location.
    char *release;
    The current release level of the implementation.
    char *version;
    The current version level of the release.
    char *machine;
    The name of the hardware type the system is running on.
  */
  if (uname(&machine_details) != 0)
  {
    listener.on_instruction_complete(properties.identifier, std::error_code(errno, std::system_category()));
    return;
  }
  logger->info("NODE NAME: {}", machine_details.nodename);
  logger->info("RELEASE  : {}", machine_details.release);
  logger->info("VERSION  : {}", machine_details.version);
  logger->info("MACHINE  : {}", machine_details.machine);
  logger->info("SYSNAME  : {}", machine_details.sysname);
  configuration.arch = std::string(machine_details.machine);
  #if defined(__FreeBSD__)
  configuration.os = "freebsd";
  #elif defined(__sun__) && defined(__SVR4)
  configuration.os = "illumos";
  #endif
  core::oci::image_entry_details manifest;
  manifest.architecture = configuration.arch;
  manifest.variant = configuration.variant;
  
  manifest.annotations = {};
  manifest.destination = image_path;
  std::size_t image_size = 0;
  for(const auto& layer : layers)
  {
    configuration.layers.emplace(layer.hash_sum, layer.path);
    manifest.layers.push_back(layer.path);
    image_size += fs::file_size(layer.path); 
  }
  /*
   - Generate the configuration and follow it up with the manifest
   - Log the generated details of each 
  */
  
  if(auto cnf = core::oci::generate_configuration(configuration); !cnf)
  {
    listener.on_instruction_complete(properties.identifier, cnf.error());
  } else {
    manifest.config_path = cnf.value();
    if(auto mnft = core::oci::create_image_manifest(manifest); !mnft)
    {
      listener.on_instruction_complete(properties.identifier, mnft.error());
    } else {
      /*
      - save image to details in the local registry
      - trigger callback
      */
      image_details details{};
      details.identifier = properties.identifier;
      details.repository = properties.name;
      details.tag = properties.tag;
      details.tag_reference = mnft->hash;
      details.os = configuration.os;
      details.variant = configuration.variant;
      details.version = configuration.version;
      details.size = image_size;
      details.registry = std::string("127.0.0.1");
      auto error = repository.save_image_details(details);
      logger->info("image details set");
      listener.on_instruction_complete(properties.identifier, error);
    }
  }
}

registration_instruction::~registration_instruction() {}
}// namespace domain::images::instructions