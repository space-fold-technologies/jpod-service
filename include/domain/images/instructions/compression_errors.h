#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_COMPRESSION_ERRORS__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_COMPRESSION_ERRORS__

#include <map>
#include <string>
#include <zip.h>

namespace domain::images::instructions
{
    inline std::map<int, std::string> compression_error_map =
        {
            {ZIP_ER_OK, "N No error"},
            {ZIP_ER_MULTIDISK, "N Multi-disk zip archives not supported"},
            {ZIP_ER_RENAME, "S Renaming temporary file failed"},
            {ZIP_ER_CLOSE, "S Closing zip archive failed"},
            {ZIP_ER_SEEK, "S Seek error"},
            {ZIP_ER_READ, "S Read error"},
            {ZIP_ER_WRITE, "S Write error"},
            {ZIP_ER_CRC, "N CRC error"},
            {ZIP_ER_ZIPCLOSED, "N Containing zip archive was closed"},
            {ZIP_ER_NOENT, "N No such file"},
            {ZIP_ER_EXISTS, "N File already exists"},
            {ZIP_ER_OPEN, "S Can't open file"},
            {ZIP_ER_TMPOPEN, "S Failure to create temporary file"},
            {ZIP_ER_ZLIB, "Z Zlib error"},
            {ZIP_ER_MEMORY, "N Malloc failure"},
            {ZIP_ER_CHANGED, "N Entry has been changed"},
            {ZIP_ER_COMPNOTSUPP, "N Compression method not supported"},
            {ZIP_ER_EOF, "N Premature end of file"},
            {ZIP_ER_INVAL, "N Invalid argument"},
            {ZIP_ER_NOZIP, "N Not a zip archive"},
            {ZIP_ER_INTERNAL, "N Internal error"},
            {ZIP_ER_INCONS, "L Zip archive inconsistent"},
            {ZIP_ER_REMOVE, "S Can't remove file"},
            {ZIP_ER_DELETED, "N Entry has been deleted"},
            {ZIP_ER_ENCRNOTSUPP, "N Encryption method not supported"},
            {ZIP_ER_RDONLY, "N Read-only archive"},
            {ZIP_ER_NOPASSWD, "N No password provided"},
            {ZIP_ER_WRONGPASSWD, "N Wrong password provided"},
            {ZIP_ER_OPNOTSUPP, "N Operation not supported"},
            {ZIP_ER_INUSE, "N Resource still in use"},
            {ZIP_ER_TELL, "S Tell error"},
            {ZIP_ER_COMPRESSED_DATA, "N Compressed data invalid"},
            {ZIP_ER_CANCELLED, "N Operation cancelled"},
            {ZIP_ER_DATA_LENGTH, "N Unexpected length of data"},
            {ZIP_ER_NOT_ALLOWED, "N Not allowed in torrentzip"}};
    struct compression_failure_category : public std::error_category
    {
        compression_failure_category() {}
        virtual ~compression_failure_category() = default;
        compression_failure_category(const compression_failure_category &) = delete;
        const char *name() const noexcept override
        {
            return "compression failures";
        }

        std::string message(int ec) const override
        {
            static const std::string unknown_error_code("unknown compression failure");
            if (auto pos = compression_error_map.find(ec); pos != compression_error_map.end())
            {
                return pos->second;
            }
            return unknown_error_code;
        }
    };

    inline const compression_failure_category &__compression_failure_category()
    {
        static compression_failure_category fc;
        return fc;
    }

    inline const std::error_code make_compression_error_code(int ec) noexcept
    {

        return std::error_code{ec, __compression_failure_category()};
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_COMPRESSION_ERRORS__