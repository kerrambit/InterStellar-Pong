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

static int load_main_page(bool display_terminal);
static struct termios init_termios();

// --------------------------------------------------------------------------------------------- //

int main() 
{   
    int terminal_enable_return_code;
    if ((terminal_enable_return_code = enable_terminal()) == -1) {
        printf("[I/O ERROR]: unable to render terminal. Application had to be terminated.\n"); // TO-DO make new error handling system
        return EXIT_FAILURE;
    }

    struct termios old_term = init_termios();

    if (load_main_page(true) == -1) {
        remove_terminal_data();
        tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
        return EXIT_FAILURE;
    }

    int c;
    while ((c = getchar()) != EOF) {

        int save_char_return_code;
        if ((save_char_return_code = save_char(c)) == -1) {
            break;
        }

        if (c == 'q' || c == 'Q') {
            break;
        }

        if (load_main_page(true) == -1) {
            remove_terminal_data();
            return EXIT_FAILURE;
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);

    if (remove_terminal_data() == -1) {
        printf("[I/O Error]: removing the file 'user_input.data' failed.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static int load_main_page(bool display_terminal)
{
    const char* GAME_LOGO = ".___        __                _________ __         .__  .__                 __________                      \n"
                            "|   | _____/  |_  ___________/   _____//  |_  ____ |  | |  | _____ _______  \\______   \\____   ____    ____  \n"
                            "|   |/    \\   __\\/ __ \\_  __ \\_____  \\\\   __\\/ __ \\|  | |  | \\__  \\\\_  __ \\  |     ___/  _ \\ /    \\  / ___\\ \n"
                            "|   |   |  \\  | \\  ___/|  | \\/        \\|  | \\  ___/|  |_|  |__/ __ \\|  | \\/  |    |  (  <_> )   |  \\/ /_/  >\n"
                            "|___|___|  /__|  \\___  >__| /_______  /|__|  \\___  >____/____(____  /__|     |____|   \\____/|___|  /\\___  / \n"
                            "         \\/          \\/             \\/           \\/               \\/                             \\//_____/  "
                            "\n\n\n\n";

    clear_canvas();
    put_text(GAME_LOGO, CANVAS_WIDTH, LEFT);
    put_text("PLAY [P]\n", CANVAS_WIDTH, CENTER);
    put_text("ABOUT [A]\n", CANVAS_WIDTH, CENTER);
    put_text("QUIT [Q]\n", CANVAS_WIDTH, CENTER);
    put_empty_row(4);

    if (display_terminal) {
        if (render_terminal(CANVAS_WIDTH) == -1) {
            return -1;
        }
    }
}

static struct termios init_termios()
{
    struct termios old_term, new_term;
    tcgetattr(STDIN_FILENO, &old_term);

    new_term = old_term;
    new_term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);

    return old_term;
}