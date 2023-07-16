/**
 * @file interstellar_pong.c
 * @author Marek Eibel
 * @brief InterStellar Pong is a game (...).
 * @version 0.1
 * @date 2023-07-16
 * 
 * @copyright Copyright (c) 2023
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "draw.h"

// --------------------------------------------------------------------------------------------- //

#define CANVAS_WIDTH 108

void load_main_page(bool enable_terminal);

int main() 
{
    load_main_page(true);
    return 0;
}

void load_main_page(bool enable_terminal)
{
    const char* GAME_LOGO = ".___        __                _________ __         .__  .__                 __________                      \n"
                      "|   | _____/  |_  ___________/   _____//  |_  ____ |  | |  | _____ _______  \\______   \\____   ____    ____  \n"
                      "|   |/    \\   __\\/ __ \\_  __ \\_____  \\\\   __\\/ __ \\|  | |  | \\__  \\\\_  __ \\  |     ___/  _ \\ /    \\  / ___\\ \n"
                      "|   |   |  \\  | \\  ___/|  | \\/        \\|  | \\  ___/|  |_|  |__/ __ \\|  | \\/  |    |  (  <_> )   |  \\/ /_/  >\n"
                      "|___|___|  /__|  \\___  >__| /_______  /|__|  \\___  >____/____(____  /__|     |____|   \\____/|___|  /\\___  / \n"
                      "         \\/          \\/             \\/           \\/               \\/                             \\//_____/  "
                      "\n\n\n\n";

    clear_canvas();
    put_text(GAME_LOGO, 0, LEFT);
    put_text("PLAY [P]\n", CANVAS_WIDTH, CENTER);
    put_text("ABOUT [A]\n", CANVAS_WIDTH, CENTER);
    put_text("QUIT [Q]\n", CANVAS_WIDTH, CENTER);
    put_space(4u);

    if (enable_terminal) {
        render_terminal(CANVAS_WIDTH);
    }
}
