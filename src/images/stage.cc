#include <images/stage.h>
#include <images/data_types.h>
#include <images/copy_instruction.h>
#include <images/run_instruction.h>
#include <images/from_instruction.h>
#include <images/work_dir_instruction.h>
#include <images/image_repository.h>
#include <core/networks/http/client.h>
#include <asio/io_context.hpp>
#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/view/split.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/replace.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/remove_if.hpp>
#include <range/v3/view/subrange.hpp>

namespace images
{
    Stage::Stage(asio::io_context &context, const std::string &id, const std::string &local_directory, const StageDetails &details) : context(context), id(id), local_directory(local_directory), details(details)
    {
    }
    void Stage::initialize(
        std::shared_ptr<Client> client,
        std::shared_ptr<ImageRepository> repository,
        std::shared_ptr<FileSystemHandler> handler,
        std::optional<Stage> previous_stage)
    {
        // parse the instructions and create the required instruction operation required
        ranges::for_each(
            details.instructions,
            [this, &client, &repository, &handler, &previous_stage](const auto &instruction)
            {
                // because, we love lambdas <<sort of :|>>
                switch (instruction.second)
                {
                case StepType::FROM:
                    this->instructions.push_back(from_instruction(instruction.first, client, repository, handler));
                    break;
                case StepType::RUN:
                    this->instructions.push_back(run_instruction(instruction.first));
                    break;
                case StepType::COPY:
                    this->instructions.push_back(copy_instruction(previous_stage, instruction.first));
                    break;
                case StepType::WORKDIR:
                    this->instructions.push_back(work_dir_instruction(instruction.first));
                    break;
                }
            });
    }
    void Stage::run()
    {
        // post to start instruction run
        
    }
    void Stage::on_instruction_runner_initialized(std::string id)
    {
    }
    void Stage::on_instruction_runner_data_received(std::string id, const std::vector<uint8_t> &content)
    {
    }
    void Stage::on_instruction_runner_completion(std::string id, const std::error_code &err)
    {
    }
    std::string Stage::image_local_directory()
    {
        return local_directory;
    }
    std::shared_ptr<Instruction> Stage::from_instruction(
        const std::string &order,
        std::shared_ptr<Client> client,
        std::shared_ptr<ImageRepository> repository,
        std::shared_ptr<FileSystemHandler> handler)
    {
        return std::make_shared<FromInstruction>(this->id, current_image_identifier, order, client, repository, handler, *this);
    }
    std::shared_ptr<Instruction> Stage::run_instruction(const std::string &order)
    {
        return std::make_shared<RunInstruction>(context, this->id, order, current_work_directory, *this);
    }
    std::shared_ptr<Instruction> Stage::copy_instruction(std::optional<Stage> previous_stage, const std::string &order)
    {
        // check the order to see if it refers to a previous directory
        if (order.find("--from=") != std::string::npos)
        {
            return std::make_shared<CopyInstruction>(this->id, order, local_directory, current_work_directory, *this);
        }

        return std::make_shared<CopyInstruction>(this->id, order, previous_stage->image_local_directory(), current_work_directory, *this);
    }
    std::shared_ptr<Instruction> Stage::work_dir_instruction(const std::string &order)
    {
        return std::make_shared<WorkDirInstruction>(this->id, order, "/", current_work_directory, *this);
    }
    Stage::~Stage()
    {
    }
}