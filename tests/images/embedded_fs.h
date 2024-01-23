#ifndef __JPOD_TESTING_IMAGES_EMBEDDED_FS__
#define __JPOD_TESTING_IMAGES_EMBEDDED_FS__

#include <cmrc/cmrc.hpp>

CMRC_DECLARE(archives);

namespace domain::images::instructions
{
    inline fs::path from_archives(const std::string &name)
    {
        fs::path target = fs::temp_directory_path() / fs::path(name);
        fs::create_directories(target.parent_path());
        if (!fs::exists(target))
        {
            fs::create_directories(target.parent_path());
            auto archive_fs = cmrc::archives::get_filesystem();
            auto file = archive_fs.open(name);
            std::vector<char> content(file.begin(), file.end());
            std::ofstream ofs(target, std::ios::out | std::ios::binary);
            ofs.write(content.data(), content.size());
            ofs.close();
        }
        return target;
    }

}

#endif // __JPOD_TESTING_IMAGES_EMBEDDED_FS__