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

    std::error_code copy_entry(struct archive *in, struct archive *out)
    {
        int ec;
        const void *buffer;
        std::size_t size;
        la_int64_t offset;
        while ((ec = archive_read_data_block(in, &buffer, &size, &offset)) == ARCHIVE_OK)
        {
            if (ec = archive_write_data_block(out, buffer, size, offset); ec != ARCHIVE_OK)
            {
                return make_compression_error_code(ec);
            }
        }
        if (ec = archive_write_finish_entry(out); ec != ARCHIVE_OK)
        {
            return make_compression_error_code(ec);
        }
        return {};
    }
    std::error_code copy_to_destination(archive_ptr in, archive_ptr out, fs::path &destination)
    {
        archive_entry *entry;
        while (archive_read_next_header(in.get(), &entry) == ARCHIVE_OK)
        {
            const char *entry_name = archive_entry_pathname(entry);
            const mode_t type = archive_entry_filetype(entry);
            fs::path full_path = destination / fs::path(std::string(entry_name));
            archive_entry_set_pathname(entry, full_path.generic_string().c_str());
            if (auto ec = archive_write_header(out.get(), entry); ec != ARCHIVE_OK)
            {
                return make_compression_error_code(ec);
            }
            else if (archive_entry_size(entry) > 0)
            {
                auto entry_size = archive_entry_size(entry);
                if (auto error = copy_entry(in.get(), out.get()); error)
                {
                    return error;
                }
            }
        }
        return {};
    }
}