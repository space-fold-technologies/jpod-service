#include <domain/images/instructions/cleanup_instruction.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/directory_resolver.h>
#include <domain/images/payload.h>
#include <filesystem>
#include <system_error>
#include <spdlog/spdlog.h>
#include <fmt/format.h>

namespace fs = std::filesystem;

namespace domain::images::instructions
{
    cleanup_instruction::cleanup_instruction(
        const std::string &identifier,
        std::vector<std::string> targets,
        directory_resolver &resolver,
        instruction_listener &listener) : instruction("CLEANUP", listener),
                                          identifier(identifier),
                                          targets(targets),
                                          resolver(resolver),
                                          logger(spdlog::get("jpod"))
    {
    }

    void cleanup_instruction::execute()
    {
        std::error_code error;
        progress_frame frame;
        frame.entry_name = identifier;
        frame.sub_entry_name = fmt::format("{}:{} STAGES", name, targets.size());
        int counter = 0;
        listener.on_instruction_initialized(identifier, name);
        for (const auto &stage_identifier : targets)
        {
            if (auto stage_directory = resolver.destination_path(stage_identifier, error); error)
            {
                listener.on_instruction_complete(identifier, error);
                return;
            }
            else if (auto removed_total = fs::remove_all(stage_directory, error); error)
            {
                listener.on_instruction_complete(identifier, error);
                return;
            }
            else
            {
                frame.percentage = (counter++) / targets.size();
                frame.feed = fmt::format("removed: {} ", removed_total);
                logger->info("nuked :{}", frame.feed);
                listener.on_instruction_data_received(identifier, pack_progress_frame(frame));
            }
        }
        listener.on_instruction_complete(identifier, {});
    }
    cleanup_instruction::~cleanup_instruction()
    {
    }
}