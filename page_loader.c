#include <stdarg.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

#include "page_loader.h"
#include "utils.h"
#include "draw.h"
#include "errors.h"
#include "terminal.h"
#include "interstellar_pong.h"
#include "player.h"

// --------------------------------------------------------------------------------------------- //

#define COMMAND_EQ(command, ch, CH, word, WORD) (STR_EQ(command, ch) || STR_EQ(command, CH) || STR_EQ(command, word) || STR_EQ(command, WORD))
#define GAME_WIDTH 80
#define PLAYERS_DATA_PATH "players.data"

// --------------------------------------------------------------------------------------------- //

// ---------------------------------------- GLOBAL VARIABLES ----------------------------------- //

/**
 * @brief This global variable is read by load_choose_player_page() & find_page().
 *        The value is being changed only in find_page() - when the user views the page while selecting a player,
 *        the player pages are loaded sequentially and the index of the currently viewed page must be saved.
 */
//int gl_curr_players_page_index = 0;

/**
 * @brief This global variable is read by find_page().
 *        The value is set and changed only by choose_pregame_page() which is a function that runs with the load_main_page or
 *        load_after_game_page to retrieve player data from a file. In doing so it sets/updates the number of players
 *        in the file (some may have been added). This means that the number in the variable will always be up to date,
 *        cases of changing the file externally are undefined behavior and thus not checked.
 */
//int gl_players_count = 0;

/**
 * @brief This global variable is read/set/updated/freed by find_page(), check_name(), is_name_unique() and load_create_new_player_page().
 *        Its job is to hold the current value of the name when creating a new player. The name must be checked for validity and uniqueness.
 *        If it is ok, it does not mean it will be used and can possibly be freed when leaving the page or replacing it with another name.
 *        Ultimately it should be stored within the player structure and released.
 */
//char *gl_curr_player_name = NULL;

/**
 * @brief This global variable is read and set in find_page() and load_create_new_player_page().
 *        Used in the load_create_new_player_page() function for proper terminal setup.
 *        If the user enters a name when creating a new player and validity/uniqueness is checked, the answer to these two tests
 *        is passed to the user through the terminal. And to let the function know that it only needs to show this message once,
 *        this flag is used. 
 */
//bool gl_curr_player_name_seen_flag = false;

/**
 * @brief This global variable is set in find_page() and freed in load_game().
 *        It is used to pass the player struct between pages.
 */
//player_t *gl_player_choosen_to_game = NULL;

// ---------------------------------------- STATIC DECLARATIONS--------------------------------- //

static page_return_code_t load_main_page(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data);
static page_return_code_t load_pre_create_new_player_page(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data);
static page_return_code_t load_choose_player_page(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data);
static page_return_code_t load_create_new_player_page(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data);
static page_return_code_t load_game(px_t height, px_t width, page_loader_inner_data_t *data);
static page_return_code_t load_after_game_page(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data);

static page_return_code_t load_not_found_page(px_t height, px_t width);
static page_return_code_t load_error_page(px_t height, px_t width);
static page_return_code_t load_quit_or_back_with_confirmation(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data);

static page_return_code_t handle_stats_for_no_player(px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data);
static players_array_t *load_players(const char* file_path);
static page_t choose_pregame_page(page_loader_inner_data_t *data);
static player_t *find_player(const char *name, const char *file_path);
static bool is_name_unique(const char *name, page_loader_inner_data_t *data);
static bool check_name(const char *name, page_loader_inner_data_t *data);
static page_t handle_save_and_play(page_loader_inner_data_t *data);
static int write_player_to_file(player_t *player, const char *file_path);
static char *create_string(const char *format, ...);
static char *create_player_string(player_t *player, bool end_with_newline);
static int update_players_stats(player_t *target_player, const char *file_path);
static void display_live_stats(game_t *game);
static void display_resources(player_t *player, int width);
static void display_hearts(const char *player_name, int number_of_hearts);

static void put_player(px_t width, px_t button_width, px_t button_height, player_t *player, bool last, px_t row_margin);
static void put_game_logo(px_t width, position_t position);

// --------------------------------------------------------------------------------------------- //

