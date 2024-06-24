#include <core/oci/oci_client.h>
#include <core/oci/layer_download_task.h>
#include <core/oci/payloads.h>
#include <core/http/async_client.h>
#include <core/http/response.h>
#include <core/http/session.h>
#include <asio/ssl/error.hpp>
#include <asio/io_context.hpp>
#include <asio/deadline_timer.hpp>
#include <core/utilities/defer.h>
#include <nlohmann/json.hpp>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <fmt/format.h>
#include <fstream>
#include <spdlog/spdlog.h>
#include <range/v3/view/split.hpp>
#include <range/v3/range/conversion.hpp>

using namespace core::http;
using namespace core::utilities;
using namespace ranges;
using json = nlohmann::json;
namespace core::oci
{
    oci_client::oci_client(asio::io_context &context, session_provider provider) : context(context),
                                                                                   provider(provider),
                                                                                   client(std::make_unique<core::http::async_client>(provider)),
                                                                                   download_tasks{},
                                                                                   logger(spdlog::get("jpod"))
    {
    }
    void oci_client::authorize(const registry_credentials &credentials, authorization_callback callback)
    {
        auto path = credentials.authorization_server;
        std::map<std::string, std::string> headers;
        std::error_code error{};
        if (credentials.variant == authorization_variant::basic_auth)
        {
            std::string encoded_credentials;
            if (error = base64_encode(credentials.credentials, encoded_credentials); error)
            {
                callback(error);
            }
            else
            {
                headers.try_emplace("Authorization", fmt::format("Basic {}", encoded_credentials));
            }
        }
        headers.try_emplace("Accept", "application/json");
        headers.try_emplace("Connection", "Keep-Alive");
        client->get(
            path,
            headers,
            [this, credentials = std::move(credentials), cb = std::move(callback)](std::error_code error, const response &resp)
            {
                if (error || !resp.is_ok())
                {
                    cb(error);
                }
                else
                {
                    auto content = json::parse(resp.data);
                    auto session = std::make_shared<registry_session>();
                    if (content.contains("token"))
                    {
                        session->name = credentials.name;
                        session->token = content["token"].template get<std::string>();
                        session->expires_in = content["expires_in"].template get<int>();
                        session->issued_at = content["issued_at"].template get<std::string>();
                        sessions.try_emplace(credentials.registry, std::move(session));
                        cb({});
                    }
                }
            });
    }
    void oci_client::fetch_image(const image_fetch_order &order, image_progress_callback callback)
    {
        logger->info("fetching image: {}", order.repository);
        if (auto session_position = sessions.find(order.registry); session_position == sessions.end())
        {
            // fail here
            logger->info("no matching registry found");
            callback(std::make_error_code(std::errc::no_such_file_or_directory), {}, {});
        }
        else
        {
            auto session = session_position->second;
            manifest_request request{};
            request.name = session->name;
            request.registry = order.registry;
            request.architecture = order.architecture;
            request.operating_system = order.operating_system;
            request.repository = order.repository;
            request.token = session->token;
            request.tag = order.tag;
            request.destination = order.destination;
            fetch_manifest(request, std::move(callback));
        }
    }

    void oci_client::on_download_started(const std::string &image_digest, const std::string &layer_digest)
    {
        logger->info("started download IMAGE: {} LAYER: {}", image_digest, layer_digest);
        // move to the next task
        // start_sequence.pop_front();
        // if (!start_sequence.empty())
        // {
        //     auto target = start_sequence.front();
        //     auto tasks = download_tasks.at(target.first);
        //     auto task = tasks.at(target.second);
        //     task->start();
        // }
    }

