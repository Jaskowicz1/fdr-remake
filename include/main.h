#pragma once

#include <dpp/dpp.h>

#include <regex>

struct bot_config {
	dpp::snowflake msg_channel{0};
	/**
	 * @brief If this is false, messages will look nicer.
	 */
	bool allow_achievements{true};
	dpp::snowflake admin_role{0};
	std::string server_path{};

	bool can_communicate_to_console{false};
};

namespace FDR {
	bot_config config;

	inline dpp::cluster* botRef;

	void read_console();
	long long last_char_read{0};

	inline static std::string pretify_log_line(const std::string& log_line) {
		return std::regex_replace(log_line, std::regex(R"(^.+\[\S+\]\s(.+)$)"), "$1");
	}
}