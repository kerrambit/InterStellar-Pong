#include <stdio.h>

#include "page_loader.h"
#include "utils.h"
#include "draw.h"

// --------------------------------------------------------------------------------------------- //

static const char* GAME_LOGO = ".___        __                _________ __         .__  .__                 __________                      \n"
                               "|   | _____/  |_  ___________/   _____//  |_  ____ |  | |  | _____ _______  \\______   \\____   ____    ____  \n"
                               "|   |/    \\   __\\/ __ \\_  __ \\_____  \\\\   __\\/ __ \\|  | |  | \\__  \\\\_  __ \\  |     ___/  _ \\ /    \\  / ___\\ \n"
                               "|   |   |  \\  | \\  ___/|  | \\/        \\|  | \\  ___/|  |_|  |__/ __ \\|  | \\/  |    |  (  <_> )   |  \\/ /_/  >\n"
                               "|___|___|  /__|  \\___  >__| /_______  /|__|  \\___  >____/____(____  /__|     |____|   \\____/|___|  /\\___  / \n"
                               "         \\/          \\/             \\/           \\/               \\/                             \\//_____/  "
                               "\n";

// --------------------------------------------------------------------------------------------- //

static int load_main_page(px_t height, px_t width);
static int load_not_found_page(px_t height, px_t width);
static int load_pregame_setting_page(px_t height, px_t width);
static int load_game(px_t height, px_t width);

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

    default:
        return NO_PAGE;
    }
}

int load_page(page_t page, px_t height, px_t width)
{
    switch (page)
    {
    case MAIN_PAGE:
        return load_main_page(height, width);
    case QUIT_WITHOUT_CONFIRMATION_PAGE:
        return -1;
    case PREGAME_SETTING_PAGE:
        return load_pregame_setting_page(height, width);
    case GAME_PAGE:
        return load_game(height, width);
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
static int load_main_page(px_t height, px_t width)
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
        return -1;
    }
    
    return 0;
}

static int load_not_found_page(px_t height, px_t width)
{
    clear_canvas();
    put_empty_row(1);
    put_text(GAME_LOGO, width, LEFT);
    put_empty_row(4);
    put_text("Page not found.", width, CENTER);
    put_empty_row(2);

    return -1;
}

static int load_pregame_setting_page(px_t height, px_t width)
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
        return -1;
    }
    
    return 0;
}

static int load_game(px_t height, px_t width)
{
    clear_canvas();
    put_empty_row(4);
    put_text("Game started\n", width, CENTER);
    put_text("⬛■█▮\n", width, CENTER);

    return -1;
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
    default:
        break;
    }
}