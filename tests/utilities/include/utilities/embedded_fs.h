#ifndef __TEST_UTILITIES_EMBEDDED_FS__
#define __TEST_UTILITIES_EMBEDDED_FS__

#include <cmrc/cmrc.hpp>
#include <string_view>
#include <filesystem>
#include <iostream>
#include <fstream>

CMRC_DECLARE(archives);

namespace fs = std::filesystem;

namespace test::utilities

{
    auto from_archives(const std::string &name) -> fs::path
    {
        fs::path target = fs::current_path() / fs::path(name);
        fs::create_directories(target.parent_path());
        if (!fs::exists(target))
        {
            fs::create_directories(target.parent_path());
            auto archive_fs = cmrc::archives::get_filesystem();
            auto file = archive_fs.open("archives/" + name);
            std::vector<char> content(file.begin(), file.end());
            std::ofstream ofs(target, std::ios::out | std::ios::binary);
            ofs.write(content.data(), content.size());
            ofs.close();
        }
        return target;
    }

    auto create_to_filesystem(const fs::path &destination, std::string_view name) -> void
    {

        fs::path target(destination / fs::path(name.size() > 1 && name.at(0) == '/' ? name.substr(1) : name));
        std::cout << target.string() << std::endl;
        if (!fs::exists(target.parent_path()))
        {
            fs::create_directories(target.parent_path());
        }
        else if (fs::exists(target))
        {
            fs::remove(target);
        }

        std::ofstream ofs(target);
        if (ofs.is_open())
        {
            ofs << "# THIS IS TEST CONTENT" << std::endl;
            ofs << "## THIS IS  SUB TEST CONTENT" << std::endl;
            ofs.close();
        }
        else
        {
            std::cerr << "failed to open file: " << strerror(errno) << std::endl;
        }
    }

    auto update_on_filesystem(const fs::path &destination, std::string_view name, std::string_view content) -> void
    {
        fs::path target(destination / fs::path(name.size() > 1 && name.at(0) == '/' ? name.substr(1) : name));
        std::ofstream ofs(target, std::ios::out | std::ios::binary);
        ofs << content << std::endl;
        ofs.close();
    }

    auto remove_from_filesystem(const fs::path &destination, std::string_view name) -> void
    {
        fs::path target(destination / fs::path(name.size() > 1 && name.at(0) == '/' ? name.substr(1) : name));
        if (fs::is_regular_file(target))
        {
            fs::remove(target);
        }
    }

    auto clean(const fs::path &destination) -> void
    {
        if (fs::exists(destination) && fs::is_directory(destination))
        {
            fs::remove_all(destination);
        }
    }
}

#endif //__TEST_UTILITIES_EMBEDDED_FS__