    void oci_client::on_download_complete(const std::string &image_digest, const std::string &layer_digest)
    {
        logger->info("download complete");
        auto tasks = download_tasks.at(image_digest);
        if (auto position = tasks.find(layer_digest); position != tasks.end())
        {
            logger->warn("nuking old task");
            tasks.erase(position);
        }
        else
        {
            logger->error("old task not found");
        }
        start_sequence.pop_front();
        if (!start_sequence.empty())
        {
            auto target = start_sequence.front();
            auto pending_tasks = download_tasks.at(target.first);
            auto task = pending_tasks.at(target.second);
            task->start();
        }
        else if (tasks.empty())
        {
            download_tasks.erase(download_tasks.find(image_digest));
            // persist the configuration and manifest
            auto image = images.at(image_digest);
            std::ofstream manifest(image.destination / fs::path("manifest.json"), std::ios::out | std::ios::binary);
            manifest.write(reinterpret_cast<const char *>(image.manifest.data()), image.manifest.size());
            manifest.close();
            std::ofstream configuration(image.destination / fs::path("config.json"), std::ios::out | std::ios::binary);
            manifest.write(reinterpret_cast<const char *>(image.configuration.data()), image.configuration.size());
            manifest.close();
        }
    }
    void oci_client::on_download_update(const update_details &details)
    {
        if (auto position = images.find(details.image_digest); position != images.end())
        {
            auto &image = position->second;
            progress_update progress{};
            progress.image_digest = details.image_digest;
            progress.layer_digest = details.layer_digest;
            progress.total_layers = image.layers.size();
            progress.progress = (details.current * 100) / details.total;
            image.callback({}, progress, image.properties);
        }
    }
    void oci_client::on_download_failure(const std::string &image_digest, const std::string &layer_digest, const std::error_code &error)
    {
        if (auto position = images.find(image_digest); position != images.end())
        {
            auto &image = position->second;
            image.callback(error, {}, image.properties);
            // stop all existing tasks
            images.erase(position);
        }
        auto tasks = download_tasks.at(image_digest);
        if (auto position = tasks.find(layer_digest); position != tasks.end())
        {
            position->second->abort();
            tasks.erase(position);
        }
        if (tasks.empty())
        {
            download_tasks.erase(download_tasks.find(image_digest));
        }
    }

    void oci_client::fetch_manifest(const manifest_request &request, image_progress_callback callback)
    {
        std::string path = fmt::format("{0}/{1}/manifests/{2}", request.registry, request.repository, request.tag);
        std::map<std::string, std::string> headers;
        headers.try_emplace("Authorization", fmt::format("Bearer {}", request.token));
        headers.try_emplace("Connection", "Keep-Alive");
        if (request.headers.empty())
        {
            headers.try_emplace("Accept", fmt::format("application/vnd.{}.distribution.manifest.v2+json", request.name));
        }
        else
        {
            headers.insert(request.headers.begin(), request.headers.end());
        }
        client->get(
            path,
            headers,
            [this, req = std::move(request), cb = std::move(callback)](std::error_code error, const response &resp)
            {
                if (error || !resp.is_ok())
                {
                    if (error.category() == asio::error::get_ssl_category())
                    {
                        char buf[128];
                        ERR_error_string_n(ERR_get_error(), buf, sizeof(buf));
                        logger->error("OCI-ERR: {}", buf);
                    }
                    cb(error, {}, {});
                }
                else
                {
                    auto payload = json::parse(resp.data);
                    if (payload.contains("manifests"))
                    {
                        logger->info("got image manifest index");
                        logger->info("#### ARCH : {} #### OS : {}", req.architecture, req.operating_system);
                        for (const auto &entry : payload["manifests"])
                        {
                            if (entry.contains("platform"))
                            {
                                auto architecture = entry["platform"]["architecture"].template get<std::string>();
                                logger->info("ARCH: {}", architecture);
                                auto operating_system = entry["platform"]["os"].template get<std::string>();
                                logger->info("OPERATING SYSTEM: {}", operating_system);
                                if (entry["platform"].contains("variant"))
                                {
                                    architecture = fmt::format("{}:{}", architecture, entry["platform"]["variant"].template get<std::string>());
                                    logger->info("VARIATION OF ARCH: {}", architecture);
                                }
                                if (req.operating_system == operating_system && req.architecture == architecture)
                                {
                                    logger->info("GOT A MATCH");
                                    manifest_request request{};
                                    request.name = req.name;
                                    request.registry = req.registry;
                                    request.repository = req.repository;
                                    request.token = req.token;
                                    request.headers.try_emplace("Accept", entry["mediaType"].template get<std::string>());
                                    request.tag = entry["digest"].template get<std::string>();
                                    request.destination = req.destination;
                                    fetch_manifest(request, std::move(cb));
                                    return;
                                }
                            }
                        }
                        logger->error("not found");
                        // no matching image for your architecture
                        cb(std::make_error_code(std::errc::not_supported), {}, {});
                    }
                    else if (payload.contains("config"))
                    {
                        logger->info("got image manifest");
                        // this means you are at the final image of interest, fetch layers
                        auto digest = payload["config"]["digest"].template get<std::string>();
                        image_details details{};
                        details.tag = req.tag;
                        details.registry = req.registry;
                        details.repository = req.repository;

                        details.destination = req.destination;

                        for (const auto &layer_entry : payload["layers"])
                        {
                            details.layers.push_back(layer{
                                layer_entry["size"].template get<std::size_t>(),
                                layer_entry["mediaType"].template get<std::string>(),
                                layer_entry["digest"].template get<std::string>()});
                        }
                        for (const auto &[key, value] : payload["annotations"].items())
                        {
                            if (key == "org.opencontainers.image.base.name")
                            {
                                details.variant = value.template get<std::string>();
                            }
                            if (key == "org.opencontainers.image.version")
                            {
                                details.version = value.template get<std::string>();
                            }
                        }
                        details.manifest.assign(resp.data.begin(), resp.data.end());
                        images.try_emplace(digest, details);

                        configuration_request conf_req{};
                        conf_req.registry = fmt::format("{}", req.registry);
                        conf_req.repository = fmt::format("{}", req.repository);
                        conf_req.token = fmt::format("{}", req.token);
                        configuration_requests.try_emplace(digest, std::move(conf_req));
                        fetch_configuration(std::move(digest), std::move(cb));
                    }
                }
            });
    }
    void oci_client::fetch_configuration(std::string digest, image_progress_callback callback)
    {
        if (auto pos = configuration_requests.find(digest); pos == configuration_requests.end())
        {
            // a not found failure
            callback(std::make_error_code(std::errc::protocol_not_supported), {}, {});
        }
        else
        {
            auto request = pos->second;
            std::string path = fmt::format("{0}/{1}/blobs/{2}", request.registry, request.repository, digest);
            logger->info("PATH: {}", path);
            std::map<std::string, std::string> headers;
            headers.try_emplace("Authorization", fmt::format("Bearer {}", request.token));
            headers.try_emplace("Connection", "Keep-Alive");
            client->get(
                path,
                headers,
                [this, digest = std::move(digest), cb = std::move(callback)](std::error_code error, const response &resp)
                {
                    if (error || !resp.is_ok())
                    {
                        if (error.category() == asio::error::get_ssl_category())
                        {
                            auto err = error.message();
                            char buf[128];
                            ::ERR_error_string_n(error.value(), buf, sizeof(buf));
                            err += std::string(" (") +
                                   std::to_string(ERR_GET_LIB(error.value())) + "," +
                                   std::to_string(ERR_GET_REASON(error.value())) + ") " +
                                   buf;
                            logger->error("SSL ERR:\n{}", err);
                        }
                        else
                        {
                            cb(error, {}, {});
                        }
                    }
                    else
                    {
                        logger->info("got image configuration");
                        add_configuration(digest, resp.data, std::move(cb));
                        logger->info("parsed configuration");
                        if (digest.empty())
                        {
                            logger->info("crap");
                        }
                        logger->info("using IMAGE DIGEST:{} to fetch layers", digest);
                        fetch_layers(std::move(digest));
                    }
                });
        }
    }

