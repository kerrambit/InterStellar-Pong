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
#include <termios.h>
#include <unistd.h>

#include "draw.h"
#include "errors.h"
#include "utils.h"
#include "page_loader.h"
#include "terminal.h"

// ---------------------------------------- MACROS --------------------------------------------- //

#define WINDOW_WIDTH 112
#define WINDOW_HEIGHT 22

// ---------------------------------- STATIC DECLARATIONS--------------------------------------- //

static struct termios init_termios();
static void restore_terminal_attributes(const struct termios *original_termios);

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

    // open and enable terminal
    terminal_data_t *terminal_data;
    if ((terminal_data = enable_terminal("Enter your commands.", TERMINAL_LOG, "Unknown command.", TERMINAL_WARNING)) == NULL) {
        resolve_error(BROKEN_TERMINAL);
        show_cursor();
        return EXIT_FAILURE;
    }

    struct termios old_term = init_termios();

    // create data holder for page loader
    page_loader_inner_data_t *data = create_page_loader_inner_data();
    if (data == NULL) {
        restore_terminal_attributes(&old_term);
        (void)close_terminal(terminal_data);
        show_cursor();
        return EXIT_FAILURE;
    }

    // try to load and render main page
    if (load_page(MAIN_PAGE, WINDOW_HEIGHT, WINDOW_WIDTH, data, terminal_data) == ERROR) {
        (void)close_terminal(terminal_data);
        restore_terminal_attributes(&old_term);
        release_page_loader_inner_data(data);
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
        page_return_code_t load_page_return_code = load_page(current_page, WINDOW_HEIGHT, WINDOW_WIDTH, data, terminal_data);

        if (load_page_return_code == ERROR) {
            break;
        } else if (load_page_return_code == SUCCESS_GAME) {
            current_page = AFTER_GAME_PAGE;
            if (load_page(AFTER_GAME_PAGE, WINDOW_HEIGHT, WINDOW_WIDTH, data, terminal_data) == ERROR) {
                break;
            } 
        }
    }

    restore_terminal_attributes(&old_term);
    release_page_loader_inner_data(data);

    if (close_terminal(terminal_data) == -1) {
        show_cursor();
        return EXIT_FAILURE;
    }

    put_empty_row(1);
    show_cursor();
    return EXIT_SUCCESS;
}

/**
 * @brief Initializes the terminal settings for interactive input.
 *
 * This function sets up the terminal settings for interactive input, disabling
 * canonical mode and echoing. It returns the original terminal settings which
 * can be restored later.
 *
 * @return The original struct termios settings before modification.
 */
static struct termios init_termios()
{
    struct termios old_term, new_term;
    tcgetattr(STDIN_FILENO, &old_term);

    new_term = old_term;
    new_term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);

    return old_term;
}

/**
 * @brief Restores terminal attributes for a file descriptor.
 *
 * This function is used to restore terminal attributes for a specified
 * file descriptor. It takes a pointer to the termios structure containing
 * the original attributes and applies them to the terminal using the
 * tcsetattr() command with the TCSANOW flag.
 *
 * @param original_termios A pointer to the termios structure containing the original attributes.
 */
static void restore_terminal_attributes(const struct termios *original_termios)
{
    tcsetattr(STDIN_FILENO, TCSANOW, original_termios);
}