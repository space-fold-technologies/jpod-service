#include <asio/io_context.hpp>
#include <core/archives/errors.h>
#include <core/archives/helper.h>
#include <domain/images/build_handler.h>
#include <domain/images/instructions/copy_instruction.h>
#include <domain/images/instructions/download_instruction.h>
#include <domain/images/instructions/registration_instruction.h>
#include <domain/images/instructions/run_instruction.h>
#include <domain/images/instructions/extraction_instruction.h>
#include <domain/images/instructions/work_dir_instruction.h>
#include <domain/images/repository.h>
#include <domain/images/payload.h>
#include <fmt/format.h>
#include <sole.hpp>
#include <spdlog/spdlog.h>

namespace domain::images {

build_handler::build_handler(core::connections::connection &connection,
  std::shared_ptr<image_repository> repository,
  oci_client_provider provider,
  const fs::path &image_folder,
  asio::io_context &context)
  : command_handler(connection), context(context), provider(provider), image_folder(image_folder),
    repository(std::move(repository)), logger(spdlog::get("jpod"))
{}

void build_handler::on_order_received(const std::vector<uint8_t> &payload)
{
  auto order = unpack_build_order(payload);
  setup_stages(order);
  run_stages();
}
void build_handler::run_stages()
{
  if (!stages.empty()) {
    auto stage_identifier = (*stages.begin()).first;
    run_stage(stage_identifier);
  } else {
    logger->info("finished building image");
    std::string message("image built");
    send_close(std::vector<uint8_t>(message.begin(), message.end()));
  }
}
void build_handler::setup_stages(const build_order &order)
{
  for (const auto &stage : order.stages) {
    auto stage_identifier = sole::uuid4().str();
    std::error_code error{};
    fs::path stage_folder;
    if (stage_folder = destination_path(stage_identifier, error); !error) {
      current_stage_work_directories.emplace(stage_identifier, stage_folder);
    }
    
    std::deque<task> instructions;
    std::string parent_image_order;
    int index = 0;
    for (const auto &[step, type] : stage.steps) {
      switch (type) {
      case step_type::from:
        add_download_instruction(stage_identifier, step);
        parent_image_order = step;
        break;

      case step_type::copy:
        add_copy_instruction(stage_identifier, step, order.current_directory);
        break;
      case step_type::extract:
        add_extraction_instruction(stage_identifier, step, order.current_directory);
        break;
      case step_type::work_dir:
        add_work_dir_instruction(stage_identifier, step);
        break;
      case step_type::run:
        add_run_instruction(stage_identifier, step);
        break;
      default:
        break;
      }
      auto last_stage = order.stages[order.stages.size() - 1];
      if(last_stage == stage)
      {
        auto layer_result = core::oci::initialize(
          stage_folder, 
          image_folder / fs::path(stage_identifier) / fs::path(fmt::format("layer-{}.tar.gz", index))
        );
        if(layer_result)
        {
         layer_states.push_back(layer_result);
        }
      }
    }
    if (parent_image_order.empty()) { parent_image_order = fmt::format("{}", stages.size() - 1); }
    resolve_stage_name(stage_identifier, index, parent_image_order);
    auto last_stage = order.stages[order.stages.size() - 1];
    if (last_stage == stage) 
    {
      // instructions.push_back(create_registration_instruction(stage_identifier, order, parent_image_order));
      last_stage_identifier = stage_identifier;
    }
    this->stages.try_emplace(std::move(stage_identifier), std::move(instructions));
    
    index++;
  }
}
void build_handler::resolve_stage_name(const std::string &identifer, int index, const std::string &order)
{
  // // check to see if there is an image alias
  if (auto position = order.find_last_of(" AS "); position != std::string::npos) {
    stage_names.try_emplace(order.substr(position + 1), identifer);
  } else {
    stage_names.try_emplace(fmt::format("{}", index), identifer);
  }
}
void build_handler::run_stage(const std::string &identifier) { stages[identifier].front()->execute(); }
void build_handler::add_download_instruction(const std::string &stage_identifier, const std::string &order)
{
  stages[stage_identifier].push_back(std::move(std::make_unique<download_instruction>(
    stage_identifier, order, provider, *repository.get(), *this, image_folder, *this)));
}
void build_handler::add_copy_instruction(const std::string &stage_identifier,
  const std::string &order,
  const std::string &local_folder)
{
  stages[stage_identifier].push_back(std::move(
    std::make_unique<copy_instruction>(stage_identifier, order, std::move(fs::path(local_folder)), *this, *this)));
}
void  build_handler::add_extraction_instruction(const std::string &stage_identifier, const std::string &order, const std::string & local_folder)
{
  stages[stage_identifier].push_back(std::move(
    std::make_unique<extraction_instruction>(stage_identifier, order, std::move(fs::path(local_folder)), *this, *this)));
}
void build_handler::add_work_dir_instruction(const std::string &stage_identifier, const std::string &order)
{
  stages[stage_identifier].push_back(std::move(std::make_unique<work_dir_instruction>(
    stage_identifier, order, current_stage_work_directories[stage_identifier], *this)));
}
void build_handler::add_run_instruction(const std::string &stage_identifier, const std::string &order)
{
  stages[stage_identifier].push_back(std::move(std::make_unique<run_instruction>(
    stage_identifier, order, context, current_stage_work_directories[stage_identifier], *this)));
}
// task build_handler::create_registration_instruction(const std::string &stage_identifier, const build_order &order,
// const std::string &parent_order)
// {
//     image_properties properties{};
//     properties.name = order.name;
//     properties.tag = order.tag;
//     properties.parent_image_order = parent_order;
//     return std::make_shared<registration_instruction>(stage_identifier, std::move(properties), *repository.get(),
//     *this);
// }
void build_handler::on_connection_closed(const std::error_code &error) 
{
}
void build_handler::on_instruction_initialized(std::string id, std::string name)
{
  std::error_code error;
  if (name == "FROM") {
    if (auto position = current_stage_work_directories.find(id); position != current_stage_work_directories.end()) {

      position->second = destination_path(id, error);
      if (error) { logger->info("error initializing work dir: {}", error.message()); }
    }
  } else if (current_stage_work_directories.find(id) == current_stage_work_directories.end()) {
    if (auto path = destination_path(id, error); !error) { current_stage_work_directories.try_emplace(id, path); }
  }
  if(last_stage_identifier == id)
  {
    layer_states
    .front()
    .and_then(core::oci::snapshot_target);
  }
}
void build_handler::on_instruction_data_received(std::string id, const std::vector<uint8_t> &content)
{
  send_frame(content);
}
void build_handler::on_instruction_complete(std::string id, std::error_code err)
{
  if (err) {
    send_error(err);
  } else {
    if(last_stage_identifier == id)
    {
      layer_states
      .front()
      .and_then(core::oci::diff_to_target)
      .and_then(core::oci::package_layer);

      layer_states
      .pop_front();
    }
    
    stages[id].pop_front();
    if (!stages[id].empty()) {
      run_stage(id);
    } else {
      stages.erase(stages.find(id));
      run_stages();
    }
  }
}
fs::path build_handler::stage_path(const std::string &label, std::error_code &error)
{
  // need to re-work this call, it's goofy
  std::string identifier;
  if (auto position = stage_names.find(label); position == stage_names.end()) {
    // fail with error
  } else {
    identifier = position->second;
  }

  if (temporary_folders.find(identifier) == temporary_folders.end()) {
    return create_temporary_folder(identifier, error);
  }
  return temporary_folders[identifier];
}
fs::path build_handler::destination_path(const std::string &identifier, std::error_code &error)
{
  if (temporary_folders.find(identifier) == temporary_folders.end()) {
    return create_temporary_folder(identifier, error);
  }
  return temporary_folders[identifier];
}
fs::path build_handler::generate_image_path(const std::string &identifier, std::error_code &error)
{
  // generate a folder in a pre-fixed path that has the ${identifier} as the final folder
  fs::path image_fs_archive = image_folder / fs::path(identifier) / fs::path("fs.tar.gz");
  if (!fs::create_directories(image_fs_archive.parent_path(), error)) {
    logger->error("build-handler : {}", error.message());
  }
  return image_fs_archive;
}
fs::path build_handler::create_temporary_folder(const std::string &identifier, std::error_code &error)
{
  fs::path output_folder = fs::temp_directory_path() / identifier;
  fs::create_directories(output_folder, error);
  if (!error) { temporary_folders.emplace(identifier, output_folder); }
  return output_folder;
}
std::error_code build_handler::extract_image(const std::string &identifier, const std::string &image_identifier)
{
  fs::path output_folder = temporary_folders[identifier];
  if (auto result_out = core::archives::initialize_writer(); !result_out) {
    return result_out.error();
  } else {
    progress_frame frame{};
    auto out = result_out.value();
    for (const auto &entry : fs::directory_iterator(image_folder / fs::path(image_identifier))) {
      if (auto result_in = core::archives::initialize_reader(entry); !result_in) {
        return result_out.error();
      } else {
        auto in = result_in.value();
        archive_entry *entry;
        while (archive_read_next_header(in.get(), &entry) == ARCHIVE_OK) {
          const char *entry_name = archive_entry_pathname(entry);
          fs::path full_path = output_folder / fs::path(std::string(entry_name));
          archive_entry_set_pathname(entry, full_path.generic_string().c_str());
          if (auto ec = archive_write_header(out.get(), entry); ec != ARCHIVE_OK) {
            logger->error("{}", archive_error_string(out.get()));
            return core::archives::make_compression_error_code(ec);
          } else if (archive_entry_size(entry) > 0) {
            if (auto error = core::archives::copy_entry(in, out); error) { return error; }
          }
        }
      }
    }
  }
  return {};
}
build_handler::~build_handler()
{
  current_stage_work_directories.clear();
  std::error_code error{};
  for(const auto&[_, identifier]:stage_names)
  {
   if (auto directory = destination_path(identifier, error); error)
   {
      logger->error("clean out error: {}", error.message());
   } 
   else if (auto removed_total = fs::remove_all(directory, error); error)
   {
      logger->error("clean out error: {}", error.message());
   } 
   else 
   {
      logger->info("clean out: {}", removed_total);
   }
  }
  stage_names.clear();
}
}// namespace domain::images