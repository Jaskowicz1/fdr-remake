#include "main.hpp"

#include "rcon.h"
#include <dpp/dpp.h>

int main() {
    rcon rcon_client{"", 34198, ""};
    
    std::string data = rcon_client.send_data_sync("players", data_type::SERVERDATA_EXECCOMMAND);
    std::cout << "data from rcon: " << data << "\n";
    
    dpp::cluster bot("");
    
    /* Output simple log messages to stdout */
    bot.on_log(dpp::utility::cout_logger());
    
    bot.on_message_create([](const dpp::message_create_t& event) -> dpp::task<void> {
        if(event.msg.channel_id == 1185279867269435512) {
            
        }
        
        co_return;
    });

    /* Handle slash command with the most recent addition to D++ features, coroutines! */
    bot.on_slashcommand([](const dpp::slashcommand_t& event) -> dpp::task<void> {
        if (event.command.get_command_name() == "ping") {
            co_await event.co_reply("Pong!");
        }
        co_return;
    });

    /* Register slash command here in on_ready */
    bot.on_ready([&bot, &rcon_client](const dpp::ready_t& event) {
        /* Wrap command registration in run_once to make sure it doesnt run on every full reconnection */
        if (dpp::run_once<struct register_bot_commands>()) {
            bot.global_command_create(dpp::slashcommand("ping", "Ping pong!", bot.me.id));
        }
        
        //std::string response = client.send("/players");
        //std::cout << response << "\n";
    });

    
    bot.start(dpp::st_wait);
}
