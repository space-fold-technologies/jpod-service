#ifndef __JPOD_SERVICE_CORE_NETWORKS_MESSAGES_REGISTRY__
#define __JPOD_SERVICE_CORE_NETWORKS_MESSAGES_REGISTRY__
#include <map>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <functional>
#include <core/networks/messages/request.h>
#include <core/networks/messages/response.h>

namespace core::networks::messages
{
    class HandlerRegistry;
    struct registration
    {
        const void *handle{nullptr};
        core::networks::messages::HandlerRegistry *registry{nullptr};
    };

    class HandlerRegistry
    {
    public:
        template <typename ClassType, typename MemberFunction>
        void register_handler(std::string route, ClassType *class_instance, MemberFunction &&function) noexcept
        {
            safe_unique_registrations_access(
                [&, this]()
                {
                    auto it = handler_registrations.emplace(
                        std::move(route),
                        [class_instance, function](const Request &req, std::shared_ptr<Response> resp)
                        {
                            (class_instance->*function)(req, resp);
                        });
                    const void *handle = static_cast<const void *>(&(it->second));

                    registrations.emplace(it->first, registration{handle, this});
                });
        }

        void call(const std::string &route, const Request &req, std::shared_ptr<Response> resp) noexcept
        {
            if (auto pos = handler_registrations.find(route); pos != handler_registrations.end())
            {
                pos->second(req, resp);
            }
        }

    private:
        template <typename Callable>
        void safe_unique_registrations_access(Callable &&callable)
        {
            try
            {
                // if this fails, an exception may be thrown.
                std::unique_lock<mutex_type> lock(registration_mutex);
                callable();
            }
            catch (std::system_error &)
            {
                // do nothing
            }
        }

    private:
        using mutex_type = std::shared_mutex;
        std::unordered_multimap<std::string, std::function<void(
                                                 const Request &req,
                                                 std::shared_ptr<Response>)>>
            handler_registrations;
        mutable mutex_type registration_mutex;
        std::map<std::string, registration> registrations;
    };
}
#endif // __JPOD_SERVICE_CORE_NETWORKS_MESSAGES_REGISTRY__