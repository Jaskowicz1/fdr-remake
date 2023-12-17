#include "../include/main.h"

#include "../include/rcon.h"

#include <iostream>
#include <fstream>

int main() {
	std::ifstream config_file("config.cfg");
	std::string config_line{};

	std::string ip;
	unsigned int port;
	std::string pass;
	std::string bot_token;

	if (!config_file.good()) {
		std::cout << "Config file doesn't exist. Please create a config.cfg and follow the README.md on how to fill it.";
		return 0;
	}

	while (std::getline(config_file, config_line)) {
		std::string temp_line = config_line;

		if (temp_line.rfind("ip=", 0) == 0) {
			temp_line.replace(temp_line.find("ip="), sizeof("ip=") - 1, "");
			ip = temp_line;
		} else if (temp_line.rfind("port=", 0) == 0) {
			temp_line.replace(temp_line.find("port="), sizeof("port=") - 1, "");
			port = std::stoi(temp_line);
		} else if (temp_line.rfind("pass=", 0) == 0) {
			temp_line.replace(temp_line.find("pass="), sizeof("pass=") - 1, "");
			pass = temp_line;
		} else if (temp_line.rfind("bot_token=", 0) == 0) {
			temp_line.replace(temp_line.find("bot_token="), sizeof("bot_token=") - 1, "");
			bot_token = temp_line;
		} else if (temp_line.rfind("msg_channel=", 0) == 0) {
			temp_line.replace(temp_line.find("msg_channel="), sizeof("msg_channel=") - 1, "");
			FDR::config.msg_channel = temp_line;
		} else if (temp_line.rfind("allow_achievements=", 0) == 0) {
			temp_line.replace(temp_line.find("allow_achievements="), sizeof("allow_achievements=") - 1, "");
			FDR::config.allow_achievements = (temp_line == "true");

			if(!FDR::config.allow_achievements) {
				std::cout << "Warning: Achievements will now be disabled for your server." << "\n";
			}
		} else if (temp_line.rfind("console_log_path=", 0) == 0) {
			temp_line.replace(temp_line.find("console_log_path="), sizeof("console_log_path=") - 1, "");
			FDR::config.server_path = temp_line;

			std::cout << "Found console path. Checking if path exists." << "\n";

			if(!std::ifstream(FDR::config.server_path).good()) {
				std::cout << "Console path does not exist. FDR will only be able to communicate to Factorio and will not receive any input from Factorio." << "\n";
				FDR::config.can_communicate_to_console = false;
			} else {
				std::cout << "Console path exists. FDR can now run at full functionality!" << "\n";
				FDR::config.can_communicate_to_console = true;
			}
		} else if (temp_line.rfind("admin_role=", 0) == 0) {
			temp_line.replace(temp_line.find("admin_role="), sizeof("admin_role=") - 1, "");
			FDR::config.admin_role = temp_line;
		}
	}

	std::cout << "Configuration loaded. Starting FDR." << "\n";
    
	rcon rcon_client{ip, port, pass};
    
	dpp::cluster bot(bot_token, dpp::i_default_intents | dpp::i_message_content | dpp::i_guild_members, 0, 0, 1, true, dpp::cache_policy::cpol_none);
    
	/* Output simple log messages to stdout */
	bot.on_log(dpp::utility::cout_logger());
    
	bot.on_message_create([&rcon_client](const dpp::message_create_t& event) {
		if (event.msg.author.is_bot()) {
			return;
		}

		if (event.msg.channel_id == FDR::config.msg_channel) {
			// ID here doesn't matter really, we're not wanting a response.
			rcon_client.send_data(event.msg.content, 999, data_type::SERVERDATA_EXECCOMMAND);
		}
	});

	bot.on_slashcommand([&rcon_client](const dpp::slashcommand_t& event) {
		if (event.command.get_command_name() == "evolution") {
    			rcon_client.send_data("/evolution", 3, data_type::SERVERDATA_EXECCOMMAND, [event](const rcon_response& response) {
				event.reply(dpp::message(response.data).set_flags(dpp::m_ephemeral));
	    		});
		} else if (event.command.get_command_name() == "time") {
	    		rcon_client.send_data("/time", 3, data_type::SERVERDATA_EXECCOMMAND, [event](const rcon_response& response) {
				event.reply(dpp::message("Server uptime: " + response.data).set_flags(dpp::m_ephemeral));
	    		});
		} else if (event.command.get_command_name() == "version") {
	    		rcon_client.send_data("/version", 3, data_type::SERVERDATA_EXECCOMMAND, [event](const rcon_response& response) {
				event.reply(dpp::message("Factorio version: " + response.data).set_flags(dpp::m_ephemeral));
	    		});
		} else if (event.command.get_command_name() == "players") {
	    		rcon_client.send_data("/players online", 3, data_type::SERVERDATA_EXECCOMMAND, [event](const rcon_response& response) {
				event.reply(dpp::message(response.data).set_flags(dpp::m_ephemeral));
	    		});
		} else if (event.command.get_command_name() == "seed") {
	    		rcon_client.send_data("/seed", 3, data_type::SERVERDATA_EXECCOMMAND, [event](const rcon_response& response) {
				event.reply(dpp::message(response.data).set_flags(dpp::m_ephemeral));
	    		});
		} else if (event.command.get_command_name() == "command") {
			if(FDR::config.allow_achievements) {
				event.reply(dpp::message("Since achievements are enabled, you are not allowed to run this command!").set_flags(dpp::m_ephemeral));
				return;
			}

			if (std::find(event.command.member.get_roles().begin(), event.command.member.get_roles().end(), FDR::config.admin_role) == event.command.member.get_roles().end()) {
				event.reply(dpp::message("You do not have the required role to run this command!").set_flags(dpp::m_ephemeral));
				return;
			}

			auto command_to_run = std::get<std::string>(event.get_parameter("cmd"));

			rcon_client.send_data("/command " + command_to_run, 3, data_type::SERVERDATA_EXECCOMMAND, [event](const rcon_response& response) {
				if(response.data.empty()) {
					return;
				}

				event.reply(dpp::message(response.data).set_flags(dpp::m_ephemeral));
		    	});
		}
	});

	/* Register slash command here in on_ready */
	bot.on_ready([&bot, &rcon_client](const dpp::ready_t& event) {
		/* Wrap command registration in run_once to make sure it doesn't run on every full reconnection */
		if (dpp::run_once<struct register_bot_commands>()) {
			dpp::slashcommand evolution_command("evolution", "See the current evolution", bot.me.id);
			dpp::slashcommand time_command("time", "See the current time", bot.me.id);
			dpp::slashcommand version_command("version", "See the current version", bot.me.id);
			dpp::slashcommand players_command("players", "See the current amount players", bot.me.id);
			dpp::slashcommand seed_command("seed", "See the current seed", bot.me.id);
			dpp::slashcommand cmd_command("command", "Run any command!", bot.me.id);

			cmd_command.add_option(dpp::command_option(dpp::co_string, "cmd", "the command to run!", true));

			bot.global_bulk_command_create({ evolution_command, time_command, version_command, players_command, seed_command, cmd_command });
		}

		rcon_client.send_data("/players online count", 2, data_type::SERVERDATA_EXECCOMMAND,
				      [&bot](const rcon_response& response) {
			std::string players = response.data;
			std::replace(players.begin(), players.end(), ':', ' ');
			std::replace(players.begin(), players.end(), '(', ' ');
			std::replace(players.begin(), players.end(), ')', ' ');
			bot.set_presence(dpp::presence(dpp::presence_status::ps_online, dpp::at_custom, players));
		});

		/* Create a timer that runs every 120 seconds, that sets the status */
		bot.start_timer([&bot, &rcon_client](const dpp::timer& timer) {
			rcon_client.send_data("/players online count", 2, data_type::SERVERDATA_EXECCOMMAND, [&bot](const rcon_response& response) {
				std::string players = response.data;
				std::replace(players.begin(), players.end(), ':', ' ');
				std::replace(players.begin(), players.end(), '(', ' ');
				std::replace(players.begin(), players.end(), ')', ' ');
				bot.set_presence(dpp::presence(dpp::presence_status::ps_online, dpp::at_custom, players));
	    		});
		}, 120);

		bot.message_create(dpp::message(FDR::config.msg_channel, "Factorio-Discord-Relay (FDR) has loaded!"), [&rcon_client](const dpp::confirmation_callback_t& callback) {
			rcon_client.send_data("Factorio-Discord-Relay (FDR) has loaded!", 999, data_type::SERVERDATA_EXECCOMMAND);
		});
	});

    	bot.start(dpp::st_wait);
    	return 0;
}
