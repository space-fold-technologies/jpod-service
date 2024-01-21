#include <domain/images/instructions/work_dir_instruction.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/errors.h>
#include <range/v3/view/split.hpp>
#include <range/v3/range/conversion.hpp>
#include <spdlog/spdlog.h>

namespace domain::images::instructions
{
    work_dir_instruction::work_dir_instruction(
        const std::string &identifier,
        const std::string &order,
        fs::path &current_directory,
        instruction_listener &listener) : instruction("WORKDIR", listener), identifier(identifier), order(order), current_directory(current_directory)
    {
    }
    void work_dir_instruction::execute()
    {
        auto parts = order | ranges::view::split(' ') | ranges::to<std::vector<std::string>>();
        if (parts.empty())
        {
            listener.on_instruction_complete(identifier, make_error_code(error_code::no_work_directory));
        }
        else if (!fs::exists(current_directory / fs::path(parts.at(0))) || !fs::is_directory(current_directory / fs::path(parts.at(0))))
        {
            listener.on_instruction_complete(identifier, make_error_code(error_code::no_work_directory));
        }
        else
        {
            std::error_code err;
            auto joined_path = current_directory / fs::path(order);
            auto sanitized_path = fs::weakly_canonical(joined_path, err);
            if (err)
            {
                listener.on_instruction_complete(identifier, err);
            }
            else
            {
                listener.on_instruction_initialized(identifier, name);
                current_directory = sanitized_path.make_preferred();
                listener.on_instruction_complete(identifier, err);
            }
        }
    }
    work_dir_instruction::~work_dir_instruction()
    {
    }
}