page_t find_page(page_t current_page, const char *command, page_loader_inner_data_t *data)
{
    switch (current_page)
    {
    case MAIN_PAGE:
        if (COMMAND_EQ(command, "q", "Q", "quit", "QUIT")) {
            return QUIT_WITHOUT_CONFIRMATION_PAGE;
        } else if (COMMAND_EQ(command, "a", "A", "about", "ABOUT")) {
            return ABOUT_PAGE;
        } else if (COMMAND_EQ(command, "p", "P", "play", "PLAY")) {
            return choose_pregame_page(data);
        }
        return NO_PAGE;
    // ----------------------------------------------------------------------------------------- //
    case PREGAME_SETTING_PAGE:
        if (COMMAND_EQ(command, "c", "C", "create player", "CREATE PLAYER")) {
            return GAME_PAGE;
        } else if (COMMAND_EQ(command, "b", "B", "back", "BACK")) {
            return MAIN_PAGE;
        } else if (COMMAND_EQ(command, "q", "Q", "quit", "QUIT")) {
            return QUIT_WITHOUT_CONFIRMATION_PAGE;
        }
        return NO_PAGE;
    // ----------------------------------------------------------------------------------------- //
    case AFTER_GAME_PAGE:
        if (COMMAND_EQ(command, "n", "N", "new game", "NEW GAME")) {
            release_player(data->player_choosen_to_game);
            data->player_choosen_to_game = NULL;
            return choose_pregame_page(data);
        } else if (COMMAND_EQ(command, "q", "Q", "quit", "QUIT")) {
            release_player(data->player_choosen_to_game);
            data->player_choosen_to_game = NULL;
            return QUIT_WITHOUT_CONFIRMATION_PAGE;
        }
        return NO_PAGE;
    // ----------------------------------------------------------------------------------------- //
    case PRE_CREATE_NEW_PLAYER_PAGE:
        if (COMMAND_EQ(command, "c", "C", "create player", "CREATE PLAYER")) {
            return CREATE_NEW_PLAYER_PAGE;
        } else if (COMMAND_EQ(command, "b", "B", "back", "BACK")) {
            return MAIN_PAGE;
        } else if (COMMAND_EQ(command, "q", "Q", "quit", "QUIT")) {
            return QUIT_WITHOUT_CONFIRMATION_PAGE;
        }
        return NO_PAGE;
    // ----------------------------------------------------------------------------------------- //
    case CHOOSE_PLAYER_PAGE:
        if (COMMAND_EQ(command, "b", "B", "back", "BACK")) {
            if (data->curr_players_page_index == 0) {
                return MAIN_PAGE;
            } else {
                data->curr_players_page_index--;
                return CHOOSE_PLAYER_PAGE;
            }
        } else if (COMMAND_EQ(command, "q", "Q", "quit", "QUIT")) {
            return QUIT_WITHOUT_CONFIRMATION_PAGE;
        } else if (COMMAND_EQ(command, "c", "C", "create player", "CREATE PLAYER")) {
            return CREATE_NEW_PLAYER_PAGE;
        } else if (COMMAND_EQ(command, "n", "N", "next", "NEXT")) {
            if (data->players_count - ((data->curr_players_page_index + 1) * 3) > 0) {
                data->curr_players_page_index++;
            }
            return CHOOSE_PLAYER_PAGE;
        } else {
            if((data->player_choosen_to_game = find_player(command, PLAYERS_DATA_PATH)) != NULL) {
                return GAME_PAGE;
            }
        }
        return NO_PAGE;
    // ----------------------------------------------------------------------------------------- //
    case CREATE_NEW_PLAYER_PAGE:
        if (COMMAND_EQ(command, "q", "Q", "quit", "QUIT") && data->curr_player_name == NULL) {
            return QUIT_WITHOUT_CONFIRMATION_PAGE;
        } else if (COMMAND_EQ(command, "q", "Q", "quit", "QUIT") && data->curr_player_name != NULL) {
            return QUIT_WITH_CONFIRMATION_FROM_CREATE_NEW_PLAYER_PAGE_PAGE;    
        } else if (COMMAND_EQ(command, "b", "B", "back", "BACK") && data->curr_player_name == NULL) {
            return choose_pregame_page(data);
        } else if (COMMAND_EQ(command, "b", "B", "back", "BACK") && data->curr_player_name != NULL) {
            return BACK_WITH_CONFIRMATION_FROM_CREATE_NEW_PLAYER_PAGE_PAGE;
        } else if (COMMAND_EQ(command, "w", "W", "play without creating player", "PLAY WITHOUT CREATING PLAYER")) {
            free(data->curr_player_name); data->curr_player_name = NULL; data->player_choosen_to_game = NULL;
            return GAME_PAGE;
        } else if (COMMAND_EQ(command, "s", "S", "save and play", "SAVE AND PLAY")) {
            return handle_save_and_play(data);
        } else {
            data->curr_player_name_seen_flag = false;
            if(!check_name(command, data)) {
                data->curr_player_name = INVALID_NAME;
                return CREATE_NEW_PLAYER_PAGE;
            }
            if (!is_name_unique(command, data)) {
                data->curr_player_name = NOT_UNIQUE_NAME;
            }
            return CREATE_NEW_PLAYER_PAGE;
        }
    // ----------------------------------------------------------------------------------------- //
    case BACK_WITH_CONFIRMATION_FROM_CREATE_NEW_PLAYER_PAGE_PAGE:
        if (COMMAND_EQ(command, "y", "Y", "yes", "YES")) {
            free(data->curr_player_name); data->curr_player_name = NULL;
            if (data->players_count == 0) {
                return PRE_CREATE_NEW_PLAYER_PAGE;
            } else {
                return CHOOSE_PLAYER_PAGE;
            }
        } else if (COMMAND_EQ(command, "n", "N", "no", "NO")) {
            return CREATE_NEW_PLAYER_PAGE;
        }
    // ----------------------------------------------------------------------------------------- //
    case QUIT_WITH_CONFIRMATION_FROM_CREATE_NEW_PLAYER_PAGE_PAGE:
        if (COMMAND_EQ(command, "y", "Y", "yes", "YES")) {
            free(data->curr_player_name); data->curr_player_name = NULL;
            return QUIT_WITHOUT_CONFIRMATION_PAGE;
        } else if (COMMAND_EQ(command, "n", "N", "no", "NO")) {
            return CREATE_NEW_PLAYER_PAGE;
        }
    // ----------------------------------------------------------------------------------------- //
    default:
        return NO_PAGE;
    }
}

