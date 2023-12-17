#pragma once

#include <dpp/dpp.h>

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
}