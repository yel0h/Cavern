# Cavern
Cavern is a simple voxel sandbox game with multiplayer support written in C++ using OpenGL shaders.

<img width="856" height="512" alt="image" src="https://github.com/user-attachments/assets/6a228bab-8e51-49d1-a9e7-824b80d8d517" />

## Running
To start the game in singleplayer, simply run `Cavern.exe`.

To host a server, run `Cavern.exe --host`.

To join a server, run `Cavern.exe --join <ip_to_join>`.

## Default controls
- WASD to move around
- Space to jump
- Mouse to look around
- Right click to switch between placing and digging
- Left click to do the chosen action
- Enter to set a spawnpoint and save world
- 1-8/Scroll to select a block
- F11 to toggle fullscreen
- F to cycle fog distance
- N to generate a new level
- ESC to pause
- F1-F10 to save and load world slots
- T to open chat
- Tab to show player list
- B to open inventory
- V to throw a bolt
- H to place sign
- P to cycle world size

## Commands
- `/warden <name>` - grant warden status
- `/unwarden <name>` - revoke warden status
- `/exile <name>` - exile player from server
- `/pardon <name>` - remove player from exile list
- `/exileip <ip>` - exile IP from server
- `/expel <name>` - expel player from server
- `/say <message>` - send public message (equivalent to using the chat function)
- `/setspawn` - set server's spawnpoint to current location
- `/teleport <name>` or `/tp <name>` - teleport player to your position
- `/forge` - place Bedrock instead of Stone

## Technical Details
The current world block data is saved in the world.dat file, either manually (see Controls) or automatically upon exiting the game. Additional world saves can be created in 5 slots, each of which exists as a save_\<id\>.dat file. Wanderers are saved in wanderers.dat, the new mobs in mobs.dat, signs in signs.dat, and the player's spawnpoint is stored in spawn.dat. Settings, including keybinds, are persisted in the settings.cfg file. When a spawnpoint is set on a server, it's saved in the server_spawn.dat file. Upon starting, the server outputs arguments that can be used by other players to join in externalurl.txt. It also provides a list of players that are currently online in logged-in.txt.

### Server config files
A server persists its configuration across multiple files containing lists of line-separated values. The server's main configuration can be set in server.cfg (see below for available options). Names of exiled players are stored in exile-list.txt, exiled IPs in exiled-ips.txt, and names of players that have been granted warden status in wardens.txt.

#### server.cfg options
The server.cfg configuration file contains a list of key=value pairs. Here's a list of all supported keys:
- `private` - when set to `true`, the externalurl.txt file doesn't get created
- `max-connections` - integer value representing the number of connections allowed from one IP; `3` by default
