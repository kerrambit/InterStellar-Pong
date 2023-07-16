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
#include <termios.h>
#include <unistd.h>

#include "draw.h"

// --------------------------------------------------------------------------------------------- //

#define CANVAS_WIDTH 108

// --------------------------------------------------------------------------------------------- //

void load_main_page(bool enable_terminal);

// --------------------------------------------------------------------------------------------- //

int main() 
{   
    int terminal_enable_return_code;
    if ((terminal_enable_return_code = enable_terminal()) == -1) {
        printf("[I/O ERROR]: Unable to render terminal. Application had to be terminated.\n"); // TO-DO make new error handling system
        return EXIT_FAILURE;
    }


    load_main_page(true);
    remove_terminal_data();
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

    struct termios old_term, new_term;
    tcgetattr(STDIN_FILENO, &old_term);
    new_term = old_term;
    new_term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);

    int c;
    while ((c = getchar()) != EOF) {

        int save_char_return_code;
        if ((save_char_return_code = save_char(c)) == -1) {
            break;
        }

        if (c == 'q' || c == 'Q') {
            break;
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
}
