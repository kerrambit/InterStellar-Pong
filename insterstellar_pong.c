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
#include "errors.h"
#include "utils.h"

// --------------------------------------------------------------------------------------------- //

#define CANVAS_WIDTH 108
#define CANVAS_HEIGHT 22

// --------------------------------------------------------------------------------------------- //

static int load_main_page(bool display_terminal);
static struct termios init_termios();

// --------------------------------------------------------------------------------------------- //

int main() 
{   
    if (enable_terminal() == -1) {
        resolve_error(BROKEN_TERMINAL);
        return EXIT_FAILURE;
    }

    struct termios old_term = init_termios();

    if (load_main_page(true) == -1) {
        if (remove_terminal_data() == -1) {
            resolve_error(FAILURE_OF_REMOVING_FILE);
        }
        tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
        return EXIT_FAILURE;
    }

    int c;
    while ((c = getchar()) != EOF) {

        char *command = NULL;
        if (save_char(c, &command) == -1) {
            break;
        }

        if (STR_EQ(command, "q")) {
            break;
        }

        free(command);

        if (load_main_page(true) == -1) {
            break;
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);

    if (remove_terminal_data() == -1) {
        resolve_error(FAILURE_OF_REMOVING_FILE);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief 
 * 
 * @param display_terminal 
 * @note Minimal height of main page is 12 pixels.
 * @return int 
 */
static int load_main_page(bool display_terminal)
{
    const char* GAME_LOGO = ".___        __                _________ __         .__  .__                 __________                      \n"
                            "|   | _____/  |_  ___________/   _____//  |_  ____ |  | |  | _____ _______  \\______   \\____   ____    ____  \n"
                            "|   |/    \\   __\\/ __ \\_  __ \\_____  \\\\   __\\/ __ \\|  | |  | \\__  \\\\_  __ \\  |     ___/  _ \\ /    \\  / ___\\ \n"
                            "|   |   |  \\  | \\  ___/|  | \\/        \\|  | \\  ___/|  |_|  |__/ __ \\|  | \\/  |    |  (  <_> )   |  \\/ /_/  >\n"
                            "|___|___|  /__|  \\___  >__| /_______  /|__|  \\___  >____/____(____  /__|     |____|   \\____/|___|  /\\___  / \n"
                            "         \\/          \\/             \\/           \\/               \\/                             \\//_____/  "
                            "\n";

    clear_canvas();
    put_empty_row(1);
    put_text(GAME_LOGO, CANVAS_WIDTH, LEFT);
    put_empty_row(4);
    put_text("PLAY [P]\n", CANVAS_WIDTH, CENTER);
    put_text("ABOUT [A]\n", CANVAS_WIDTH, CENTER);
    put_text("QUIT [Q]\n", CANVAS_WIDTH, CENTER);
    put_empty_row(5);

    if (display_terminal) {
        if (render_terminal(CANVAS_WIDTH) == -1) {
            return -1;
        }
    }

    return 0;
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