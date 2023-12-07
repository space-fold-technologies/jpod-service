#include <core/images/image_controller.h>

namespace images {
    void on_list(const Request &req, std::shared_ptr<Response> resp);
        void on_fetch(const Request &req, std::shared_ptr<Response> resp);
        void on_build(const Request &req, std::shared_ptr<Response> resp);
        void on_remove(const Request &req, std::shared_ptr<Response> resp);
}