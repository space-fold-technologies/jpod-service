#ifndef __DAEMON_CORE_HTTP_UTILITY__
#define __DAEMON_CORE_HTTP_UTILITY__

#include <asio/buffers_iterator.hpp>
#include <asio/read_until.hpp>

namespace core::http
{
    class header_end_match
    {
    public:
        std::pair<asio::buffers_iterator<asio::const_buffers_1>, bool> operator()(asio::buffers_iterator<asio::const_buffers_1> begin, asio::buffers_iterator<asio::const_buffers_1> end)
        {
            auto it = begin;
            for (; it != end; ++it)
            {
                if (*it == '\n')
                {
                    if (crlfcrlf == 1)
                        ++crlfcrlf;
                    else if (crlfcrlf == 2)
                        crlfcrlf = 0;
                    else if (crlfcrlf == 3)
                        return {++it, true};
                    if (lflf == 0)
                        ++lflf;
                    else if (lflf == 1)
                        return {++it, true};
                }
                else if (*it == '\r')
                {
                    if (crlfcrlf == 0)
                        ++crlfcrlf;
                    else if (crlfcrlf == 2)
                        ++crlfcrlf;
                    else
                        crlfcrlf = 0;
                    lflf = 0;
                }
                else
                {
                    crlfcrlf = 0;
                    lflf = 0;
                }
            }
            return {it, false};
        }

    private:
        int crlfcrlf = 0;
        int lflf = 0;
    };
}

namespace asio
{
    template <>
    struct is_match_condition<core::http::header_end_match> : public std::true_type {};
}

#endif //__DAEMON_CORE_HTTP_UTILITY__