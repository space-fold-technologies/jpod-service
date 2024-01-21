#include <domain/images/instructions/copy_instruction.h>
#include <domain/images/instructions/directory_resolver.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/errors.h>
#include <range/v3/view/split.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/replace.hpp>
#include <range/v3/range/conversion.hpp>
#include <spdlog/spdlog.h>

namespace domain::images::instructions
{
    copy_instruction::copy_instruction(
        const std::string &identifier,
        const std::string &order,
        directory_resolver &resolver,
        instruction_listener &listener) : instruction("COPY", listener),
                                          identifier(identifier),
                                          order(order),
                                          resolver(resolver),
                                          logger(spdlog::get("jpod"))
    {
    }
    void copy_instruction::execute()
    {
        auto parts = order | ranges::view::split(' ') | ranges::to<std::vector<std::string>>();
        auto total = parts.size();
        if (total < 2)
        {
            listener.on_instruction_complete(identifier, make_error_code(error_code::invalid_copy_instruction));
            return;
        }
        else if (auto err = (total == 3 ? setup_stage_copy_origin(parts.at(0), parts.at(1)) : setup_local_copy_origin(parts.at(0))); err)
        {
            listener.on_instruction_complete(identifier, err);
            return;
        }
        else if (auto err = setup_destination(total == 3 ? parts.at(2) : parts.at(1)); err)
        {
            listener.on_instruction_complete(identifier, err);
            return;
        }
        else
        {
            listener.on_instruction_initialized(identifier, name);
            const auto options = fs::copy_options::overwrite_existing | fs::copy_options::recursive;
            fs::copy(origin, destination, options, err);
            listener.on_instruction_complete(identifier, err);
        }
    }
    fs::path copy_instruction::sanitize_route(const std::string &path, const std::string &target, std::error_code& err)
    {
        if (target.back() == '/')
        {
            if (auto pos = target.find("./"); pos != std::string::npos)
            {
                return fs::path(fs::path(path) / fs::path(std::string(target).erase(pos, 2))).string();
            }
            auto joined_path = fs::path(fs::path(path) / fs::path(target.substr(0, target.find_last_of("/"))));
    
            auto sanitized_path = fs::weakly_canonical(joined_path, err);
            return sanitized_path.make_preferred();
        }

        auto joined_path = fs::path(fs::path(path) / fs::path(target));
        auto sanitized_path = fs::weakly_canonical(joined_path, err);
        return sanitized_path.make_preferred();
    }
    std::error_code copy_instruction::setup_local_copy_origin(const std::string &order)
    {
        std::error_code err;
        auto local_folder = resolver.local_folder();
        if (origin = sanitize_route(local_folder, order, err); err || !fs::exists(origin))
        {
            return err ? err : make_error_code(error_code::invalid_origin);
        }
        return {};
    }
    std::error_code copy_instruction::setup_stage_copy_origin(const std::string &base, const std::string &order)
    {
        std::error_code err;
        if (auto position = base.find_first_of("--from=") == std::string::npos)
        {
            return make_error_code(error_code::invalid_origin);
        }
        else if (auto stage_path = resolver.stage_path(base.substr(position + BASE_IMAGE_NAME_POSITION), err); err)
        {
            return err;
        }
        else if (origin = sanitize_route(stage_path, order, err); err || !fs::exists(origin))
        {
            return make_error_code(error_code::invalid_origin);
        }
        return {};
    }
    std::error_code copy_instruction::setup_destination(const std::string &order)
    {
        std::error_code err;
        auto destination_folder = resolver.destination_path(identifier, err);
        if (err)
        {
            return err;
        }
        else if (destination = sanitize_route(destination_folder, order, err); err || !fs::exists(destination))
        {
            return make_error_code(error_code::invalid_destination);
        }
        return {};
    }
    copy_instruction::~copy_instruction()
    {
    }
}