page_return_code_t load_page(page_t page, px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data)
{
    switch (page)
    {
    case MAIN_PAGE:
        return load_main_page(height, width, data, terminal_data);
    case QUIT_WITHOUT_CONFIRMATION_PAGE:
        return ERROR;
    case GAME_PAGE:
        return load_game(height, GAME_WIDTH, data);
    case AFTER_GAME_PAGE:
        return load_after_game_page(height, width, data, terminal_data);
    case PRE_CREATE_NEW_PLAYER_PAGE:
        return load_pre_create_new_player_page(height, width, data, terminal_data);
    case CHOOSE_PLAYER_PAGE:
        return load_choose_player_page(height, width, data, terminal_data);
    case ERROR_PAGE:
        return load_error_page(height, width);
    case CREATE_NEW_PLAYER_PAGE:
        return load_create_new_player_page(height, width, data, terminal_data);
    case BACK_WITH_CONFIRMATION_FROM_CREATE_NEW_PLAYER_PAGE_PAGE:
        return load_quit_or_back_with_confirmation(height, width, data, terminal_data);
    case QUIT_WITH_CONFIRMATION_FROM_CREATE_NEW_PLAYER_PAGE_PAGE:
        return load_quit_or_back_with_confirmation(height, width, data, terminal_data);
    default:
        return load_not_found_page(height, width);
    }
}

const char *convert_page_2_string(page_t page)
{
    switch (page)
    {
    case NO_PAGE: return "no page";
    case MAIN_PAGE: return "Main page";
    case QUIT_WITHOUT_CONFIRMATION_PAGE: return "Quit Without Confirmation page";
    case ABOUT_PAGE: return "About page";
    case PREGAME_SETTING_PAGE: return "Pregame Setting page";
    case PRE_CREATE_NEW_PLAYER_PAGE: return "Pre-create New Player Page";
    case CREATE_NEW_PLAYER_PAGE: return "Create New Player page";
    case CHOOSE_PLAYER_PAGE: return "Choose Player page";
    case NOT_FOUND_PAGE: return "Not Found page";
    case GAME_PAGE: return "Game page";
    case BACK_PAGE: return "Back page";
    case AFTER_GAME_PAGE: return "After Game page";
    case ERROR_PAGE: return "Error page";
    default:
        break;
    }
}