    void oci_client::add_configuration(const std::string &digest, const std::vector<uint8_t> &data, image_progress_callback callback)
    {
        auto payload = json::parse(data);
        image_properties properties{};
        auto image = images.at(digest);

        for (const auto &layer : image.layers)
        {
            properties.size += layer.size;
        }
        properties.os = payload["os"].template get<std::string>();
        properties.tag = image.tag;
        properties.registry = image.registry;
        properties.repository = image.repository;
        properties.variant = image.variant;
        properties.version = image.version;

        for (auto &[key, value] : payload["config"]["ExposedPorts"].items())
        {
            auto parts = key | views::split('/') | to<std::vector<std::string>>();
            auto port = static_cast<uint16_t>(std::atoi(parts.at(0).c_str()));
            auto protocol = parts.size() > 1 ? parts.at(1) : "tcp";
            properties.exposed_ports.try_emplace(port, protocol);
        }
        for (const auto &env_var : payload["config"]["Env"])
        {
            std::string entry = env_var.template get<std::string>();
            auto parts = entry | views::split('=') | to<std::vector<std::string>>();
            properties.env_vars.try_emplace(parts.at(0), parts.at(1));
        }
        for (auto &[key, value] : payload["config"]["Labels"].items())
        {
            properties.labels.try_emplace(key, value);
        }
        for (auto &[key, value] : payload["config"]["Volumes"].items())
        {
            properties.volumes.push_back(key);
        }
        for (auto &parts : payload["config"]["Entrypoint"])
        {
            properties.entry_point.push_back(parts.template get<std::string>());
        }
        for (auto &parts : payload["config"]["Cmd"])
        {
            properties.command.push_back(parts.template get<std::string>());
        }

        if (payload["rootfs"].contains("diff_ids"))
        {
            for (auto layer_id : payload["rootfs"]["diff_ids"])
            {
                properties.layer_diffs.push_back(layer_id.template get<std::string>());
            }
        }
        logger->info("inside configuration packaging");
        if (auto position = images.find(digest); position != images.end())
        {
            logger->info("using digest to pack properties and callback");
            position->second.properties = std::move(properties);
            position->second.callback = std::move(callback);
            image.configuration.assign(data.begin(), data.end());
        }
    }

