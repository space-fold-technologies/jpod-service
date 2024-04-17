#ifndef __JPOD_TESTING_IMAGES_UTILITIES__
#define __JPOD_TESTING_IMAGES_UTILITIES__

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <optional>
#include <fmt/format.h>
#include <domain/images/payload.h>
#include <domain/images/http/contracts.h>
#include <domain/images/http/request.h>
#include <domain/images/mappings.h>
#include <domain/images/instructions/errors.h>
#include <domain/images/instructions/directory_resolver.h>

namespace fs = std::filesystem;

namespace domain::images::instructions
{
    using destination = std::shared_ptr<domain::images::http::download_destination>;
    inline std::string create_folder(const std::string &name)
    {
        auto current = fs::current_path();
        fs::path target = current / fs::path(name);
        if (!fs::exists(target))
        {
            if (!fs::create_directories(target))
            {
                return "";
            }
        }
        return target.string();
    }

    inline std::string create_sub_folder(const std::string &parent, const std::string &name)
    {
        fs::path target = fs::path(parent) / fs::path(name);
        if (!fs::exists(target))
        {
            if (!fs::create_directories(target))
            {
                return "";
            }
        }
        return target.string();
    }

    inline void create_file_in_folder(const std::string &folder, const std::string &name)
    {
        fs::path target = fs::path(folder) / fs::path(name);
        if (!fs::exists(target))
        {
            fs::create_directories(target.parent_path());
            std::ofstream ofs(target);
            ofs << "This is some text in the new file\n";
            ofs.close();
        }
    }

    inline void remove_folder(const std::string &name)
    {
        if (fs::is_directory(name))
        {
            fs::remove_all(name);
        }
    }
    inline auto no_image_found = [](const domain::images::http::request &req, domain::images::http::response_callback callback)
    {
        callback(make_error_code(error_code::no_matching_image_found), domain::images::http::response{});
    };
    inline auto no_registry_access = [](const domain::images::http::request &req, domain::images::http::response_callback callback)
    {
        callback(make_error_code(error_code::no_registry_access), domain::images::http::response{});
    };
    inline auto access_permitted = [](const domain::images::http::request &req, domain::images::http::response_callback callback) { // have to come up with the content to pack as binary in the response
        domain::images::image_meta meta{};
        meta.host = "zepkun";
        meta.identifier = "71dbec89-cad4-4f60-a73f-9be9a7ba6aca";
        meta.name = "mega-surf";
        meta.os = "linux";
        meta.tag = "latest";
        meta.variant = "alpine";
        meta.version = "1.2.0";
        meta.size = std::size_t(1024);
        meta.mount_points.push_back(domain::images::mount_point_details{"linprocfs", "/dev/shm", "mode=263", 0});
        domain::images::http::response resp{};
        msgpack::sbuffer buffer;
        msgpack::pack(buffer, meta);
        resp.data = std::vector<uint8_t>(buffer.size());
        std::memcpy(resp.data.data(), buffer.data(), buffer.size());
        resp.status_code = "200";
        resp.headers.emplace("Content-Length", fmt::format("{}", resp.data.size()));
        resp.headers.emplace("Content-Type", "application/x-msgpack");
        callback({}, resp);
    };

    inline auto download_invoked = [](const std::string &path, const std::map<std::string, std::string> &headers, destination sink, domain::images::http::report_callback callback)
    {
        domain::images::http::download_status status{};
        status.complete = true;
        status.total = 100;
        status.current = 100;
        status.unit = "bytes";
        callback({}, status);
    };
    inline auto extraction_invoked = [](const std::string &identifier, const std::string &image_identifier, domain::images::instructions::extraction_callback callback)
    {
        domain::images::progress_frame frame{};
        frame.percentage = 100;
        callback({}, frame);
    };
    inline auto fox_soft_registry = std::optional<domain::images::registry_access_details>{domain::images::registry_access_details{"https://registry.fox-soft.pods.com", "xfrnzzfhere%*#)@"}};

    inline auto wikin_registry = std::optional<domain::images::registry_access_details>{domain::images::registry_access_details{"https://registry.wikin.pods.com", "xfrnzzfhere%*#)@"}};

    inline std::vector<domain::images::mount_point> mount_points()
    {
        std::vector<domain::images::mount_point> pts{};
        pts.push_back(domain::images::mount_point{"devfs", "dev", "rw", 0});
        pts.push_back(domain::images::mount_point{"linprocfs", "proc", "rw", 0});
        pts.push_back(domain::images::mount_point{"linsys", "sys", "rw", 0});
        pts.push_back(domain::images::mount_point{"tmpfs", "dev/shm", "rw,mode=1777", 0});
        return pts;
    }
    inline std::optional<domain::images::image_details> dummy_image_details()
    {
        domain::images::image_details details{};
        details.identifier = "71dbec89-cad4-4f60-a73f-9be9a7ba6aca";
        details.name = "mega-surf";
        details.os = "linux";
        details.tag = "latest";
        details.variant = "alpine";
        details.version = "1.2.0";
        details.size = std::size_t(1024);
        details.labels.emplace("AUTHOR", "lostcause@doomed.com");
        details.env_vars.emplace("JAVA_HOME", "/opt/java");
        details.mount_points.push_back(domain::images::mount_point{"linprocfs", "/dev/shm", "mode=1777", 0});
        return details;
    }
}

#endif //__JPOD_TESTING_IMAGES_UTILITIES__