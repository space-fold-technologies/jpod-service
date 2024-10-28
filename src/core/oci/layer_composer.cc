#include <core/oci/layer_composer.h>
#include <core/archives/helper.h>
#include <core/archives/errors.h>
#include <core/utilities/defer.h>
#include <core/sha/utilities.h>
#include <core/oci/errors.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <fmt/chrono.h>
#include <fstream>
#include <chrono>

using json = nlohmann::json;

namespace core::oci
{
    layer_result initialize(const fs::path &root_path, const fs::path &target_folder, const fs::path &layer_archive)
    {
        std::error_code error;
        if (!fs::exists(root_path, error))
        {
            return !error ? tl::make_unexpected(error) : tl::make_unexpected(make_layer_error_code(layer_error_codes::no_file_or_directory_found));
        }
        // create the directories if not present
        if (!fs::exists(target_folder) && fs::create_directories(target_folder, error); error)
        {
            return tl::make_unexpected(error);
        }
        else if (!fs::exists(layer_archive.parent_path()) && fs::create_directories(layer_archive.parent_path(), error); error)
        {
            return tl::make_unexpected(error);
        }
        layer_state state{root_path, target_folder, layer_archive, {}};
        return state;
    }
    layer_result copy_root(layer_state state)
    {
        std::error_code error{};
        if (fs::is_regular_file(state.root_path))
        {
            // assuming this is [tar, tar.gz, tar.xz]
            if (auto reader = core::archives::initialize_reader(state.root_path); !reader)
            {
                return tl::make_unexpected(reader.error());
            }
            else if (auto writer = core::archives::initialize_writer(); !writer)
            {
                return tl::make_unexpected(writer.error());
            }
            else if (error = core::archives::copy_to_destination(reader.value(), writer.value(), state.target_path); error)
            {
                return tl::make_unexpected(error);
            }
            return state;
        }
        // clang-format off
        // copy over the content of the folder
        const auto options = fs::copy_options::recursive 
                           | fs::copy_options::copy_symlinks
                           | fs::copy_options::create_hard_links
                           | fs::copy_options::update_existing;
            // clang-format on                               
        
        if(fs::copy(state.root_path, state.target_path, options, error); error)
        {
            return tl::make_unexpected(error);
        }
        return state;
    }
    layer_result snapshot_target(layer_state state)
    {
        for(const auto& file : fs::recursive_directory_iterator(state.target_path))
        {
            if(!file.is_directory()) 
            {
                entry e{};
                e.path = file.path();
                e.stamp = fs::last_write_time(file);
                e.type = change_type::none;
                state.changes.try_emplace(file.path().string(), e);
            }
        }
        return state;
    }
    layer_result diff_to_target(layer_state state)
    {
        // look into figuring out c++ 17
        // check for deletions 
        for(auto& [path, details]: state.changes)
        {
            if(auto exists = fs::exists(path); !exists) 
            {
                details.type = change_type::deleted;
                details.path = details.path.parent_path() / fs::path(fmt::format(".wh.{}",details.path.filename().string()));
                std::ofstream ofs(details.path);
                if(ofs.is_open())
                {
                    ofs.close();
                }
            } else if (details.stamp != fs::last_write_time(fs::path(path))) 
            {
                details.type = change_type::modified;
            }
        }
        for(const auto& file : fs::recursive_directory_iterator(state.target_path))
        {
            if(!file.is_directory() && state.changes.find(file.path().string()) == state.changes.end())
            {
                entry e{};
                    e.path = file.path();
                    e.stamp = fs::last_write_time(file);
                    e.type = change_type::added;
                    state.changes.try_emplace(file.path().string(), e);
            }
        }
        return state;
    }
    layer_report package_layer(layer_state state)
    {
        if(auto writer = core::archives::archive_writer(state.layer_archive); !writer)
        {
            return tl::make_unexpected(writer.error());
        } else 
        {
         archive_entry *entry = archive_entry_new();
         core::utilities::defer clean_entry([&entry](){ archive_entry_free(entry);});
         layer_details details{};
         std::vector<std::string> hashes;
         for(const auto&[path, file_entry] : state.changes)
         {
            auto file_entry_path = fs::relative(file_entry.path, state.target_path);
            if(file_entry.type != change_type::none)
            {
                if(auto error =  core::archives::add_header(file_entry.path, entry, writer.value()); !error)
                {
                    return tl::make_unexpected(error);
                }
                // need to correct the base path for the entries
                if(file_entry.type != change_type::deleted) 
                {
                    archive_entry_set_pathname(entry, file_entry_path.c_str());
                    if(auto error = core::archives::add_entry(file_entry.path, writer.value()); error)
                    {
                        return tl::make_unexpected(error);
                    }
                } 
                
                archive_entry_clear(entry);
                if(auto report = note_change(file_entry.path, file_entry.type); !report)
                {
                    return tl::make_unexpected(report.error());
                } else if(!report.value().empty()) {
                    hashes.push_back(report.value());
                }
            }
         }
         if(auto result = sha::compute_sha256_sum(hashes); !result)
         {
            return tl::make_unexpected(result.error());
         } else 
         {
            details.hash_sum = result.value();
         }
         details.path = state.layer_archive;
         return details;
        }
    }
    hash_report note_change(const fs::path& file_path, change_type change)
    {  
       if(change == change_type::deleted)
       {
        return "";
       }
       if(auto report = sha::compute_sha256_of_file(file_path); !report)
       {
        return tl::make_unexpected(report.error());
       } else 
       {
        return report.value();
       }
    }
    configuration_report generate_configuration(const configuration_order &order)
    {
        json config;
        config["os"] = order.os;
        config["os.version"] = order.version;
        config["architecture"] = order.arch;
        config["author"] = order.author;
        if(!order.variant.empty())
        {
            config["variant"] = order.variant;
        }
        config["created"] = fmt::format("{:%Y-%m-%dT%H:%M:%SZ}", std::chrono::system_clock::now());
        config["rootfs"]["type"] = "layers";
        config["rootfs"]["diff_ids"] = json::array();
        for(const auto&[diff_id, _]: order.layers)
        {
            config["rootfs"]["diff_ids"].push_back(diff_id);
        }
        if(!order.labels.empty())
        {
            config["config"]["Labels"] = json::object();
            for(const auto&[key, value]: order.labels)
            {
                config["config"]["Labels"].push_back({key, value});
            }
            if(!config["config"]["Labels"].contains("org.opencontainers.image.created"))
            {
                config["config"]["Labels"].push_back({"org.opencontainers.image.created", config["created"].template get<std::string>()});
            }
        }
        config["config"]["Env"] = json::array();
        for(const auto&[key, value]: order.env_vars)
        {
            config["config"]["Env"].push_back(fmt::format("{0}={1}", key, value));
        }
        if(!order.entrypoint.empty())
        {
            config["config"]["Entrypoint"] = order.entrypoint;
        }
        if(!order.work_dir.empty()) {
            config["config"]["WorkDir"] = order.work_dir;
        }
        if(!order.command.empty())
        {
            config["config"]["Cmd"] = order.command;
        }
        if(!order.ports.empty())
        {
            config["config"]["ExposedPorts"] = json::object(); 
            for(const auto&[port, protocol]:order.ports)
            {
                config["config"]["ExposedPorts"].push_back({fmt::format("{0}/{1}", port, protocol), json::object()});
            }
        }
        if(!order.volumes.empty())
        {
            config["config"]["Volumes"] = json::object();
            for(const auto& [volume, _]: order.volumes)
            {
             config["config"]["Volumes"].push_back({volume, json::object()});   
            }
        }
        
        config["config"]["StopSignal"] = order.stop_signal;
        auto target_path = order.destination/ fs::path("config.json");
        std::ofstream out(target_path);
        out << std::setw(4) << config << std::endl;
        return target_path;
    }
}
