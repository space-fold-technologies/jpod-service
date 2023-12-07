#ifndef __JPOD_SERVICE_CORE_BUS_EVENT_BUS__
#define __JPOD_SERVICE_CORE_BUS_EVENT_BUS__

#include <any>
#include <atomic>
#include <core/bus/function_traits.h>
#include <functional>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <thread>
#include <typeindex>
#include <unordered_map>
#include <utility>

namespace core::bus
{
    class EventBus
    {
    public:
        EventBus() = default;
        template <typename EventType,
                  typename EventHandler,
                  typename = std::enable_if_t<std::is_invocable_v<EventHandler> || std::is_invocable_v<EventHandler, EventType>>>
        void register_handler(const std::string &identifier, EventHandler &&handler)
        {
            // using traits = details::function_traits<EventHandler>;
            const auto type_index = std::type_index(typeid(EventType));
            safe_unique_registrations_access([&]()
                                             {
      handlers.try_emplace(identifier, std::unordered_map<std::type_index, std::function<void(std::any)>>());
      // if (handlers.find(identifier) == handlers.end()) { handlers[identifier] = {}; }
      handlers[identifier].emplace(type_index,
        [func = std::forward<EventHandler>(handler)](auto value) { func(std::any_cast<EventType>(value)); }); });
        }

        template <typename EventType, typename = std::enable_if_t<!std::is_pointer_v<EventType>>>
        void send(const std::string &identifier, EventType &&event) noexcept
        {
            safe_shared_registrations_access([this, &identifier, local_event = std::forward<EventType>(event)]()
                                             {
      if (auto iterator = handlers.find(identifier); iterator != handlers.end()) {
        if (auto position = iterator->second.find(std::type_index(typeid(EventType)));
            position != iterator->second.end()) {
          position->second(local_event);
        }
      } else {
        // this meants there is no known identifier that matches what you are looking for
      } });
        }

        bool remove_owned_by(const std::string &identifier)
        {
            bool result = false;
            safe_unique_registrations_access([&]()
                                             {
      if (auto iterator = handlers.find(identifier); iterator != handlers.end()) {
        handlers.erase(iterator);
        result = true;
      } });
            return result;
        }

        void remove_handlers() noexcept
        {
            safe_unique_registrations_access([this]()
                                             { handlers.clear(); });
        }

    private:
        using mutex_type = std::shared_mutex;
        mutable mutex_type registration_mutex;
        std::unordered_map<std::string, std::unordered_map<std::type_index, std::function<void(std::any)>>> handlers;

    private:
        template <typename Callable>
        void safe_unique_registrations_access(Callable &&callable) const
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

        template <typename Callable>
        void safe_shared_registrations_access(Callable &&callable) const
        {
            try
            {
                // if this fails, an exception may be thrown.
                std::shared_lock<mutex_type> lock(registration_mutex);
                callable();
            }
            catch (std::system_error &)
            {
                // do nothing
            }
        }
    };
}
#endif // __JPOD_SERVICE_CORE_BUS_EVENT_BUS__