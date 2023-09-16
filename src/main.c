/**
 * @file interstellar_pong.c
 * @author Marek Eibel
 * @brief InterStellar Pong - A terminal-based resource-collecting Pong game.
 * 
 * InterStellar Pong is a simple terminal-based game inspired by the classic Pong
 * with an exciting twist! In this game, players not only play the traditional Pong,
 * but also collect valuable resources such as stone, iron, copper, and gold. These
 * resources are crucial for progressing through higher levels, creating an engaging
 * and unique gameplay experience.
 * 
 * @version 0.1
 * @date 2023-07-16
 * 
 * @copyright Copyright (c) 2023
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "termify/draw.h"
#include "termify/log.h"
#include "termify/utils.h"
#include "termify/page_loader.h"
#include "termify/terminal.h"

// ---------------------------------------- MACROS --------------------------------------------- //

#define WINDOW_WIDTH 112
#define WINDOW_HEIGHT 22

// ----------------------------------------- PROGRAM-------------------------------------------- //

/**
 * @brief The main function for the InterStellar Pong game.
 *
 * This function is the entry point for the InterStellar Pong game. It initializes
 * the game environment, handles user input, loads and renders pages, and manages
 * the program loop until the user exits the game. It ensures that the terminal
 * attributes are set correctly, and that the game environment is cleaned up
 * properly before exiting.
 *
 * @return Returns EXIT_SUCCESS if the program runs successfully, or EXIT_FAILURE on errors.
 */
int main() 
{   
    hide_cursor();

    // enable terminal
    terminal_data_t *terminal_data;
    if ((terminal_data = enable_terminal("Enter your commands.", TERMINAL_LOG, "Unknown command.", TERMINAL_WARNING)) == NULL) {
        resolve_error(BROKEN_TERMINAL);
        show_cursor();
        return EXIT_FAILURE;
    }

    // create data holder for page loader
    page_loader_inner_data_t *page_loader_data = create_page_loader_inner_data();
    if (page_loader_data == NULL) {
        (void)close_terminal(terminal_data);
        show_cursor();
        return EXIT_FAILURE;
    }

    // try to load and render main page
    if (load_page(MAIN_PAGE, WINDOW_HEIGHT, WINDOW_WIDTH, page_loader_data, terminal_data) == ERROR) {
        (void)close_terminal(terminal_data);
        release_page_loader_inner_data(page_loader_data);
        show_cursor();
        return EXIT_FAILURE;
    }

    int c;
    page_t current_page = MAIN_PAGE;

    // program main loop
    while ((c = getchar()) != EOF) {

        char *command = NULL;
        if (process_command(terminal_data, c, &command) == -1) {
            free(command);
            break;
        }

        page_t new_page = NO_PAGE;
        page_loader_data->terminal_signal = false;
        if (command != NULL) {
            new_page = find_page(current_page, command, page_loader_data);
            if (new_page != NO_PAGE) {
                current_page = new_page;
            } else {
                page_loader_data->terminal_signal = true;
            }
        }

        free(command);
        page_return_code_t load_page_return_code = load_page(current_page, WINDOW_HEIGHT, WINDOW_WIDTH, page_loader_data, terminal_data);

        if (load_page_return_code == ERROR) {
            break;
        } else if (load_page_return_code == SUCCESS_GAME) {
            current_page = AFTER_GAME_PAGE;
            if (load_page(AFTER_GAME_PAGE, WINDOW_HEIGHT, WINDOW_WIDTH, page_loader_data, terminal_data) == ERROR) {
                break;
            } 
        }
    }

    release_page_loader_inner_data(page_loader_data);

    if (close_terminal(terminal_data) == -1) {
        show_cursor();
        return EXIT_FAILURE;
    }

    put_empty_row(1);
    show_cursor();
    return EXIT_SUCCESS;
}
