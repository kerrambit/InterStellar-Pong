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
#include "page_loader.h"
#include "terminal.h"

// --------------------------------------------------------------------------------------------- //

#define CANVAS_WIDTH 112
#define CANVAS_HEIGHT 22

// --------------------------------------------------------------------------------------------- //

static struct termios init_termios();

// --------------------------------------------------------------------------------------------- //

int main() 
{   
    hide_cursor();

    // open and enable terminal
    if (enable_terminal() == -1) {
        resolve_error(BROKEN_TERMINAL);
        show_cursor();
        return EXIT_FAILURE;
    }

    struct termios old_term = init_termios();

    // try to load and render main page
    if (load_page(MAIN_PAGE, CANVAS_HEIGHT, CANVAS_WIDTH, false) == ERROR) {
        if (remove_terminal_data() == -1) {
            resolve_error(FAILURE_OF_REMOVING_FILE);
        }
        tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
        show_cursor();
        return EXIT_FAILURE;
    }

    int c;
    page_t current_page = MAIN_PAGE;

    // program while loop
    while ((c = getchar()) != EOF) {

        char *command = NULL;
        if (process_command(c, &command) == -1) {
            free(command);
            break;
        }

        page_t new_page = NO_PAGE;
        bool unknown_command = false;
        if (command != NULL) {
            new_page = find_page(current_page, command);
            if (new_page != NO_PAGE) {
                current_page = new_page;
            } else {
                unknown_command = true;
            }
        }

        free(command);
        page_return_code_t load_page_return_code = load_page(current_page, CANVAS_HEIGHT, CANVAS_WIDTH, unknown_command);

        if (load_page_return_code == ERROR) {
            break;
        } else if (load_page_return_code == SUCCESS_GAME) {
            current_page = AFTER_GAME_PAGE;
            if (load_page(AFTER_GAME_PAGE, CANVAS_HEIGHT, CANVAS_WIDTH, unknown_command) == ERROR) {
                break;
            } 
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);

    if (remove_terminal_data() == -1) {
        resolve_error(FAILURE_OF_REMOVING_FILE);
        show_cursor();
        return EXIT_FAILURE;
    }

    put_empty_row(1);
    show_cursor();
    return EXIT_SUCCESS;
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