/**
 * @brief 
 * 
 * @param display_terminal 
 * @note Minimal height of main page is 12 pixels.
 * @return int 
 */
static page_return_code_t load_main_page(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data)
{
    clear_canvas();
    draw_borders(height, width);
    set_cursor_at_beginning_of_canvas();

    put_empty_row(1);
    put_game_logo(width, CENTER);
    put_empty_row(3);
    put_text("PLAY [P]", width, CENTER);
    put_text("ABOUT [A]", width, CENTER);
    put_text("QUIT [Q]", width, CENTER);
    put_empty_row(5);

    if (render_terminal(terminal_data, width, data->terminal_signal, NULL, TERMINAL_N_A) == -1) {
        return ERROR;
    }

    return SUCCES;
}

static page_return_code_t load_quit_or_back_with_confirmation(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data)
{
    clear_canvas();
    draw_borders(height, width);
    set_cursor_at_beginning_of_canvas();

    put_empty_row(1);
    put_game_logo(width, CENTER);
    put_empty_row(3);
    put_text("Do you want to leave your unsaved work?", width, CENTER);
    put_text("YES [Y]", width, CENTER);
    put_text("NO [N]", width, CENTER);
    put_empty_row(5);

    if (render_terminal(terminal_data, width, data->terminal_signal, NULL, TERMINAL_N_A) == -1) {
        return ERROR;
    }

    return SUCCES;
}

static page_return_code_t load_not_found_page(px_t height, px_t width)
{
    clear_canvas();
    draw_borders(height, width);
    set_cursor_at_beginning_of_canvas();

    put_empty_row(1);
    put_game_logo(width, CENTER);
    put_empty_row(5);
    put_text("Page was not found.", width, CENTER);
    put_empty_row(9);

    return ERROR;
}

static page_return_code_t load_error_page(px_t height, px_t width)
{
    clear_canvas();
    draw_borders(height, width);
    set_cursor_at_beginning_of_canvas();

    put_empty_row(1);
    put_game_logo(width, CENTER);
    put_empty_row(5);
    put_text("Ooopsie, something bad happened ):", width, CENTER);
    put_text("[TODO] Check error logs file.", width, CENTER);
    put_empty_row(8);

    return ERROR;
}

static players_array_t *load_players(const char* file_path)
{
    players_array_t *players = create_players_array();
    if (players == NULL) {
        return NULL;
    }

    if (access(file_path, F_OK) != 0) {
        return players;
    }

    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        resolve_error(UNOPENABLE_FILE);
        return NULL;
    }

    char* line = NULL;
    size_t line_length = 0;
    ssize_t bytes_read;

    while ((bytes_read = getline(&line, &line_length, file)) != -1) {

        if (STR_EQ(line, "\n")) {
            continue;
        }

        player_t *player = create_player_from_string(line);
        if (player == NULL) {
            release_players_array(players);
            free(line);
            fclose(file);
            return NULL;
        }

        if (add_to_players_array(players, player) == NULL) {
            resolve_error(MEM_ALOC_FAILURE);
            release_players_array(players);
            free(line);
            fclose(file);
            return NULL;
        }
    }

    free(line);
    fclose(file);
    return players;
}

static char *create_player_string(player_t *player, bool end_with_newline)
{
    char *string;
    if (end_with_newline) {
        string = create_string("%s;%d;%d;%d;%d;%d\n", player->name, player->level, player->stone, player->copper, player->iron, player->gold);
    } else {
        string = create_string("\n%s;%d;%d;%d;%d;%d", player->name, player->level, player->stone, player->copper, player->iron, player->gold);
    }

    if (string == NULL) {
        return NULL;
    }

    return string;
}

static int write_player_to_file(player_t *player, const char *file_path)
{
    FILE* file = fopen(file_path, "a");
    if (file == NULL) {
        return -1;
    }

    char *result = create_player_string(player, false);
    if (result == NULL) {
        fclose(file);
        return -1;
    }

    fputs(result, file);

    free(result);
    fclose(file);
    return 0;
}

static page_t choose_pregame_page(page_loader_inner_data_t *data)
{
    players_array_t *players = load_players(PLAYERS_DATA_PATH);
    if (players == NULL) {
        return ERROR_PAGE;
    }

    int players_count = players->count;
    release_players_array(players);
    data->players_count = players_count;

    if (players_count > 0) {
        return CHOOSE_PLAYER_PAGE;
    } else {
        return PRE_CREATE_NEW_PLAYER_PAGE;
    }
}

