# Cavern
Cavern is a simple voxel sandbox game with multiplayer support written in C++ using OpenGL shaders.

<img width="856" height="512" alt="image" src="https://github.com/user-attachments/assets/418e0032-4dee-4c63-b5e0-6354fbf2703e" />

## Running
To start the game in singleplayer, simply run `Cavern.exe`.

To host a server, run `Cavern.exe --host`.

To join a server, run `Cavern.exe --join <ip_to_join>`.

## Controls
- WASD to move around
- Space to jump
- Mouse to look around
- R to respawn
- Right click to switch between placing and digging
- Left click to do the chosen action
- Enter to set a spawnpoint and save world
- 1-8/Scroll to select a block
- G to spawn a Wanderer
- F11 to toggle fullscreen
- F to cycle fog distance
- N to generate a new level
- Y to invert mouse Y axis
- ESC to pause
- F1-F10 to save and load world slots
- T to open chat
- Tab to show player list

## Commands
- `/warden <name>` - grant warden status
- `/unwarden <name>` - revoke warden status
- `/exile <name>` - exile player from server
- `/pardon <name>` - remove player from exile list
- `/exileip <ip>` - exile IP from server
- `/expel <name>` - expel player from server
- `/say <message>` - send public message (equivalent to using the chat function)
- `/setspawn` - set server's spawnpoint to current location

## Technical Details
The current world block data is saved in the world.dat file, either manually (see Controls) or automatically upon exiting the game. Additional world saves can be created in 5 slots, each of which exists as a save_\<id\>.dat file. Wanderers are saved in wanderers.dat, while the player's spawnpoint is stored in spawn.dat. When a spawnpoint is set on a server, it's saved in the server_spawn.dat file. Upon starting, the server outputs arguments that can be used by other players to join in externalurl.txt. It also provides a list of players that are currently online in logged-in.txt.

### Server config files
A server persists its configuration across multiple files containing lists of line-separated values. The server's main configuration can be set in server.cfg (see below for available options). Names of exiled players are stored in exile-list.txt, exiled IPs in exiled-ips.txt, and names of players that have been granted warden status in wardens.txt.

#### server.cfg options
The server.cfg configuration file contains a list of key=value pairs. Here's a list of all supported keys:
- `private` - when set to `true`, the externalurl.txt file doesn't get created
