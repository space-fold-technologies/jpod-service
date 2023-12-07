#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

auto console = spdlog::stdout_color_mt("jpod");