# InterStellar Pong - v1.1.0-beta 

Welcome to InterStellar Pong, a simple terminal-based game inspired by the classic Pong with an exciting twist!
In this game, you'll not only play Pong but also collect valuable resources like stone, iron, copper, and gold to progress through higher levels.

## Features

- Classic Pong gameplay with resource collection.
- Gather stone, iron, copper, and gold to advance.
- Simple page "engine" for account management and player creation.
- Custom library for page rendering and visuals.

*Note: The ability to gather stone, iron, copper, and gold will be introduced in future versions of the game. Stay tuned!*

## Installation

#### To play InterStellar Pong, follow these steps.

- Clone the repository:

   ```bash
   git clone https://github.com/kerrambit/InterStellar-Pong && cd InterStellar-Pong

- Compile (the compilation is guaranteed to run without problems for at least version gcc-11):

   ```bash
   gcc main.c draw.c errors.c page_loader.c terminal.c utils.c interstellar_pong.c player.c -o interstellar_pong

- Run:

   ```bash
   ./interstellar_pong

## Bug Fixes
- **v1.0.0-beta**
  - Fixed: Issue with game crashing after the user runs the game for the first time and chosses to play without creating player (error log: "[ERROR] - [Application Error]: file with data was not found.").
  - Description: The game was crashing due to the fact, that program tried to update data in players.data file, however, such a file did not exist at the moment.

## Version History
- **v1.1.0-beta**
  - This version introduces enhancements to the enemy paddle movement, resulting in smoother gameplay. Additionally, the game's difficulty has been adjusted to make it more beatable, as the enemy's strategy is no longer optimized. Another improvement involves the heart loss mechanism; when a heart is lost, the positions of the player, enemy, and ball are reset to their initial states, and the game is briefly paused. This release also addresses a bug related to pressing the Arrow Up/Down keys.