# Cavern
Cavern is a simple voxel sandbox game written in C++ using OpenGL shaders.

<img width="856" height="512" alt="image" src="https://github.com/user-attachments/assets/2ac9e527-2284-44aa-8233-e5ce3356a279" />

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
