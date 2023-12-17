#include "../include/main.h"

#include "../include/rcon.h"
#include <dpp/dpp.h>

int main(int argc, char *argv[]) {

	// ./fdr ip port password bot_token channel_id
	if(argc < 6) {
		std::cout << "Not enough arguments specified. Please check the command you are running." << "\n";
		return 0;
	}
    
	unsigned int port = std::atoi(argv[2]);
	const dpp::snowflake msg_channel{argv[5]};
    
	rcon rcon_client{argv[1], port, argv[3]};
    
	dpp::cluster bot(argv[4], dpp::i_default_intents | dpp::i_message_content, 0, 0, 1, true, dpp::cache_policy::cpol_none);
    
	/* Output simple log messages to stdout */
	bot.on_log(dpp::utility::cout_logger());
    
	bot.on_message_create([&rcon_client, msg_channel](const dpp::message_create_t& event) {
		if(event.msg.channel_id == msg_channel && !event.msg.author.is_bot()) {
			// ID here doesn't matter really, we're not wanting a response.
			rcon_client.send_data(event.msg.content, 999, data_type::SERVERDATA_EXECCOMMAND);
		}
	});

	bot.on_slashcommand([&rcon_client](const dpp::slashcommand_t& event) {
		if (event.command.get_command_name() == "evolution") {
    			rcon_client.send_data("/evolution", 3, data_type::SERVERDATA_EXECCOMMAND, [event](const std::string& data) {
				event.reply(dpp::message(data).set_flags(dpp::m_ephemeral));
	    		});
		} else if (event.command.get_command_name() == "time") {
	    		rcon_client.send_data("/time", 3, data_type::SERVERDATA_EXECCOMMAND, [event](const std::string& data) {
				event.reply(dpp::message(data).set_flags(dpp::m_ephemeral));
	    		});
		} else if (event.command.get_command_name() == "version") {
	    		rcon_client.send_data("/version", 3, data_type::SERVERDATA_EXECCOMMAND, [event](const std::string& data) {
				event.reply(dpp::message(data).set_flags(dpp::m_ephemeral));
	    		});
		} else if (event.command.get_command_name() == "players") {
	    		rcon_client.send_data("/players online", 3, data_type::SERVERDATA_EXECCOMMAND, [event](const std::string& data) {
				event.reply(dpp::message(data).set_flags(dpp::m_ephemeral));
	    		});
		} else if (event.command.get_command_name() == "seed") {
	    		rcon_client.send_data("/seed", 3, data_type::SERVERDATA_EXECCOMMAND, [event](const std::string& data) {
				event.reply(dpp::message(data).set_flags(dpp::m_ephemeral));
	    		});
		} else if (event.command.get_command_name() == "command") {
			auto command_to_run = std::get<std::string>(event.get_parameter("cmd"));

			rcon_client.send_data("/command " + command_to_run, 3, data_type::SERVERDATA_EXECCOMMAND, [event](const std::string& data) {
				event.reply(dpp::message(data).set_flags(dpp::m_ephemeral));
		    	});
		}
	});

	/* Register slash command here in on_ready */
	bot.on_ready([&bot, &rcon_client](const dpp::ready_t& event) {
		/* Wrap command registration in run_once to make sure it doesnt run on every full reconnection */
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
				      [&bot](const std::string& data) {
			std::string players = data;
			std::replace(players.begin(), players.end(), ':', ' ');
			std::replace(players.begin(), players.end(), '(', ' ');
			std::replace(players.begin(), players.end(), ')', ' ');
			bot.set_presence(dpp::presence(dpp::presence_status::ps_online, dpp::at_custom, players));
		});

		/* Create a timer that runs every 120 seconds, that sets the status */
		bot.start_timer([&bot, &rcon_client](const dpp::timer& timer) {
			rcon_client.send_data("/players online count", 2, data_type::SERVERDATA_EXECCOMMAND, [&bot](const std::string& data) {
				std::string players = data;
				std::replace(players.begin(), players.end(), ':', ' ');
				std::replace(players.begin(), players.end(), '(', ' ');
				std::replace(players.begin(), players.end(), ')', ' ');
				bot.set_presence(dpp::presence(dpp::presence_status::ps_online, dpp::at_custom, players));
	    		});
		}, 120);
	});

    	bot.start(dpp::st_wait);
    	return 0;
}
