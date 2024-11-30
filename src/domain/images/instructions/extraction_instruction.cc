#include <core/archives/helper.h>
#include <domain/images/instructions/directory_resolver.h>
#include <domain/images/instructions/errors.h>
#include <domain/images/instructions/extraction_instruction.h>
#include <domain/images/instructions/instruction_listener.h>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/split.hpp>
#include <range/v3/view/transform.hpp>
#include <spdlog/spdlog.h>

namespace domain::images::instructions {
extraction_instruction::extraction_instruction(const std::string &identifier,
  const std::string &order,
  fs::path local_folder,
  directory_resolver &resolver,
  instruction_listener &listener)
  : instruction("EXTRACT", listener), identifier(identifier), order(order), local_folder(std::move(local_folder)),
    resolver(resolver), logger(spdlog::get("jpod"))
{}
void extraction_instruction::execute()
{
  auto parts = order | ranges::views::split(' ') | ranges::to<std::vector<std::string>>();
  if (parts.size() < 2) {
    listener.on_instruction_complete(identifier, make_error_code(error_code::invalid_extraction_instruction));
  } else if (auto origin = sanitize_route(local_folder, parts.at(0)); !origin || !fs::exists(origin.value())) {
    auto error = !origin ? origin.error() : make_error_code(error_code::invalid_origin);
    listener.on_instruction_complete(identifier, error);
  } else if (auto destination = setup_destination(parts.at(1)); !destination) {
    listener.on_instruction_complete(identifier, destination.error());
  } else if (!fs::is_regular_file(origin.value())) {
    listener.on_instruction_complete(identifier, make_error_code(error_code::archive_file_expected));
  } else if (auto reader = core::archives::initialize_reader(origin.value()); !reader) {
    listener.on_instruction_complete(identifier, reader.error());
  } else if (auto writer = core::archives::initialize_writer(); !writer) {
    listener.on_instruction_complete(identifier, writer.error());
  } else {
    listener.on_instruction_initialized(identifier, name);
    auto message = fmt::format("EXTRACTING: {} TO {}\n", parts.at(0), parts.at(1));
    listener.on_instruction_data_received(identifier, std::vector<uint8_t>(message.begin(), message.end()));
    if (auto error = core::archives::copy_to_destination(reader.value(), writer.value(), destination.value()); error) {
      listener.on_instruction_complete(identifier, error);
    } else {
      message = fmt::format("EXTRACTED: {} TO {}\n", parts.at(0), parts.at(1));
      listener.on_instruction_data_received(identifier, std::vector<uint8_t>(message.begin(), message.end()));
      listener.on_instruction_complete(identifier, {});
    }
  }
}

path_result extraction_instruction::sanitize_route(const std::string &path, const std::string &target)
{
  std::error_code error;
  if (target.back() == '/') {
    if (auto pos = target.find("./"); pos != std::string::npos) {
      return fs::path(fs::path(path) / fs::path(std::string(target).erase(pos, 2))).string();
    }
    auto joined_path = fs::path(fs::path(path) / fs::path(target.substr(0, target.find_last_of("/"))));

    if (auto sanitized_path = fs::weakly_canonical(joined_path, error); error) {
      return tl::make_unexpected(error);
    } else {
      return sanitized_path.make_preferred();
    }
  }

  auto joined_path = fs::path(fs::path(path) / fs::path(target));
  if (auto sanitized_path = fs::weakly_canonical(joined_path, error); error) {
    return tl::make_unexpected(error);
  } else {
    return sanitized_path.make_preferred();
  }
}
path_result extraction_instruction::setup_destination(const std::string &order)
{
  std::error_code error;
  if (auto destination_folder = resolver.destination_path(identifier, error); error) {
    return tl::make_unexpected(error);
  } else if (auto destination = sanitize_route(destination_folder, order); !destination) {
    return tl::make_unexpected(destination.error());
  } else if (!fs::exists(destination.value())) {
    return tl::make_unexpected(make_error_code(error_code::invalid_destination));
  } else {
    return destination;
  }
}
extraction_instruction::~extraction_instruction() {}
}// namespace domain::images::instructions