static player_t *find_player(const char *name, const char *file_path)
{
    players_array_t *players = load_players(file_path);
    if (players == NULL) {
        return NULL;
    }

    player_t *found_player = NULL;
    for (int i = 0; i < players->count; ++i) {
        if (STR_EQ(name, players->players[i]->name)) {
            found_player = players->players[i];
            break;
        }
    }

    if (found_player == NULL) {
        release_players_array(players);
        return NULL;
    }

    player_t *player_copy = create_player(found_player->name, found_player->level, found_player->stone, found_player->copper, found_player->iron, found_player->gold);
    
    release_players_array(players);
    if (player_copy == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        return NULL;
    }

    return player_copy;
}

/**
 * @brief 
 * 
 * @param width 
 * @param button_width 
 * @param button_height 
 * @param player 
 * @param last 
 * @param row_margin
 * 
 * @warning Minimal <button_width> value is 21 px_t. This value is set to the length of warning string used if the player's name is too long!
 */
static void put_player(px_t width, px_t button_width, px_t button_height, player_t *player, bool last, px_t row_margin)
{
    char result[button_width - 2 - 6]; // two characters for border and 6 characters for text ": LVL "

    int written = snprintf(result, sizeof(result), "%s: LVL %d", player->name, player->level);
    if (written >= sizeof(result)) {
        strcpy(result, "Error: too long name!");
    }

    if (written < sizeof(result)) {
        result[written] = '\0';
    }

    put_button(width, button_width, button_height, result, CENTER, last, row_margin);
}

static page_return_code_t load_choose_player_page(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data)
{
    players_array_t *players = load_players(PLAYERS_DATA_PATH);
    if (players == NULL) {
        return ERROR;
    }

    clear_canvas();
    draw_borders(height, width);
    set_cursor_at_beginning_of_canvas();

    put_empty_row(1);
    put_game_logo(width, CENTER);
    put_empty_row(1);
    put_text("Enter the name of player account you want to play.", width, CENTER);
    put_empty_row(1);

    int row_margin = 0;
    int rest = players->count - (data->curr_players_page_index * 3);
    if (rest > 3) {
        rest = 3;
    }
    
    for (int i = 0; i < rest; ++i) {
        if (i > 0) {
            write_text(" ");
        }
        put_player(width / rest, 30, 5, players->players[(data->curr_players_page_index * 3) + i], (i == (rest - 1) ? false : true), row_margin);
        row_margin += (width / rest);
    }

    put_empty_row(1);
    put_text("\t\t\t BACK [B]     CREATE PLAYER [C]     QUIT [Q]     NEXT [N]", width, LEFT);
    put_empty_row(1);

    release_players_array(players);

     if (render_terminal(terminal_data, width, data->terminal_signal, NULL, TERMINAL_N_A) == -1) {
        return ERROR;
    }
    
    return SUCCES;
}

static page_return_code_t load_pre_create_new_player_page(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data)
{
    clear_canvas();
    draw_borders(height, width);
    set_cursor_at_beginning_of_canvas();

    put_empty_row(1);
    put_game_logo(width, CENTER);
    put_empty_row(2);
    put_text("No saved players accounts were found.", width, CENTER);
    put_empty_row(1);
    put_text("CREATE PLAYER [C]", width, CENTER);
    put_text("BACK [B]", width, CENTER);
    put_text("QUIT [Q]", width, CENTER);
    put_empty_row(4);
    
    if (render_terminal(terminal_data, width, data->terminal_signal, NULL, TERMINAL_N_A) == -1) {
        return ERROR;
    }
    
    return SUCCES;
}

static page_t handle_save_and_play(page_loader_inner_data_t *data)
{
    if (data->curr_player_name == NULL) {
        data->curr_player_name = NO_NAME_ENTERED;
        data->curr_player_name_seen_flag = false;
        return CREATE_NEW_PLAYER_PAGE;
    }

    data->player_choosen_to_game = create_player(data->curr_player_name, 0, 0, 0, 0, 0);

    free(data->curr_player_name);
    data->curr_player_name = NULL;

    if (data->player_choosen_to_game == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        return ERROR_PAGE;
    }

    if (write_player_to_file(data->player_choosen_to_game, PLAYERS_DATA_PATH) == -1) {
        resolve_error(UNOPENABLE_FILE);
        return ERROR_PAGE;
    }
    return GAME_PAGE;
}

