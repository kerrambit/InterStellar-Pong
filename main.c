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

    // set up data storage for page_loader functions
    page_loader_inner_data_t *data = create_page_loader_inner_data();
    if (data == NULL) {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
        show_cursor();
        return EXIT_FAILURE;
    }

    // try to load and render main page
    if (load_page(MAIN_PAGE, CANVAS_HEIGHT, CANVAS_WIDTH, data) == ERROR) {
        (void)remove_terminal_data();
        tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
        release_plage_loader_inner_data(data);
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
        data->terminal_signal = false;
        if (command != NULL) {
            new_page = find_page(current_page, command, data);
            if (new_page != NO_PAGE) {
                current_page = new_page;
            } else {
                data->terminal_signal = true;
            }
        }

        free(command);
        page_return_code_t load_page_return_code = load_page(current_page, CANVAS_HEIGHT, CANVAS_WIDTH, data);

        if (load_page_return_code == ERROR) {
            break;
        } else if (load_page_return_code == SUCCESS_GAME) {
            current_page = AFTER_GAME_PAGE;
            if (load_page(AFTER_GAME_PAGE, CANVAS_HEIGHT, CANVAS_WIDTH, data) == ERROR) {
                break;
            } 
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
    release_plage_loader_inner_data(data);

    if (remove_terminal_data() == -1) {
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