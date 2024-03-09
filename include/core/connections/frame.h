#ifndef __DAEMON_CORE_CONNECTIONS_FRAME__
#define __DAEMON_CORE_CONNECTIONS_FRAME__

#include <cstdint>
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <array>
#include <spdlog/spdlog.h>

namespace core::connections
{

    static constexpr uint8_t _header_operation_mask = 0b00011111;
    static constexpr uint8_t _header_target_mask = 0b11100000;
    enum class operation_target : std::uint8_t
    {
        image = 0b00000000,
        container = 0b00100000,
        registry = 0b01000000,
        plugin = 0b01100000,
        client = 0b10000000
    };

    inline std::map<operation_target, std::string> operation_target_map =
        {
            {operation_target::image, "image-operation"},
            {operation_target::container, "container-operation"},
            {operation_target::registry, "registry-operation"},
            {operation_target::plugin, "plugin-operation"},
            {operation_target::client, "client-operation"}};

    inline std::string &operation_string_value(operation_target target)
    {
        static std::string unknown_key = "no matching operation target for supplied key";
        if (auto position = operation_target_map.find(target); position != operation_target_map.end())
        {
            return position->second;
        }
        return unknown_key;
    };

    enum class request_operation : std::uint8_t
    {
        list = 0x00,
        details = 0x01,
        removal = 0x02,
        push = 0x03,
        pull = 0x04,
        build = 0x05,
        import = 0x06,
        authorize = 0x07,
        start = 0x08,
        shell = 0x09,
        logs = 0x0A

    };

    inline std::map<request_operation, std::string> request_operations_map =
        {
            {request_operation::list, "list-sub-operation"},
            {request_operation::details, "details-sub-operation"},
            {request_operation::removal, "removal-sub-operation"},
            {request_operation::push, "push-sub-operation"},
            {request_operation::pull, "pull-sub-operation"},
            {request_operation::build, "build-sub-operation"},
            {request_operation::import, "import-sub-operation"},
            {request_operation::authorize, "authorize-sub-operation"},
            {request_operation::shell, "shell-sub-operation"},
            {request_operation::start, "start-sub-operation"},
            {request_operation::logs, "logs-sub-operation"}};

    inline std::string &request_operation_value(request_operation operation)
    {
        static std::string unknown_key = "no matching request-operation for the supplied key";
        if (auto position = request_operations_map.find(operation); position != request_operations_map.end())
        {
            return position->second;
        }
        return unknown_key;
    }
    enum class response_operation : std::uint8_t
    {
        success = 0x00,
        failure = 0x01,
        progress = 0x02,
        close_order = 0x03,
        frame = 0x04
    };

    inline std::map<response_operation, std::string> response_operation_map =
        {
            {response_operation::success, "success-response-type"},
            {response_operation::failure, "failure-response-type"},
            {response_operation::progress, "progress-response-type"},
            {response_operation::close_order, "close-order-type"},
            {response_operation::frame, "frame-type"}};

    inline std::string &response_operation_value(response_operation operation)
    {
        static std::string unknown_key = "unknown response operation type";
        if (auto position = response_operation_map.find(operation); position != response_operation_map.end())
        {
            return position->second;
        }
        return unknown_key;
    }

    struct frame
    {
        operation_target target;
        request_operation operation;
        std::vector<uint8_t> payload;
    };

    inline std::vector<uint8_t> encode_frame(operation_target target, response_operation operation, const std::vector<uint8_t> &data)
    {
        std::vector<uint8_t> header;
        std::size_t length = data.size();
        header.assign(2 + (length >= 126 ? 2 : 0) + (length >= 65536 ? 6 : 0), 0);
        header[0] = (static_cast<uint8_t>(operation) & _header_operation_mask) | (static_cast<uint8_t>(target) & _header_target_mask);

        if (length < 126)
        {
            header[1] = (length & 0xff) | 0;
        }
        else if (length < 65536)
        {
            header[1] = 126 | 0;
            header[2] = (length >> 8) & 0xff;
            header[3] = (length >> 0) & 0xff;
        }
        else
        {
            header[1] = 127 | 0;
            header[2] = (length >> 56) & 0xff;
            header[3] = (length >> 48) & 0xff;
            header[4] = (length >> 40) & 0xff;
            header[5] = (length >> 32) & 0xff;
            header[6] = (length >> 24) & 0xff;
            header[7] = (length >> 16) & 0xff;
            header[8] = (length >> 8) & 0xff;
            header[9] = (length >> 0) & 0xff;
        }

        std::vector<uint8_t> frame(header.begin(), header.end());
        frame.insert(frame.end(), data.begin(), data.end());
        return frame;
    }
    inline frame decode_frame(const std::vector<uint8_t> &data)
    {
        frame frm;
        frm.target = static_cast<operation_target>(data.at(0) & _header_target_mask);
        frm.operation = static_cast<request_operation>(data.at(0) & _header_operation_mask);
        std::size_t boundary = data.at(1) & 0x7F;
        std::size_t content_length = 0;
        std::size_t num_bytes = 0;
        // find the content length and how many bytes to skip to get to it
        if (boundary < 126)
        {
            content_length = boundary;
        }
        else if (boundary == 126)
        {
            num_bytes = 2;
            std::array<uint8_t, 2> length_bytes;
            std::memcpy(length_bytes.data(), &data[2], num_bytes);
            for (std::size_t c = 0; c < num_bytes; c++)
            {
                content_length += static_cast<std::size_t>(length_bytes[c]) << (8 * (num_bytes - 1 - c));
            }
        }
        else
        {
            num_bytes = 8;
            std::array<uint8_t, 8> length_bytes;
            std::memcpy(length_bytes.data(), &data[2], num_bytes);
            for (std::size_t c = 0; c < num_bytes; c++)
            {
                content_length += static_cast<std::size_t>(length_bytes[c]) << (8 * (num_bytes - 1 - c));
            }
        }
        frm.payload.reserve(content_length);
        auto start_position = data.begin() + (2 + num_bytes);
        auto end_position = data.begin() + (2 + num_bytes) + content_length;
        frm.payload.insert(frm.payload.end(), start_position, end_position);
        return frm;
    }
}
#endif // __DAEMON_CORE_CONNECTIONS_FRAME__