    void oci_client::fetch_layers(std::string digest)
    {
        logger->info("setting up download tasks");
        auto &details = images.at(digest);
        auto request = configuration_requests.at(digest);
        std::map<std::string, std::shared_ptr<layer_download_task>> tasks{};
        download_tasks.try_emplace(digest, std::move(tasks));
        uint16_t index = 0;
        //details.destination = details.destination / fs::path("sha256") / fs::path(digest.replace("sha256:", ""));
        std::error_code error;
        if (fs::create_directories(details.destination, error); error)
        {
            details.callback(error, {}, {});
            return;
        }
        for (const auto &layer : details.layers)
        {
            download_details order{};
            order.folder = details.destination;
            order.image_digest = digest;
            order.token = request.token;
            order.layer_digest = layer.digest;
            order.media_type = layer.media_type;
            order.provider = provider;
            order.layer_url = fmt::format("{0}/{1}/blobs/{2}", request.registry, request.repository, layer.digest);
            order.layer_size = layer.size;
            order.index = index;
            auto task = std::make_shared<layer_download_task>(context, order, *this);
            download_tasks.at(digest).try_emplace(layer.digest, task);
            start_sequence.push_back({digest, layer.digest});
            resolve_target target{};
            target.image_digest = digest;
            target.layer_digest = layer.digest;
            target.token = request.token;
            target.path = fmt::format("{0}/{1}/blobs/{2}", request.registry, request.repository, layer.digest);
            target.media_type = layer.media_type;
            resolve_queue.push_back(target);
            index++;
        }
        logger->info("Creating Image Target Directory: {}");

        logger->info("TOTAL ORDERS: {}", download_tasks.at(digest).size());
        resolve_layer();
    }

    void oci_client::resolve_layer()
    {

        if (!resolve_queue.empty())
        {
            auto target = resolve_queue.front();
            std::map<std::string, std::string> headers;
            headers.emplace("Referer", target.path);
            headers.emplace("Range", "bytes=0-");
            headers.emplace("Accept", target.media_type);
            headers.emplace("Authorization", fmt::format("Bearer {}", target.token));
            logger->info("layer endpoint:{}", target.path);
            client->head(
                target.path,
                headers,
                [this, target](const std::error_code &error, const core::http::response &response)
                {
                    if (error)
                    {
                        logger->error("resolution failed: {}", error.message());
                    }
                    else if (response.is_redirect())
                    {
                        download_tasks.at(target.image_digest).at(target.layer_digest)->update_target(response.location());
                    }
                    resolve_queue.pop_front();
                    resolve_layer();
                });
        }
        else
        {
            if (!start_sequence.empty())
            {
                auto target = start_sequence.front();
                auto current_tasks = download_tasks.at(target.first);
                auto task = current_tasks.at(target.second);
                task->start();
            }
        }
    }

    std::error_code oci_client::base64_encode(const std::string &input, std::string &output)
    {
        if (auto *encoder = BIO_new(BIO_f_base64()); encoder == nullptr)
        {
            return std::error_code(errno, std::system_category());
        }
        else
        {
            defer close_bio([encoder]
                            { BIO_free_all(encoder); });
            if (auto *sink = BIO_new(BIO_s_mem()); sink == nullptr)
            {
            }
            else
            {
                sink = BIO_push(encoder, sink);
                BIO_set_flags(sink, BIO_FLAGS_BASE64_NO_NL);
                BIO_write(sink, input.data(), static_cast<int>(input.length()));
                BIO_flush(sink);
                BUF_MEM *buffer_memory{};
                BIO_get_mem_ptr(encoder, &buffer_memory);
                output = std::string(buffer_memory->data, buffer_memory->length);
            }
        }
        return {};
    }
    oci_client::~oci_client()
    {
    }
}