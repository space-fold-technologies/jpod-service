#ifndef __DAEMON_DOMAIN_IMAGES_HTTP_CONNECTION__
#define __DAEMON_DOMAIN_IMAGES_HTTP_CONNECTION__
#include <domain/images/http/request.h>
#include <domain/images/http/response.h>
#include <domain/images/http/contracts.h>
#include <memory>

// https://stackoverflow.com/questions/8927219/transfer-large-files-using-http-post
// https://sharetechnotes.com/technotes/c-how-to-write-a-file-to-a-socket-using-the-chunked-http-transfer-protocol-in-boostasio/
// https://github.com/sprinfall/webcc/tree/master?tab=readme-ov-file#uploading-files
// https://racineennis.ca/2017/03/21/Resumable-file-transfers-with-content-range-header
// https://datatracker.ietf.org/doc/html/rfc7233

namespace domain::images::http
{
    class connection
    {
    public:
        virtual void download(const internal::uri &uri, const std::map<std::string, std::string> &headers, std::shared_ptr<download_destination> sink, report_callback callback) = 0;
        virtual void execute(const request &req, response_callback callback) = 0;
        virtual void upload(
            const internal::uri &uri,
            const std::map<std::string, std::string> &headers,
            const fs::path &file_path,
            upload_callback callback) = 0;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_HTTP_CONNECTION__