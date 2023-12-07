#ifndef __JPOD_TESTING_IMAGES_UTILITIES__
#define __JPOD_TESTING_IMAGES_UTILITIES__

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

namespace images
{
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
}

#endif //__JPOD_TESTING_IMAGES_UTILITIES__