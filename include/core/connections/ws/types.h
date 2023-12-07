#ifndef __CORE_CONNECTIONS_WEBSOCKET_TYPES__
#define __CORE_CONNECTIONS_WEBSOCKET_TYPES__

#include <string>
#include <map>

namespace core::connections::ws
{
    enum class StatusCode
    {
        unknown = 0,
        information_continue = 100,
        information_switching_protocols,
        information_processing,
        success_ok = 200,
        success_created,
        success_accepted,
        success_non_authoritative_information,
        success_no_content,
        success_reset_content,
        success_partial_content,
        success_multi_status,
        success_already_reported,
        success_im_used = 226,
        redirection_multiple_choices = 300,
        redirection_moved_permanently,
        redirection_found,
        redirection_see_other,
        redirection_not_modified,
        redirection_use_proxy,
        redirection_switch_proxy,
        redirection_temporary_redirect,
        redirection_permanent_redirect,
        error_upgrade_required = 426,
        error_internal_server_error = 500,
        error_not_implemented,
        error_bad_gateway,
        error_service_unavailable,
        error_gateway_timeout,
        error_http_version_not_supported,
        error_variant_also_negotiates,
        error_insufficient_storage,
        error_loop_detected,
        error_not_extended = 510,
        error_network_authentication_required,
        error_unmasked_payload = 1002,
        error_max_payload_limit_reached = 1009
    };

    static const std::map<StatusCode, std::string> StatusCode_strings =
        {
            {StatusCode::unknown, ""},
            {StatusCode::information_continue, "100 Continue"},
            {StatusCode::information_switching_protocols, "101 Switching Protocols"},
            {StatusCode::information_processing, "102 Processing"},
            {StatusCode::success_ok, "200 OK"},
            {StatusCode::success_created, "201 Created"},
            {StatusCode::success_accepted, "202 Accepted"},
            {StatusCode::success_non_authoritative_information, "203 Non-Authoritative Information"},
            {StatusCode::success_no_content, "204 No Content"},
            {StatusCode::success_reset_content, "205 Reset Content"},
            {StatusCode::success_partial_content, "206 Partial Content"},
            {StatusCode::success_multi_status, "207 Multi-Status"},
            {StatusCode::success_already_reported, "208 Already Reported"},
            {StatusCode::success_im_used, "226 IM Used"},
            {StatusCode::redirection_multiple_choices, "300 Multiple Choices"},
            {StatusCode::redirection_moved_permanently, "301 Moved Permanently"},
            {StatusCode::redirection_found, "302 Found"},
            {StatusCode::redirection_see_other, "303 See Other"},
            {StatusCode::redirection_not_modified, "304 Not Modified"},
            {StatusCode::redirection_use_proxy, "305 Use Proxy"},
            {StatusCode::redirection_switch_proxy, "306 Switch Proxy"},
            {StatusCode::redirection_temporary_redirect, "307 Temporary Redirect"},
            {StatusCode::redirection_permanent_redirect, "308 Permanent Redirect"},
            {StatusCode::error_upgrade_required, "426 Client Upgrade required"},
            {StatusCode::error_internal_server_error, "500 Internal Server Error"},
            {StatusCode::error_not_implemented, "501 Not Implemented"},
            {StatusCode::error_bad_gateway, "502 Bad Gateway"},
            {StatusCode::error_service_unavailable, "503 Service Unavailable"},
            {StatusCode::error_gateway_timeout, "504 Gateway Timeout"},
            {StatusCode::error_http_version_not_supported, "505 HTTP Version Not Supported"},
            {StatusCode::error_variant_also_negotiates, "506 Variant Also Negotiates"},
            {StatusCode::error_insufficient_storage, "507 Insufficient Storage"},
            {StatusCode::error_loop_detected, "508 Loop Detected"},
            {StatusCode::error_not_extended, "510 Not Extended"},
            {StatusCode::error_network_authentication_required, "511 Network Authentication Required"},
            {StatusCode::error_unmasked_payload, "1002 Unmasked Payload Received"}};

    inline const std::string &status_code_value(StatusCode code) noexcept
    {
        auto position = StatusCode_strings.find(code);
        if (position == StatusCode_strings.end())
        {
            static std::string empty_string;
            return empty_string;
        }
        return position->second;
    }
}

#endif // __CORE_CONNECTIONS_WEBSOCKET_TYPES__