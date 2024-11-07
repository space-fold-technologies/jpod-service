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
registration_instruction::registration_instruction(const std::string &identifier,
  image_properties properties,
  image_repository &repository,
  const fs::path& image_folder,
  const std::vector<core::oci::layer_details> layers,
  instruction_listener &listener)
  : instruction("REGISTRATION", listener), identifier(identifier), properties(properties), repository(repository),
    image_folder(image_folder), layers(layers),logger(spdlog::get("jpod"))
{}

void registration_instruction::execute() {
  auto image_path = image_folder / fs::path(identifier);
  /*
   - iterate over all the generated layers
   - put up the information into the configuration files
   - create the config and manifest files along side the layers
   */

  /*
   NB:
   - Need to gather the version of FreeBSD, architecture, variant if any as well as the last work-dir
  */
  
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
    listener.on_instruction_complete(identifier, std::error_code(errno, std::system_category()));
    return;
  }
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
  for(const auto& layer : layers)
  {
    configuration.layers.emplace(layer.hash_sum, layer.path);
    manifest.layers.push_back(layer.path);
  }
  /*
   - Generate the configuration and follow it up with the manifest
   - Log the generated details of each 
  */
  if(auto cnf = core::oci::generate_configuration(configuration); !cnf)
  {
    listener.on_instruction_complete(identifier, cnf.error());
  } else {
    manifest.config_path = cnf.value();
    if(auto mnft = core::oci::create_image_manifest(manifest); !mnft)
    {
      listener.on_instruction_complete(identifier, mnft.error());
    } else {
      listener.on_instruction_complete(identifier, {});
    }
  }
}

registration_instruction::~registration_instruction() {}
}// namespace domain::images::instructions