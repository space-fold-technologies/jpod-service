#include <core/connections/ws/crypto.h>
#include <openssl/buffer.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <memory>

// TODO 2017: remove workaround for MSVS 2012
#if _MSC_VER == 1700 // MSVS 2012 has no definition for round()
inline double round(double x) noexcept
{ // Custom definition of round() for positive numbers
  return floor(x + 0.5);
}
#endif

namespace core::connections::ws
{
  std::string Crypto::base64_encode(const std::string &input) noexcept
  {
    std::string base64;
    BIO *bio, *b64;
    auto bptr = BUF_MEM_new();

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new(BIO_s_mem());
    BIO_push(b64, bio);
    BIO_set_mem_buf(b64, bptr, BIO_CLOSE);

    // Write directly to base64-buffer to avoid copy
    auto base64_length = static_cast<std::size_t>(round(4 * ceil(static_cast<double>(input.size()) / 3.0)));
    base64.resize(base64_length);
    bptr->length = 0;
    bptr->max = base64_length + 1;
    bptr->data = &base64[0];

    if (BIO_write(b64, &input[0], static_cast<int>(input.size())) <= 0 || BIO_flush(b64) <= 0)
      base64.clear();

    // To keep &base64[0] through BIO_free_all(b64)
    bptr->length = 0;
    bptr->max = 0;
    bptr->data = nullptr;

    BIO_free_all(b64);

    return base64;
  }
  std::string Crypto::base64_decode(const std::string &base64) noexcept
  {
    std::string ascii((6 * base64.size()) / 8,
                      '\0'); // The size is a up to two bytes too large.

    BIO *b64, *bio;

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
// TODO: Remove in 2022 or later
#if (defined(OPENSSL_VERSION_NUMBER) &&       \
     OPENSSL_VERSION_NUMBER < 0x1000214fL) || \
    (defined(LIBRESSL_VERSION_NUMBER) &&      \
     LIBRESSL_VERSION_NUMBER < 0x2080000fL)
    bio = BIO_new_mem_buf(const_cast<char *>(&base64[0]),
                          static_cast<int>(base64.size()));
#else
    bio = BIO_new_mem_buf(&base64[0], static_cast<int>(base64.size()));
#endif
    bio = BIO_push(b64, bio);

    auto decoded_length =
        BIO_read(bio, &ascii[0], static_cast<int>(ascii.size()));
    if (decoded_length > 0)
      ascii.resize(static_cast<std::size_t>(decoded_length));
    else
      ascii.clear();

    BIO_free_all(b64);

    return ascii;
  }

  std::string Crypto::message_digest(const std::string &str, const EVP_MD *evp_md,
                                     std::size_t digest_length) noexcept
  {
    std::string md(digest_length, '\0');
    auto ctx = EVP_MD_CTX_create();
    EVP_MD_CTX_init(ctx);
    EVP_DigestInit_ex(ctx, evp_md, nullptr);
    EVP_DigestUpdate(ctx, str.data(), str.size());
    EVP_DigestFinal_ex(ctx, reinterpret_cast<unsigned char *>(&md[0]), nullptr);
    EVP_MD_CTX_destroy(ctx);
    return md;
  }

  std::string Crypto::sha1(const std::string &input,
                           std::size_t iterations) noexcept
  {
    auto evp_md = EVP_sha1();
    auto hash = message_digest(input, evp_md, SHA_DIGEST_LENGTH);
    for (std::size_t i = 1; i < iterations; ++i)
      hash = message_digest(hash, evp_md, SHA_DIGEST_LENGTH);
    return hash;
  }
} // namespace core::connections::ws