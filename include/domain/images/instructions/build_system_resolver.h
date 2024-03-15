#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_BUILD_SYSTEM_RESOLVER__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_BUILD_SYSTEM_RESOLVER__

#include <domain/images/instructions/directory_resolver.h>
#include <map>
#include <memory>

namespace spdlog
{
    class logger;
};
struct archive;
namespace domain::images::instructions
{
    const int BUFFER_SIZE = 10240;
    using archive_ptr = std::unique_ptr<archive, std::function<void(archive *)>>;
    class build_system_resolver : public directory_resolver
    {
    public:
        explicit build_system_resolver(
            std::string local_directory,
            const std::map<std::string, std::string> &stage_names);
        virtual ~build_system_resolver();
        fs::path local_folder() override;
        fs::path stage_path(const std::string &label, std::error_code &error) override;
        fs::path destination_path(const std::string &identifier, std::error_code &error) override;
        fs::path generate_image_path(const std::string &identifier, std::error_code &error) override;
        void extract_image(const std::string &identifier, const std::string &image_identifier, extraction_callback callback) override;

    private:
        archive_ptr initialize_reader(const fs::path &image_fs_archive, std::error_code &error);
        archive_ptr initialize_writer();
        std::error_code copy_entry(struct archive* in, struct archive* out);

    private:
        std::string local_directory;
        fs::path image_folder;
        fs::path temporary_folder;
        const std::map<std::string, std::string> &stage_names;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_BUILD_SYSTEM_RESOLVER__