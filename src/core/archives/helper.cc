#include <core/archives/helper.h>
#include <core/archives/errors.h>

namespace core::archives
{
    constexpr std::size_t BUFFER_SIZE = 10240;
    tl::expected<archive_ptr, std::error_code> initialize_reader(fs::path archive_path)
    {
        archive_ptr arch = {
            archive_read_new(),
            [](archive *instance) -> void
            {
                archive_read_close(instance);
                archive_read_free(instance);
            }};
        archive_read_support_filter_all(arch.get());
        archive_read_support_format_raw(arch.get());
        if (auto ec = archive_read_open_filename(arch.get(), archive_path.c_str(), BUFFER_SIZE); ec != ARCHIVE_OK)
        {
            return tl::make_unexpected(make_compression_error_code(ec));
        }
        return arch;
    }
    
    tl::expected<archive_ptr, std::error_code> initialize_writer()
    {
        archive_ptr arch = {
            archive_write_disk_new(),
            [](archive *instance) -> void
            {
                archive_write_close(instance);
                archive_write_free(instance);
            }};
        int flags = ARCHIVE_EXTRACT_TIME;
        flags |= ARCHIVE_EXTRACT_PERM;
        flags |= ARCHIVE_EXTRACT_ACL;
        flags |= ARCHIVE_EXTRACT_FFLAGS;
        archive_write_disk_set_options(arch.get(), flags);
        archive_write_disk_set_standard_lookup(arch.get());
        return arch;
    }
}