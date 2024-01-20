#ifndef __DAEMON_DOMAIN_IMAGES_HTTP_CLIENT__
#define __DAEMON_DOMAIN_IMAGES_HTTP_CLIENT__

#include <string>
#include <domain/images/http/contracts.h>
#include <domain/images/http/request.h>
#include <memory>

namespace domain::images::http
{

    class client
    {
    public:
        virtual void download(const std::string &path,const std::map<std::string, std::string> &headers, std::shared_ptr<download_destination> sink, report_callback callback) = 0;
        virtual void execute(const request &req, response_callback callback) = 0;
        virtual void upload(const std::string &path, const std::map<std::string, std::string> &headers, const fs::path &file_path, upload_callback callback) = 0;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_HTTP_CLIENT__