#include <core/sha/utilities.h>
#include <openssl/evp.h>
#include <fstream>
#include <sstream>
#include <memory>
#include <array>

namespace core::sha
{
    using evp_md_ctx_ptr = std::unique_ptr<EVP_MD_CTX, decltype(&::EVP_MD_CTX_free)>;

    template <typename T>
    std::string convert_to_hex(const T &input)
    {
        std::ostringstream ss;
        ss << std::hex << std::setfill('0');
        for (unsigned int i = 0; i < input.size(); ++i)
        {
            ss << std::setw(2) << static_cast<unsigned>(input.at(i));
        }

        return ss.str();
    }

    hash_report compute_sha256_of_file(const std::filesystem::path &path)
    {
        std::array<uint8_t, 32> hash{};
        evp_md_ctx_ptr ctx(EVP_MD_CTX_new(), ::EVP_MD_CTX_free);
        if (auto ec = EVP_DigestInit_ex(ctx.get(), EVP_sha256(), nullptr); ec != 1)
        {
            // TODO:: need to make error codes for openssl
            return tl::make_unexpected(std::error_code{ec, std::system_category()});
        }
        std::fstream fp(path, std::ios::in | std::ios::binary);
        if (!fp.good())
        {
            return tl::make_unexpected(std::error_code{errno, std::system_category()});
        }
        std::size_t buffer_size = 1 << 12;
        char buffer[buffer_size];
        while (fp.good())
        {
            fp.read(buffer, buffer_size);
            if (auto ec = EVP_DigestUpdate(ctx.get(), buffer, fp.good()); ec != 1)
            {
                return tl::make_unexpected(std::error_code{ec, std::system_category()});
            }
        }
        if (auto ec = EVP_DigestFinal_ex(ctx.get(), hash.data(), nullptr); ec != 1)
        {
            return tl::make_unexpected(std::error_code{ec, std::system_category()});
        }
        return convert_to_hex(hash);
    }
    hash_report compute_sha256_sum(const std::vector<std::string> &hash_list)
    {
        std::array<uint8_t, 32> hash{};
        evp_md_ctx_ptr ctx(EVP_MD_CTX_new(), ::EVP_MD_CTX_free);
        if (auto ec = EVP_DigestInit_ex(ctx.get(), EVP_sha256(), nullptr); ec != 1)
        {
            // TODO:: need to make error codes for openssl
            return tl::make_unexpected(std::error_code{ec, std::system_category()});
        }
        for (const auto &entry : hash_list)
        {
            if (auto ec = EVP_DigestUpdate(ctx.get(), entry.c_str(), hash.size()); ec != 1)
            {
                return tl::make_unexpected(std::error_code{ec, std::system_category()});
            }
        }
        if (auto ec = EVP_DigestFinal_ex(ctx.get(), hash.data(), nullptr); ec != 1)
        {
            return tl::make_unexpected(std::error_code{ec, std::system_category()});
        }
        return convert_to_hex(hash);
    }
}