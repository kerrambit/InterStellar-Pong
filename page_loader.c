#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "page_loader.h"
#include "utils.h"
#include "draw.h"
#include "errors.h"
#include "terminal.h"

// --------------------------------------------------------------------------------------------- //

static const char* GAME_LOGO = ".___        __                _________ __         .__  .__                 __________                      \n"
                               "|   | _____/  |_  ___________/   _____//  |_  ____ |  | |  | _____ _______  \\______   \\____   ____    ____  \n"
                               "|   |/    \\   __\\/ __ \\_  __ \\_____  \\\\   __\\/ __ \\|  | |  | \\__  \\\\_  __ \\  |     ___/  _ \\ /    \\  / ___\\ \n"
                               "|   |   |  \\  | \\  ___/|  | \\/        \\|  | \\  ___/|  |_|  |__/ __ \\|  | \\/  |    |  (  <_> )   |  \\/ /_/  >\n"
                               "|___|___|  /__|  \\___  >__| /_______  /|__|  \\___  >____/____(____  /__|     |____|   \\____/|___|  /\\___  / \n"
                               "         \\/          \\/             \\/           \\/               \\/                             \\//_____/  "
                               "\n";

// --------------------------------------------------------------------------------------------- //

static page_return_code_t load_main_page(px_t height, px_t width);
static page_return_code_t load_not_found_page(px_t height, px_t width);
static page_return_code_t load_pregame_setting_page(px_t height, px_t width);
static page_return_code_t load_game(px_t height, px_t width);
static page_return_code_t load_after_game_page(px_t height, px_t width);

// --------------------------------------------------------------------------------------------- //

page_t find_page(page_t current_page, const char *command)
{
    switch (current_page)
    {
    case MAIN_PAGE:
        if (STR_EQ(command, "q") || STR_EQ(command, "Q") || STR_EQ(command, "quit") || STR_EQ(command, "QUIT")) {
            return QUIT_WITHOUT_CONFIRMATION_PAGE;
        } else if (STR_EQ(command, "a") || STR_EQ(command, "A") || STR_EQ(command, "about") || STR_EQ(command, "ABOUT")) {
            return ABOUT_PAGE;
        } else if (STR_EQ(command, "p") || STR_EQ(command, "P") || STR_EQ(command, "play") || STR_EQ(command, "PLAY")) {
            return PREGAME_SETTING_PAGE;
        }

    case PREGAME_SETTING_PAGE:
        if (STR_EQ(command, "s") || STR_EQ(command, "S") || STR_EQ(command, "start") || STR_EQ(command, "START")) {
            return GAME_PAGE;
        } else if (STR_EQ(command, "b") || STR_EQ(command, "B") || STR_EQ(command, "back") || STR_EQ(command, "BACK")) {
            return MAIN_PAGE;
        }

    case AFTER_GAME_PAGE:
        if (STR_EQ(command, "n") || STR_EQ(command, "N") || STR_EQ(command, "new game") || STR_EQ(command, "NEW GAME")) {
            return PREGAME_SETTING_PAGE;
        } else if (STR_EQ(command, "q") || STR_EQ(command, "Q") || STR_EQ(command, "quit") || STR_EQ(command, "QUIT")) {
            return QUIT_WITHOUT_CONFIRMATION_PAGE;
        }

    default:
        return NO_PAGE;
    }
}

page_return_code_t load_page(page_t page, px_t height, px_t width)
{
    switch (page)
    {
    case MAIN_PAGE:
        return load_main_page(height, width);
    case QUIT_WITHOUT_CONFIRMATION_PAGE:
        return ERROR;
    case PREGAME_SETTING_PAGE:
        return load_pregame_setting_page(height, width);
    case GAME_PAGE:
        return load_game(height, width);
    case AFTER_GAME_PAGE:
        return load_after_game_page(height, width);
    default:
        return load_not_found_page(height, width);
    }
}

