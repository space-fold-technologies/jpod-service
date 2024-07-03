
#include <domain/images/import_handler.h>
#include <domain/images/repository.h>
#include <domain/images/payload.h>
#include <core/archives/helper.h>
#include <core/archives/errors.h>
#include <range/v3/view/split.hpp>
#include <range/v3/range/conversion.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <fstream>

using json = nlohmann::json;
using namespace ranges;

namespace domain::images
{
    import_handler::import_handler(core::connections::connection &connection,
                                   fs::path &image_folder,
                                   std::shared_ptr<image_repository> repository) : command_handler(connection),
                                                                                   image_folder(image_folder),
                                                                                   repository(std::move(repository))
    {
    }
    void import_handler::on_order_received(const std::vector<uint8_t> &payload)
    {
        auto order = unpack_import_order(payload);
        auto local_file_path = fs::path(order.archive_path);
        auto result = initialize_state(std::move(repository), local_file_path, image_folder)
                          .and_then(read_image_details)
                          .and_then(extract_index)
                          .and_then(extract_configuration)
                          .and_then(extract_image_content)
                          .and_then(resolve_image_meta)
                          .and_then(persist_image_details);
        if (result)
        {
            send_success(result.value());
        }
        else
        {
            send_error(result.error());
        }
    }

    void import_handler::on_connection_closed(const std::error_code &error)
    {
    }
    import_result import_handler::initialize_state(std::shared_ptr<image_repository> repository, fs::path local_file_path, fs::path &image_folder)
    {

        if (auto result = core::archives::initialize_reader(local_file_path); !result)
        {
            return tl::make_unexpected(result.error());
        }
        else
        {
            import_state state{};
            state.image_archive = local_file_path;
            state.image_folder = image_folder;
            state.store = std::move(repository);
            state.logger = spdlog::get("jpod");
            return state;
        }
    }
    import_result import_handler::read_image_details(import_state state)
    {
        archive_entry *entry;
        auto logger = state.logger;
        uint32_t discovered_meta_details = 0;
        int ec = 0;
        if (auto result = core::archives::initialize_reader(state.image_archive); !result)
        {
            return tl::make_unexpected(result.error());
        }
        else
        {
            auto in = result.value();
            do
            {
                ec = archive_read_next_header(in.get(), &entry);
                if (ec != ARCHIVE_OK && discovered_meta_details < 2)
                {
                    return tl::make_unexpected(core::archives::make_compression_error_code(ec));
                }
                const mode_t type = archive_entry_filetype(entry);
                char const *current_entry_name = archive_entry_pathname(entry);
                std::size_t file_size = archive_entry_size(entry);
                if ((S_ISREG(type)) && std::strcmp(current_entry_name, MANIFEST.c_str()) == 0)
                {
                    logger->info("found image manifest of size: {}", file_size);
                    std::vector<uint8_t> buffer(file_size);
                    if (auto ec = archive_read_data(in.get(), &buffer[0], file_size); ec < 0)
                    {
                        return tl::make_unexpected(core::archives::make_compression_error_code(ec));
                    }
                    discovered_meta_details++;
                    state.manifest.assign(buffer.begin(), buffer.end());
                    logger->info("read in image manifest");
                }
                else if ((S_ISREG(type)) && std::strcmp(current_entry_name, INDEX.c_str()) == 0)
                {
                    logger->info("found image index of size: {}", file_size);
                    std::vector<uint8_t> buffer(file_size);
                    if (auto ec = archive_read_data(in.get(), &buffer[0], file_size); ec < 0)
                    {
                        return tl::make_unexpected(core::archives::make_compression_error_code(ec));
                    }
                    discovered_meta_details++;
                    state.index.assign(buffer.begin(), buffer.end());
                    logger->info("read in image index");
                }
            } while (ec == ARCHIVE_OK && discovered_meta_details < 2);
        }
        logger->info("finished picking up manifest and index information");
        return state;
    }
    import_result import_handler::extract_configuration(import_state state)
    {
        auto logger = state.logger;
        auto content = json::parse(std::string(state.manifest.begin(), state.manifest.end()));
        std::error_code error{};
        fs::path folder = state.image_folder / fs::path(state.identifier);
        if (fs::create_directories(folder, error); error)
        {
            return tl::make_unexpected(error);
        }
        auto manifest = content.is_array() ? content[0] : content;
        state.entries.try_emplace(manifest["Config"].template get<std::string>(), folder / fs::path("config.json"));
        int index = 0;
        std::map<std::string, std::string> media_types;
        if (manifest.contains("LayerSources"))
        {
            logger->info("has layer sources");
            for (const auto [key, value] : manifest["LayerSources"].items())
            {
                media_types.try_emplace(key, value["mediaType"].template get<std::string>());
                state.size += value["size"].template get<std::size_t>();
            }
            logger->info("got details from layer sources");
        }
        
        for (const auto &entry : manifest["Layers"])
        {
            auto path = entry.template get<std::string>();
            auto digest = fmt::format("sha256:{}", path.substr(path.find_last_of("/") + 1));
            auto media_type = media_types.at(digest);
            auto extension = media_type.find_last_of("tar+gzip") != std::string::npos ? "tar.gz" : "tar";
            auto file_path = fs::path(folder / fs::path(fmt::format("layer_{0}.{1}", index, extension)))
                                 .generic_string();
            state.entries.try_emplace(path, file_path);
            index++;
        }

        logger->info("extracted configuration details");
        return state;
    }
    import_result import_handler::extract_index(import_state state)
    {
        auto logger = state.logger;
        logger->info("extracting index details of size: {}", state.index.size());
        logger->info("INDEX CONTENT: \n {}", std::string(state.index.begin(), state.index.end()));
        auto index = json::parse(std::string(state.index.begin(), state.index.end()));
        logger->info("parsed index");
        if (!index.contains("manifests") || index["manifests"].size() == 0)
        {
            return tl::make_unexpected(std::make_error_code(std::errc::no_link));
        }
        state.identifier = index["manifests"].at(0)["digest"].template get<std::string>();
        logger->info("ID: {}", state.identifier);
        std::string header("sha256:");
        state.identifier.replace(0, header.size(), "");
        if (index["manifests"].at(0).contains("annotations"))
        {
            auto annotations = index["manifests"].at(0)["annotations"];
            auto name = annotations["io.containerd.image.name"].template get<std::string>();
            state.tag = annotations["org.opencontainers.image.ref.name"].template get<std::string>();
            state.repository = name.substr(0, name.find_last_of(":"));
        } else 
        {
            state.repository.append("unknown");
            state.registry.append("unknown");
        }
        logger->info("extracted index details");
        return state;
    }
    import_result import_handler::extract_image_content(import_state state)
    {
        int ec = 0;
        archive_entry *entry;
        if (auto result = core::archives::initialize_reader(state.image_archive); !result)
        {
            return tl::make_unexpected(result.error());
        }
        else
        {
            auto logger = state.logger;
            auto in = result.value();
            logger->info("extracting layers");
            while (archive_read_next_header(in.get(), &entry) == ARCHIVE_OK)
            {
                const mode_t type = archive_entry_filetype(entry);
                char const *current_entry_name = archive_entry_pathname(entry);
                if ((S_ISREG(type)))
                {
                    if (auto pos = state.entries.find(std::string(current_entry_name)); pos != state.entries.end())
                    {
                        logger->info("extracting: {} to {}", current_entry_name, pos->second.string());
                        // now we can write file to desired destination
                        std::ofstream stream(pos->second, std::ios::binary | std::ios::app);
                        std::size_t chunk_size = 0L;
                        std::vector<uint8_t> buffer(WRITE_BUFFER_SIZE);
                        do
                        {
                            if (chunk_size = archive_read_data(in.get(), buffer.data(), WRITE_BUFFER_SIZE); chunk_size > 0)
                            {
                                stream.write(reinterpret_cast<const char *>(buffer.data()), chunk_size);
                            }
                        } while (chunk_size > 0);
                    }
                }
            }
            logger->info("extracted all layers");
        }
        return state;
    }
    import_result import_handler::resolve_image_meta(import_state state)
    {
        auto logger = state.logger;
        logger->info("resolving image metadata");
        fs::path file = state.image_folder / fs::path(state.identifier) / fs::path("config.json");
        std::ifstream stream(file);
        if(!stream.is_open())
        {
            logger->warn("file is written and open");
        } 
        auto payload = json::parse(stream);
        logger->info("got payload");
        state.os = payload["os"].template get<std::string>();
        logger->info("OS: {}", state.os);
        auto configuration = payload["config"];
        if (configuration.contains("Labels") && configuration["Labels"].is_array())
        {
            for (auto &[key, value] : configuration["Labels"].items())
            {
                state.labels.try_emplace(key, value);
            }
        }
        if (configuration.contains("Volumes") && configuration["Volumes"].is_array())
        {
            for (auto &[key, _] : configuration["Volumes"].items())
            {
                state.volumes.push_back(key);
            }
        }
        if (configuration.contains("ExposedPorts"))
        {
            for (auto &[key, value] : configuration["ExposedPorts"].items())
            {
                auto parts = key | views::split('/') | to<std::vector<std::string>>();
                auto port = static_cast<uint16_t>(std::atoi(parts.at(0).c_str()));
                auto protocol = parts.size() > 1 ? parts.at(1) : "tcp";
                state.exposed_ports.try_emplace(port, protocol);
            }
        }

        if (configuration.contains("Env"))
        {
            for (const auto &env_var : configuration["Env"])
            {
                std::string entry = env_var.template get<std::string>();
                auto parts = entry | views::split('=') | to<std::vector<std::string>>();
                state.env_vars.try_emplace(parts.at(0), parts.at(1));
            }
        }
        if (configuration.contains("EntryPoint"))
        {
            for (auto &parts : configuration["Entrypoint"])
            {
                state.entry_point.push_back(parts.template get<std::string>());
            }
        }
        if (configuration.contains("Cmd"))
        {
            for (auto &parts : configuration["Cmd"])
            {
                state.command.push_back(parts.template get<std::string>());
            }
        }

        return state;
    }
    tl::expected<std::string, std::error_code> import_handler::persist_image_details(import_state state)
    {
        image_details details{};
        details.identifier = state.identifier;
        details.registry = state.registry;
        details.repository = state.repository;
        details.tag = state.tag;
        details.os = state.os;
        details.size = state.size;
        details.variant = state.variant;
        details.version = state.version;
        if (auto error = state.store->save_image_details(details); error)
        {
            return tl::make_unexpected(error);
        }
        return state.identifier;
    }
    import_handler::~import_handler()
    {
    }
}