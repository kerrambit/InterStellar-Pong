   .___        __                _________ __         .__  .__                 __________                      
  |   | _____/  |_  ___________/   _____//  |_  ____ |  | |  | _____ _______  \______   \____   ____    ____   
  |   |/    \   __\/ __ \_  __ \_____  \\   __\/ __ \|  | |  | \__  \\_  __ \  |     ___/  _ \ /    \  / ___\  
  |   |   |  \  | \  ___/|  | \/        \|  | \  ___/|  |_|  |__/ __ \|  | \/  |    |  (  <_> )   |  \/ /_/  > 
  |___|___|  /__|  \___  >__| /_______  /|__|  \___  >____/____(____  /__|     |____|   \____/|___|  /\___  /  
           \/          \/             \/           \/               \/                             \//_____/ 

# InterStellar Pong - Beta Version 0.1

Welcome to InterStellar Pong, a simple terminal-based game inspired by the classic Pong with an exciting twist!
In this game, you'll not only play Pong but also collect valuable resources like stone, iron, copper, and gold to progress through higher levels.

## Features

- Classic Pong gameplay with resource collection.
- Gather stone, iron, copper, and gold to advance.
- Simple page "engine" for account management and player creation.
- Custom library for page rendering and visuals.

## Installation

To play InterStellar Pong, follow these steps:

1. Clone the repository:
   ```bash
   git clone https://github.com/your-username/interstellar-pong.git
   cd interstellar-pong
   gcc main.c draw.c errors.c page_loader.c terminal.c utils.c interstellar_pong.c player.c -o interstellar_pong
   ./interstellar_pong

