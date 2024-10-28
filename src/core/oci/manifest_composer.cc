#include <core/oci/manifest_composer.h>
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
    manifest_result create_image_manifest(const image_entry_details &details)
    {
        json manifest;
        manifest["schemaVersion"] = 2;
        manifest["mediaType"] = "application/vnd.oci.image.manifest.v1+json";
        manifest["artifactType"] = "application/vnd.example+type";
        if (auto report = sha::compute_sha256_of_file(details.config_path); !report)
        {
            return tl::make_unexpected(report.error());
        }
        else
        {
            manifest["config"] = json::object();
            manifest["config"]["mediaType"] = "application/vnd.oci.image.config.v1+json";
            manifest["config"]["digest"] = report.value();
            manifest["config"]["size"] = fs::file_size(details.config_path);
        }

        manifest["layers"] = json::array();
        for (const auto &path : details.layers)
        {
            if (auto report = sha::compute_sha256_of_file(details.config_path); !report)
            {
                return tl::make_unexpected(report.error());
            }
            else
            {
                auto entry = json::object();
                entry.push_back({"mediaType", "application/vnd.oci.image.layer.v1.tar+gzip"});
                entry.push_back({"digest", report.value()});
                entry.push_back({"size", fs::file_size(details.config_path)});
                manifest["layers"].push_back(entry);
            }
        }
        if (!details.annotations.empty())
        {
            manifest["annotations"] = json::object();
            for (const auto &[key, value] : details.annotations)
            {
                manifest["annotations"].push_back({key, value});
            }
        }

        auto target_path = details.destination / fs::path("manifest.json");
        std::ofstream out(target_path);
        out << std::setw(4) << manifest << std::endl;
        out.close();
        if (auto report = sha::compute_sha256_of_file(target_path); !report)
        {
            return tl::make_unexpected(report.error());
        }
        else
        {
            auto file_size = fs::file_size(target_path);
            return manifest_report{report.value(), file_size, target_path};
        }
    }
}