static bool check_name(const char *name, page_loader_inner_data_t *data)
{
    if (name != NULL) {
        for (int i = 0; i < strlen(name); ++i) {
            if (name[i] == ';') {
                return false;
            }
        }
    }

    free(data->curr_player_name);
    data->curr_player_name = NULL;

    data->curr_player_name = malloc(strlen(name) + 1);
    if (data->curr_player_name == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        return false;
    }

    strcpy(data->curr_player_name, name);
    return true;
}

static bool is_name_unique(const char *name, page_loader_inner_data_t *data)
{
    players_array_t *players = load_players(PLAYERS_DATA_PATH);
    if (players == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        return false;
    }

    for (int i = 0; i < players->count; ++i) {
        if (STR_EQ(name, players->players[i]->name)) {
            free(data->curr_player_name);
            data->curr_player_name = NULL;
            release_players_array(players);
            return false;
        }
    }

    release_players_array(players);
    return true;
}

static page_return_code_t load_create_new_player_page(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data)
{
    clear_canvas();
    draw_borders(height, width);
    set_cursor_at_beginning_of_canvas();

    put_empty_row(1);
    put_game_logo(width, CENTER);
    put_empty_row(2);
    put_text("You are creating new player.", width, CENTER);
    put_text("Please, enter name without ';' and press Enter to validate the name.", width, CENTER);

    put_empty_row(1);
    put_text("PLAY WITHOUT CREATING PLAYER [W]", width, CENTER);
    put_text("SAVE AND PLAY [S]", width, CENTER);
    put_text("BACK [B]", width, CENTER);
    put_text("QUIT [Q]", width, CENTER);
    put_empty_row(2);

    if (data->curr_player_name != NULL && !data->curr_player_name_seen_flag) {
        if (STR_EQ(data->curr_player_name, INVALID_NAME)) {
            data->curr_player_name = NULL;
            if (render_terminal(terminal_data, width, true, "This name is invalid.", TERMINAL_ERROR) == -1) {
                return ERROR;
            }
        } else if (STR_EQ(data->curr_player_name, NOT_UNIQUE_NAME)) {
            data->curr_player_name = NULL;
            if (render_terminal(terminal_data, width, true, "This name is not unique.", TERMINAL_ERROR) == -1) {
                return ERROR;
            }
        } else if (STR_EQ(data->curr_player_name, NO_NAME_ENTERED)) {
            data->curr_player_name = NULL;
            if (render_terminal(terminal_data, width, true, "No name was entered.", TERMINAL_ERROR) == -1) {
                return ERROR;
            }
        } else if (render_terminal(terminal_data, width, true, "This name is valid and unique.", TERMINAL_APPROVAL) == -1) {
            return ERROR;
        }
        data->curr_player_name_seen_flag = true;
        return SUCCES;
    }

    if (render_terminal(terminal_data, width, false, NULL, TERMINAL_N_A) == -1) {
        return ERROR;
    }
    
    return SUCCES;
}

static char *create_string(const char *format, ...)
{
    va_list args, args_copy;
    va_start(args, format);
    va_copy(args_copy, args);
    
    int size = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);

    char *string = (char *)malloc(size + 1);
    if (string == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        va_end(args);
        return NULL;
    }

    vsnprintf(string, size + 1, format, args);
    va_end(args);

    return string;
}

static page_return_code_t handle_stats_for_no_player(px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data)
{
    put_empty_row(1);
    put_text("No statistics available for the game without the player.", width, CENTER);
    put_empty_row(3);
    put_text("NEW GAME [N]", width, CENTER);
    put_text("QUIT [Q]", width, CENTER);
    put_empty_row(3);

    if (render_terminal(terminal_data, width, data->terminal_signal, NULL, TERMINAL_N_A) == -1) {
        return ERROR;
    }
    
    return SUCCES;
}

