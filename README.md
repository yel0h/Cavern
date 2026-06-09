# Cavern
Cavern is a simple voxel sandbox game with multiplayer support written in C++ using OpenGL shaders.

<img width="856" height="512" alt="image" src="https://github.com/user-attachments/assets/f8e56c82-501f-4ac2-b4d2-0b2aea43fb05" />

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

## Technical Details
The current world block data is saved in the world.dat file, either manually (see Controls) or automatically upon exiting the game. Additional world saves can be created in 5 slots, each of which exists as a save_\<id\>.dat file. Wanderers are saved in wanderers.dat, while the player's spawnpoint is stored in spawn.dat.
