#include <images/copy_instruction.h>
#include <images/errors.h>
#include <filesystem>
#include <vector>
#include <range/v3/view/split.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/replace.hpp>
#include <range/v3/range/conversion.hpp>
#include <spdlog/spdlog.h>

namespace fs = std::filesystem;
namespace images
{
    CopyInstruction::CopyInstruction(const std::string &id,
                                     const std::string &order,
                                     const std::string &local_folder,
                                     const std::string &destination_folder,
                                     InstructionListener &listener) : id(id),
                                                                      order(order),
                                                                      local_folder(local_folder),
                                                                      destination_folder(destination_folder),
                                                                      listener(listener),
                                                                      logger(spdlog::get("jpod"))
    {
    }

    std::error_code CopyInstruction::parse()
    {
        std::error_code err{};
        // split string by space to figure out what the source is and the destination
        if (local_folder.empty() || !fs::is_directory(fs::path(local_folder)))
        {
            logger->error("origin is invalid");
            return make_error_code(error_code::invalid_origin);
        }
        if (destination_folder.empty() || !fs::is_directory(fs::path(destination_folder)))
        {
            logger->error("destination is invalid");
            return make_error_code(error_code::invalid_destination);
        }
        auto parts = order | ranges::view::split(' ') | ranges::to<std::vector<std::string>>();
        if (parts.size() < 2)
        {
            return make_error_code(error_code::invalid_copy_instruction);
        }
        // handle from parts
        err = parts.size() == 3 ? setup_stage_copy_origin(parts) : setup_local_copy_origin(parts);

        if (auto path = sanitize_route(destination_folder, parts.size() == 3 ? parts.at(2) : parts.at(1)); !fs::exists(path, err))
        {
            return err ? err : make_error_code(error_code::invalid_destination);
        }
        else
        {
            this->to = path;
        }
        listener.on_instruction_runner_initialized(this->id);
        return err;
    }
    std::string CopyInstruction::sanitize_route(const std::string &path, const std::string &target)
    {
        if (target.back() == '/')
        {
            if (auto pos = target.find("./"); pos != std::string::npos)
            {
                return fs::path(fs::path(path) / fs::path(std::string(target).erase(pos, 2))).string();
            }
            return fs::path(fs::path(path) / fs::path(target.substr(0, target.find_last_of("/")))).string();
        }

        return fs::path(fs::path(path) / fs::path(target)).string();
    }
    std::error_code CopyInstruction::setup_local_copy_origin(const std::vector<std::string> &order)
    {
        std::error_code err;
        if (auto path = sanitize_route(local_folder, order.at(0)); !fs::exists(path, err))
        {
            return err ? err : make_error_code(error_code::invalid_origin);
        }
        else
        {
            this->from = path;
            return err;
        }
    }
    std::error_code CopyInstruction::setup_stage_copy_origin(const std::vector<std::string> &order)
    {
        std::error_code err;
        if (auto position = order.at(0).find("--from="); position != std::string::npos)
        {
            // then append the part of the string to local folder path
            auto image_folder_path = fs::path(local_folder) / fs::path(order.at(0).substr(position + 1));
            if (auto path = sanitize_route(local_folder, order.at(0)); !fs::exists(path, err))
            {
                return err ? err : make_error_code(error_code::invalid_origin);
            }
            else
            {
                this->from = path;

            }
        }
        return err;
    }
    void CopyInstruction::execute()
    {
        std::error_code err;
        const auto options = fs::copy_options::overwrite_existing | fs::copy_options::recursive;
        fs::copy(from, to, options, err);
        if (err)
        {
            logger->error("copy failed with err: {} from: {} to: {}", err.message(), from, to);
        }
        listener.on_instruction_runner_completion(this->id, err);
    }
    CopyInstruction::~CopyInstruction()
    {
    }
}