static page_return_code_t load_after_game_page(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data)
{
    clear_canvas();
    draw_borders(height, width);
    set_cursor_at_beginning_of_canvas();
    put_empty_row(1);
    put_game_logo(width, CENTER);
    put_empty_row(1);

    if (data->player_choosen_to_game == NULL) {
        return handle_stats_for_no_player(width, data, terminal_data);
    }

    player_t *updated_player = find_player(data->player_choosen_to_game->name, PLAYERS_DATA_PATH);
    if (updated_player == NULL) {
        return ERROR;
    }

    char *intro = create_string("Game statistics for the player '%s'.", data->player_choosen_to_game->name);
    if (intro == NULL) {
        release_player(updated_player);
        release_player(data->player_choosen_to_game);
        return ERROR;
    }

    char *game_collected = create_string("In the game you collected: %d STONE, %d IRON, %d COPPER and %d GOLD.", updated_player->stone - data->player_choosen_to_game->stone,
                                                                                                                 updated_player->iron - data->player_choosen_to_game->iron,
                                                                                                                 updated_player->copper - data->player_choosen_to_game->copper,
                                                                                                                 updated_player->gold - data->player_choosen_to_game->gold);
    if (game_collected == NULL) {
        release_player(updated_player);
        release_player(data->player_choosen_to_game);
        free(intro);
        return ERROR;
    }

    char *level_info = create_string("For the level %d you need still: STONE (%d/?), IRON (%d/?), COPPER (%d/?) and GOLD (%d/?).", updated_player->level + 1,
                                                                                                                                   updated_player->stone,
                                                                                                                                   updated_player->iron,
                                                                                                                                   updated_player->copper,
                                                                                                                                   updated_player->gold);
    if (level_info == NULL) {
        free(game_collected);
        free(intro);
        release_player(updated_player);
        release_player(data->player_choosen_to_game);
        return ERROR;
    }
    
    put_text(intro, width, CENTER);
    put_empty_row(1);
    put_text(game_collected, width, CENTER);
    put_text(level_info, width, CENTER);
    put_empty_row(2);
    put_text("NEW GAME [N]", width, CENTER);
    put_text("QUIT [Q]", width, CENTER);
    put_empty_row(2);

    release_player(updated_player);
    free(game_collected);
    free(level_info);
    free(intro);

    if (render_terminal(terminal_data, width, data->terminal_signal, NULL, TERMINAL_N_A) == -1) {
        return ERROR;
    }
    
    return SUCCES;
}

static int init_file_descriptor_monitor()
{
    // setup the file descriptor set for select
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(STDIN_FILENO, &read_fds); // add standard input to the set

    // set the timeout for select as non-blocking behavior
    struct timeval timeout;
    timeout.tv_sec = 0; timeout.tv_usec = 0;

    // call select to check for input readiness
    return select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &timeout);
}

static int update_players_stats(player_t *target_player, const char *file_path)
{
    if (access(file_path, F_OK) != 0) {
        resolve_error(MISSING_DATA_FILE);
        return -1;
    }

    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        resolve_error(UNOPENABLE_FILE);
        return -1;
    }

    FILE* temp_file = fopen("_temp.data", "w");
    if (temp_file == NULL) {
        resolve_error(UNOPENABLE_FILE);
        fclose(file);
        return -1;
    }

    char* line = NULL;
    size_t line_length = 0;
    ssize_t bytes_read;
    bool target_found = false;

    while ((bytes_read = getline(&line, &line_length, file)) != -1) {

        if (STR_EQ(line, "\n")) {
            continue;
        }

        player_t *player = create_player_from_string(line);
        if (player == NULL) {
            free(line);
            fclose(file);
            fclose(temp_file);
            return -1;
        }

        if (STR_EQ(player->name, target_player->name)) {
            target_found = true;
            char *result = create_player_string(target_player, true);
            if (result == NULL) {
                free(line);
                fclose(file);
                fclose(temp_file);
                return -1;
            }
            fputs(result, temp_file);
            free(result);
        } else {
            fputs(line, temp_file);
        }

        release_player(player);
    }

    free(line);
    fclose(file);
    fclose(temp_file);

    if (remove(file_path) != 0) {
        resolve_error(FAILURE_OF_REMOVING_FILE);
        return -1;
    }

    if (rename("_temp.data", file_path) != 0) {
        resolve_error(FAILURE_OF_RENAMING_FILE);
        return -1;
    }

    if (!target_found) {
        if (write_player_to_file(target_player, "_temp.data") == -1) {
            resolve_error(UNOPENABLE_FILE);
            return -1;
        }
    }
    
    return 0;
}

