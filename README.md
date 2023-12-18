# Factorio-Discord-Relay Revamped
Factorio Discord Relay (FDR), remade in C++! You can view the original project [here](https://github.com/Jaskowicz1/Factorio-Discord-Relay).

FDR is a simple executable (NOT a Factorio mod) which allows you to connect your Factorio server to Discord!

#### Powered by [rconpp](https://github.com/Jaskowicz1/rconpp)

### Features

- Support for all commands.
- Achievements still enabled when installed.
- Talk to Discord from Factorio and vice versa.

### Security Notice

Ideally, you should run this on the same machine as the Factorio server.
RCON is extremely insecure as it sends all the data as plain text over the network, meaning anyone can see what you're doing.

---

## Requirements

- [D++](https://github.com/brainboxdotcc/DPP/) (10.0.29 or higher).
- A Factorio Server.
- A Discord Bot with the `Message Content` and the `Server Members` intent on.
- A channel for messages.
- RCON enabled on the Factorio server (Add `--rcon-port <port (recommend: 27015)> --rcon-password <password>` to the arguments when running the server).

---

## Installation

Download the latest executable from the releases tab.

Once downloaded, place it anywhere you like. Create a `config.cfg` file next to the executable.

Your `config.cfg` should look something like this:
```
ip=127.0.0.1
port=27015
pass=changeme
bot_token=<token_here>
msg_channel=<channel_id>
allow_achievements=true
console_log_path=<log_path>
admin_role=<role_id>
```

From here, you can do `./fdr` and you should see FDR up and running! If everything went well, FDR should have sent a message in your Factorio and Discord chat!

## Configuration Information

This section will tell you what each part of the config means.

- `ip`, `port`, and `pass`, are your RCON connection config lines. These are essential for making sure FDR can connect to your Factorio server.
- `bot_token` is the Discord bot's token that you want to use for this.
- `msg_channel` is the ID of the channel that will be used for sending and receiving messages.
- `allow_achievements` is either `true` or `false`. If true (default), messages will be prefixed with `<server>:`. If false, messages will look tidier.
- `console_log_path` should be pointing to a log file created by adding `--console-log=<path>` to your server's arguments. Whatever path you give to that, set `console_log_path` to that.
- `admin_role` is the ID of a role that is allowed to execute the `/command <any_factorio_command>` command in Discord.
