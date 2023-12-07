#include <images/work_dir_instruction.h>
#include <range/v3/view/split.hpp>
#include <range/v3/range/conversion.hpp>
#include <spdlog/spdlog.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace images
{

    WorkDirInstruction::WorkDirInstruction(
        const std::string &id,
        const std::string &source,
        const std::string &base_directory,
        std::string &current_directory,
        InstructionListener &listener) : id(id),
                                         source(source),
                                         base_directory(base_directory),
                                         current_directory(current_directory),
                                         listener(listener),
                                         logger(spdlog::get("jpod"))
    {
    }

    std::error_code WorkDirInstruction::parse()
    {
        std::error_code err;
        if (!fs::exists(base_directory, err) || source.empty())
        {
            // scream
            return err ? err : std::make_error_code(std::errc::io_error);
        }
        return err;
    }
    void WorkDirInstruction::execute()
    {
        // going to split the string from this point
        auto parts = source | ranges::view::split(' ') | ranges::to<std::vector<std::string>>();
        if (parts.size() != 1)
        {
            listener.on_instruction_runner_completion(this->id, std::make_error_code(std::errc::io_error));
        }
        else
        {
            auto joined_path = fs::path(base_directory) / fs::path(parts.at(0));
            if (!fs::is_directory(joined_path))
            {
                listener.on_instruction_runner_completion(this->id, std::make_error_code(std::errc::not_a_directory));
            }
            else
            {
                current_directory = joined_path.string();
                listener.on_instruction_runner_completion(this->id, std::make_error_code(std::errc::not_a_directory));
            }
        }
    }
    WorkDirInstruction::~WorkDirInstruction() {}
}