/**
 * @brief 
 * 
 * @param display_terminal 
 * @note Minimal height of main page is 12 pixels.
 * @return int 
 */
static page_return_code_t load_main_page(px_t height, px_t width)
{
    clear_canvas();
    put_empty_row(1);
    put_text(GAME_LOGO, width, LEFT);
    put_empty_row(4);
    put_text("PLAY [P]\n", width, CENTER);
    put_text("ABOUT [A]\n", width, CENTER);
    put_text("QUIT [Q]\n", width, CENTER);
    put_empty_row(5);

    if (render_terminal(width) == -1) {
        return ERROR;
    }
    
    return SUCCES;
}

static page_return_code_t load_not_found_page(px_t height, px_t width)
{
    clear_canvas();
    put_empty_row(1);
    put_text(GAME_LOGO, width, LEFT);
    put_empty_row(4);
    put_text("Page not found.", width, CENTER);
    put_empty_row(2);

    return ERROR;
}

static page_return_code_t load_pregame_setting_page(px_t height, px_t width)
{
    clear_canvas();
    put_empty_row(1);
    put_text(GAME_LOGO, width, LEFT);
    put_empty_row(4);
    put_text("[TODO] In working process...\n", width, CENTER);
    put_empty_row(1);
    put_text("START GAME [S]\n", width, CENTER);
    put_text("BACK [B]\n", width, CENTER);
    put_empty_row(5);

    if (render_terminal(width) == -1) {
        return ERROR;
    }
    
    return SUCCES;
}

static page_return_code_t load_after_game_page(px_t height, px_t width)
{
    clear_canvas();
    put_empty_row(1);
    put_text(GAME_LOGO, width, LEFT);
    put_empty_row(4);
    put_text("[TODO] Game statistics... in preparetion\n", width, CENTER);
    put_empty_row(1);
    put_text("NEW GAME [N]\n", width, CENTER);
    put_text("QUIT [Q]\n", width, CENTER);
    put_empty_row(5);

    if (render_terminal(width) == -1) {
        return ERROR;
    }
    
    return SUCCES;
}

static page_return_code_t load_game(px_t height, px_t width)
{
    // ⬛■█▮

    pixel_buffer_t *pixel_buffer = create_pixel_buffer(height, width);
    if (pixel_buffer == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        return ERROR;
    }

    put_empty_row(2);

    bool game_running = true;
    while (game_running) {

        clear_canvas();
        render_graphics(pixel_buffer);

        // Setup the file descriptor set for select
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds); // Add standard input (keyboard) to the set

        // Set the timeout for select (0 seconds, 0 microseconds for non-blocking behavior)
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;

        // Call select to check for input readiness
        int ready_fds = select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &timeout);

        if (ready_fds > 0) {
            // User input is ready to be read
            int c = getchar(); // Read the user input

            // Process the user input
            if (c == 'w' || c == 'W') {
                pixel_buffer->buff[0] = 1;
            } else if (c == 's' || c == 'S') {
                pixel_buffer->buff[0] = 0;
            } else if (c == 'q' || c == 'Q') {
                game_running = false;
            }
        }

        usleep(20);
    }

    release_pixel_buffer(pixel_buffer);
    return SUCCESS_GAME;
}

const char *convert_page_2_string(page_t page)
{
    switch (page)
    {
    case NO_PAGE: return "No page";
    case MAIN_PAGE: return "Main page";
    case QUIT_WITHOUT_CONFIRMATION_PAGE: return "Quit without confirmation page";
    case ABOUT_PAGE: return "About page";
    case PREGAME_SETTING_PAGE: return "Pregame setting page";
    case NOT_FOUND_PAGE: return "Not found page";
    case GAME_PAGE: return "Game page";
    case BACK_PAGE: return "Back page";
    case AFTER_GAME_PAGE: return "After game page";
    default:
        break;
    }
}