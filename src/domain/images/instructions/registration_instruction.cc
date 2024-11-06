#include <domain/images/helpers.h>
#include <domain/images/instructions/errors.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/registration_instruction.h>
#include <domain/images/mappings.h>
#include <domain/images/repository.h>
#include <core/oci/layer_composer.h>
#include <core/oci/manifest_composer.h>
#include <spdlog/spdlog.h>
namespace domain::images::instructions {
registration_instruction::registration_instruction(const std::string &identifier,
  image_properties properties,
  image_repository &repository,
  const fs::path& image_folder,
  instruction_listener &listener)
  : instruction("REGISTRATION", listener), identifier(identifier), properties(properties), repository(repository),
    image_folder(image_folder), logger(spdlog::get("jpod"))
{}

void registration_instruction::execute() {

}

registration_instruction::~registration_instruction() {}
}// namespace domain::images::instructions