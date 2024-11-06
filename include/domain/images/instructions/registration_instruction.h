#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_REGISTRATION_INSTRUCTION__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_REGISTRATION_INSTRUCTION__

#include <domain/images/instructions/instruction.h>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>

namespace spdlog {
class logger;
};
namespace domain::images {
class image_repository;
}

namespace fs = std::filesystem;
namespace domain::images::instructions {

class instruction_listener;
struct image_properties
{
  std::string name;
  std::string tag;
  std::string entry_point;
  std::map<std::string, std::string> labels;
  std::map<std::string, std::string> env_vars;
  std::vector<std::string> ports;
};
class registration_instruction : public instruction
{
public:
  explicit registration_instruction(const std::string &identifier,
    image_properties properties,
    image_repository &repository,
    const fs::path &image_folder,
    instruction_listener &listener);
  virtual ~registration_instruction();
  void execute() override;

private:
  const std::string &identifier;
  image_properties properties;
  image_repository &repository;
  const fs::path &image_folder;
  std::shared_ptr<spdlog::logger> logger;
};
}// namespace domain::images::instructions

#endif// __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_REGISTRATION_INSTRUCTION__