static void display_hearts(const char *player_name, int number_of_hearts)
{
    write_text("%s hearts (%d/3): ", player_name, number_of_hearts);
    for (int i = 0; i < number_of_hearts; ++i) {
        write_text("\033[0;31m♥\033[0m");
    }
    if (number_of_hearts < 3) {
        write_text(" ");
    }
}

static void display_resources(player_t *player, int width)
{
    put_text("Resources:", width, CENTER);
    write_text("\t\t   STONE (%d/?) IRON(%d/?) COPPER(%d/?) GOLD(%d/?) ", player->stone, player->iron, player->copper, player->gold);
}

static void display_live_stats(game_t *game)
{
    write_text("\n");
    display_hearts("Enemy", game->enemy->hearts);
    display_hearts("\t\t\t      Your", game->player->hearts);
    write_text("\n");
    display_resources(game->player, game->width);
}

static page_return_code_t load_game(px_t height, px_t width, page_loader_inner_data_t *data)
{
    game_t *game = init_game(data->player_choosen_to_game, height, width);
    if (game == NULL) {
        release_player(data->player_choosen_to_game);
        return ERROR;
    }

    scene_t *scene = init_scene(game);
    if (scene == NULL) {
        release_game(game);
        return ERROR;
    }

    pixel_buffer_t *pixel_buffer1 = create_pixel_buffer(height, width);
    if (pixel_buffer1 == NULL) {
        release_game(game);
        release_scene(scene);
        release_player(data->player_choosen_to_game);
        return ERROR;
    }

    pixel_buffer_t *pixel_buffer2 = create_pixel_buffer(height, width);
    if (pixel_buffer2 == NULL) {
        release_game(game);
        release_scene(scene);
        release_pixel_buffer(pixel_buffer1);
        release_player(data->player_choosen_to_game);
        return ERROR;
    }

    clear_canvas();
    start_game(game);

    while (get_game_state(game) != STOPPED) {

        draw_borders(height + 1, width);
        set_cursor_at_beginning_of_canvas();
        reset_pixel_buffer(pixel_buffer2);

        if (init_file_descriptor_monitor() > 0) {
            int c = getchar();
            handle_event(game, c);
        }

        update_scene(game, pixel_buffer2);

        pixel_buffer_t *tmp_buffer = pixel_buffer1;
        pixel_buffer1 = pixel_buffer2;
        pixel_buffer2 = tmp_buffer;
        render_graphics(pixel_buffer1, scene);

        display_live_stats(game);
        usleep(70000);
    }

    release_pixel_buffer(pixel_buffer1);
    release_pixel_buffer(pixel_buffer2);

    if (update_players_stats(game->player, PLAYERS_DATA_PATH) == -1) {
        release_game(game);
        release_player(data->player_choosen_to_game);
        return ERROR;
    }

    release_game(game);
    return SUCCESS_GAME;
}

static void put_game_logo(px_t width, position_t position)
{
    put_text(".___        __                _________ __         .__  .__                 __________                      ", width, position);
    put_text("|   | _____/  |_  ___________/   _____//  |_  ____ |  | |  | _____ _______  \\______   \\____   ____    ____  ", width, position);
    put_text("|   |/    \\   __\\/ __ \\_  __ \\_____  \\\\   __\\/ __ \\|  | |  | \\__  \\\\_  __ \\  |     ___/  _ \\ /    \\  / ___\\ ", width, position);
    put_text("|   |   |  \\  | \\  ___/|  | \\/        \\|  | \\  ___/|  |_|  |__/ __ \\|  | \\/  |    |  (  <_> )   |  \\/ /_/  >", width, position);
    put_text("|___|___|  /__|  \\___  >__| /_______  /|__|  \\___  >____/____(____  /__|     |____|   \\____/|___|  /\\___  / ", width, position);
    put_text("         \\/          \\/             \\/           \\/               \\/                             \\//_____/  ", width, position);
}

page_loader_inner_data_t *create_page_loader_inner_data()
{
    page_loader_inner_data_t *data = malloc(sizeof(page_loader_inner_data_t));
    if (data == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        return NULL;
    }

    data->curr_player_name_seen_flag = false; data->curr_players_page_index = 0; data->players_count = 0;
    data->terminal_signal = false; data->curr_player_name = NULL; data->player_choosen_to_game = NULL;

    return data;
}

void release_page_loader_inner_data(page_loader_inner_data_t *data)
{
    if (data != NULL) {
        free(data);
    }
}