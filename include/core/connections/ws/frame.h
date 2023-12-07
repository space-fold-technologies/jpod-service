#ifndef __CORE_CONNECTIONS_WEBSOCKET_FRAME__
#define __CORE_CONNECTIONS_WEBSOCKET_FRAME__
#include <vector>
#include <cstdint>
namespace core::connections::ws
{
    struct message
    {
        uint8_t opcode;
        uint32_t segments;
        std::vector<uint8_t> data;
    };
}
#endif // __CORE_CONNECTIONS_WEBSOCKET_FRAME__