#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>

#include "draw.h"
#include "errors.h"
#include "../interstellar-pong-implementation/interstellar_pong.h"
#include "page_loader.h"
#include "../interstellar-pong-implementation/paths.h"
#include "../interstellar-pong-implementation/player.h"
#include "terminal.h"
#include "utils.h"

// ---------------------------------------- MACROS --------------------------------------------- //

#define COMMAND_EQ(command, ch, CH, word, WORD) (STR_EQ(command, ch) || STR_EQ(command, CH) || STR_EQ(command, word) || STR_EQ(command, WORD))
#define GAME_WIDTH 80

// ---------------------------------------- STATIC DECLARATIONS--------------------------------- //

static page_return_code_t load_error_page(px_t height, px_t width);
static page_return_code_t load_about_page(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data);
static page_return_code_t load_not_found_page(px_t height, px_t width);
static page_return_code_t load_game(px_t height, px_t width, page_loader_inner_data_t *data);
static page_return_code_t load_main_page(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data);
static page_return_code_t load_after_game_page(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data);
static page_return_code_t load_choose_player_page(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data);
static page_return_code_t load_create_new_player_page(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data);
static page_return_code_t load_pre_create_new_player_page(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data);
static page_return_code_t load_quit_or_back_with_confirmation(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data);

static page_return_code_t display_new_name_in_terminal(px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data);
static page_return_code_t handle_stats_for_no_player(px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data);
static void put_player(px_t width, px_t button_width, px_t button_height, player_t *player, bool last, px_t row_margin);
static void check_and_set_player_name(const char *command, page_loader_inner_data_t *data);
static void display_resources(player_t *player, levels_table_t *levels, int width);
static int update_players_stats(player_t *target_player, const char *file_path);
static const char *create_resources_string(player_t *player, level_row_t level);
static bool is_name_too_long(const char *name, page_loader_inner_data_t *data);
static bool is_name_unique(const char *name, page_loader_inner_data_t *data);
static char *create_player_string(player_t *player, bool end_with_newline);
static void display_hearts(const char *player_name, int number_of_hearts);
static int write_player_to_file(player_t *player, const char *file_path);
static bool is_name_valid(const char *name, page_loader_inner_data_t *data);
static player_t *find_player(const char *name, const char *file_path);
static page_t handle_save_and_play(page_loader_inner_data_t *data);
static page_t choose_pregame_page(page_loader_inner_data_t *data);
static const char *create_level_info_string(player_t *player);
static players_array_t *load_players(const char* file_path);
static void put_game_logo(px_t width, position_t position);
static void display_live_stats(game_t *game);

// ----------------------------------------- PROGRAM-------------------------------------------- //

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
    case ABOUT_PAGE:
        if (COMMAND_EQ(command, "q", "Q", "quit", "QUIT")) {
            return QUIT_WITHOUT_CONFIRMATION_PAGE;
        } else if (COMMAND_EQ(command, "b", "B", "back", "BACK")) {
            return MAIN_PAGE;
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
        if (COMMAND_EQ(command, "p", "P", "play again", "PLAY AGAIN")) {
            if (data->player_choosen_to_game == NULL) {
                return GAME_PAGE;
            }
            char *name = strdup(data->player_choosen_to_game->name);
            if (name == NULL) {
                release_player(data->player_choosen_to_game);
                data->player_choosen_to_game = NULL;
                resolve_error(MEM_ALOC_FAILURE);
                return ERROR_PAGE;
            }
            release_player(data->player_choosen_to_game);
            data->player_choosen_to_game = NULL;
            if ((data->player_choosen_to_game = find_player(name, PLAYERS_DATA_PATH)) != NULL) {
                free(name);
                return GAME_PAGE;
            }
            free(name);
        } else if (COMMAND_EQ(command, "n", "N", "new game", "NEW GAME")) {
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
            if ((data->player_choosen_to_game = find_player(command, PLAYERS_DATA_PATH)) != NULL) {
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
            check_and_set_player_name(command, data);
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
    case ABOUT_PAGE:
        return load_about_page(height, width, data, terminal_data);
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
    case QUIT_WITH_CONFIRMATION_FROM_CREATE_NEW_PLAYER_PAGE_PAGE: return "Quit With Confirmation From Create New Player page page";
    case BACK_WITH_CONFIRMATION_FROM_CREATE_NEW_PLAYER_PAGE_PAGE: return "Back With Confirmation From Create New Player page page";
    default:
        break;
    }
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

/**
 * @brief Loads the main page content onto the terminal screen.
 *
 * @param height The height of the terminal window.
 * @param width The width of the terminal window.
 * @param data Page loader inner data structure.
 * @param terminal_data Terminal data structure for rendering.
 * @return The return code indicating success or error.
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

    return SUCCESS;
}

/**
 * @brief Loads the about page content onto the terminal screen.
 *
 * @param height The height of the terminal window.
 * @param width The width of the terminal window.
 * @param data Page loader inner data structure.
 * @param terminal_data Terminal data structure for rendering.
 * @return The return code indicating success or error.
 */
static page_return_code_t load_about_page(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data)
{
    clear_canvas();
    draw_borders(height, width);
    set_cursor_at_beginning_of_canvas();

    put_empty_row(1);
    put_game_logo(width, CENTER);
    put_empty_row(1);
    put_text("Interstellar Pong, set in the vast outer space, is a captivating and modern take on the classic game of Pong.", width, CENTER);
    put_empty_row(1);
    put_text("~ How To ~", width, CENTER);
    put_empty_row(1);
    put_text("• App is based on the virtual terminal. Enter commands as their full name or their shortcuts", width, CENTER);
    put_text("• In the game itself, use the \"W\" key to move up, the \"S\" key to move down and \"Q\" to quit the game.", width, CENTER);
    put_empty_row(1);
    
    put_text("BACK [B]", width, CENTER);
    put_text("QUIT [Q]", width, CENTER);
    put_empty_row(1);

    if (render_terminal(terminal_data, width, data->terminal_signal, NULL, TERMINAL_N_A) == -1) {
        return ERROR;
    }

    return SUCCESS;
}

/**
 * @brief Loads a confirmation page for quitting or going back.
 *
 * This function displays a confirmation page with options to leave unsaved work.
 *
 * @param height The height of the canvas.
 * @param width The width of the canvas.
 * @param data Pointer to page_loader_inner_data_t structure.
 * @param terminal_data Pointer to terminal_data_t structure.
 * @return Page return code indicating the result of loading the page.
 */
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

    return SUCCESS;
}

/**
 * @brief Loads a "page not found" page.
 *
 * This function displays a page indicating that the requested page was not found.
 *
 * @param height The height of the canvas.
 * @param width The width of the canvas.
 * @return Page return code indicating the result of loading the page.
 */
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

/**
 * @brief Loads an error page.
 *
 * This function displays a page indicating that an error has occurred.
 *
 * @param height The height of the canvas.
 * @param width The width of the canvas.
 * @return Page return code indicating the result of loading the page.
 */
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

/**
 * @brief Loads players' data from a file.
 *
 * This function loads player data from a file and returns it as a players_array_t structure.
 *
 * @param file_path The path to the file containing players' data.
 * @return Pointer to a players_array_t structure containing loaded player data.
 * @retval NULL if there was an error.
 */
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

/**
 * @brief Creates a formatted player string.
 *
 * This function creates a formatted string from a player's data.
 *
 * @param player Pointer to a player_t structure.
 * @param end_with_newline Indicates whether the string should end with a newline character.
 * @return Pointer to the created formatted string.
 */
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

/**
 * @brief Writes a player's data to a file.
 *
 * This function writes a player's data to a file.
 *
 * @param player Pointer to a player_t structure.
 * @param file_path The path to the file where the data will be written.
 * @return 0 on success, -1 on error.
 */
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

/**
 * @brief Chooses the appropriate pregame page based on the presence of saved players.
 *
 * This function determines whether to show the choose player page or create new player page.
 *
 * @param data Pointer to page_loader_inner_data_t structure.
 * @return The chosen page.
 */
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

/**
 * @brief Finds a player by name in the given file.
 *
 * This function searches for a player by name in a file and returns a copy of the player data.
 *
 * @param name The name of the player to find.
 * @param file_path The path to the file containing players' data.
 * @return Pointer to a copied player_t structure if found, NULL otherwise.
 */
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
 * @brief Displays a player button on the screen.
 *
 * This function displays a player button with their name and level.
 *
 * @param width The width of the canvas.
 * @param button_width The width of the button (at least 21 px_t).
 * @param button_height The height of the button.
 * @param player Pointer to a player_t structure.
 * @param last Indicates if this is the last button in the row.
 * @param row_margin Margin for positioning the button.
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

/**
 * @brief Loads the "choose player" page.
 *
 * This function displays the "choose player" page with player selection options.
 *
 * @param height The height of the canvas.
 * @param width The width of the canvas.
 * @param data Pointer to page_loader_inner_data_t structure.
 * @param terminal_data Pointer to terminal_data_t structure.
 * @return Page return code indicating the result of loading the page.
 */
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
        put_player(width / rest - ((rest == 1) ? 1 : 0), 30, 5, players->players[(data->curr_players_page_index * 3) + i], (i == (rest - 1) ? false : true), row_margin);
        row_margin += (width / rest);
    }

    put_empty_row(1);
    put_text("\t\t\t BACK [B]     CREATE PLAYER [C]     QUIT [Q]     NEXT [N]", width, LEFT);
    put_empty_row(1);

    release_players_array(players);

     if (render_terminal(terminal_data, width, data->terminal_signal, NULL, TERMINAL_N_A) == -1) {
        return ERROR;
    }
    
    return SUCCESS;
}

/**
 * Loads the pre-create new player page, displaying information for creating a new player or going back to the main menu.
 * 
 * @param height The height of the display.
 * @param width The width of the display.
 * @param data Inner data structure for page loading.
 * @param terminal_data Terminal data for rendering.
 * @return The page return code indicating the outcome of the loading process.
 */
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
    
    return SUCCESS;
}

/**
 * Handles saving the created player and transitioning to the game page.
 * 
 * @param data Inner data structure for page loading.
 * @return The page to transition to based on the handling outcome.
 */
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

/**
 * @brief Checks and sets the player's name based on the provided command.
 *
 * This function performs validation and uniqueness checks on the provided player name command.
 * If the name is valid, unique, and not too long, it is set as the current player's name in the
 * `page_loader_inner_data_t` structure. Otherwise, appropriate error flags are set.
 *
 * @param command The player name command to be checked and set.
 * @param data Pointer to the page loader inner data structure.
 *
 * @note This function frees the current player's name in `data` before setting a new name.
 */
static void check_and_set_player_name(const char *command, page_loader_inner_data_t *data)
{
    free(data->curr_player_name);
    data->curr_player_name = NULL;

    if (!is_name_valid(command, data)) { // name is being stored here
        data->curr_player_name = INVALID_NAME;
        return;
    }

    if (!is_name_unique(command, data)) {
        data->curr_player_name = NOT_UNIQUE_NAME;
        return;
    }

    if (is_name_too_long(command, data)) {
        data->curr_player_name = TOO_LONG_NAME;
        return;
    }
}

/**
 * Checks the validity of the entered player name and stores it in the data structure.
 * 
 * @param name The name to check.
 * @param data Inner data structure for page loading.
 * @return Returns true if the name is valid and stored successfully, otherwise false.
 */
static bool is_name_valid(const char *name, page_loader_inner_data_t *data)
{
    if (name != NULL) {
        if (STR_EQ(name, "")) {
            return false;
        }
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

/**
 * Checks if the entered player name is unique among existing players.
 * 
 * @param name The name to check for uniqueness.
 * @param data Inner data structure for page loading.
 * @return Returns true if the name is unique, otherwise false.
 */
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

/**
 * Checks if the entered player name is too long.
 * The hard limit was set to 14 bacause of the width of buttons in players' gallery.
 * 
 * @param name The name to check for uniqueness.
 * @param data Inner data structure for page loading.
 * @return Returns true if the name is too long, otherwise false.
 */
static bool is_name_too_long(const char *name, page_loader_inner_data_t *data)
{
    if (strlen(name) >= 15) {
        free(data->curr_player_name);
        data->curr_player_name = NULL;
        return true;
    }

    return false;
}

/**
 * Loads the create new player page, allowing the user to enter a new player name and validating its uniqueness.
 * 
 * @param height The height of the display.
 * @param width The width of the display.
 * @param data Inner data structure for page loading.
 * @param terminal_data Terminal data for rendering.
 * @return The page return code indicating the outcome of the loading process.
 */
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
        return display_new_name_in_terminal(width, data, terminal_data);
    }

    if (render_terminal(terminal_data, width, false, NULL, TERMINAL_N_A) == -1) {
        return ERROR;
    }
    
    return SUCCESS;
}

/**
 * @brief Displays a new player name in the terminal.
 *
 * This function displays a player's name in the terminal along with an associated message,
 * depending on the validity and uniqueness of the name.
 *
 * @param width The width of the display.
 * @param data Pointer to the page loader inner data structure.
 * @param terminal_data Pointer to the terminal data structure.
 *
 * @return SUCCESS if the name is valid and unique else ERROR
 */
static page_return_code_t display_new_name_in_terminal(px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data)
{
    char *message = NULL;
    terminal_output_mode_t terminal_output_mode = TERMINAL_ERROR;

    if (STR_EQ(data->curr_player_name, INVALID_NAME)) {
        message = "This name is not valid";
        data->curr_player_name = NULL;
    } else if (STR_EQ(data->curr_player_name, NOT_UNIQUE_NAME)) {
        data->curr_player_name = NULL;
        message = "This name is not unique.";
    } else if (STR_EQ(data->curr_player_name, NO_NAME_ENTERED)) {
        data->curr_player_name = NULL;
        message = "No name was entered.";
    } else if (STR_EQ(data->curr_player_name, TOO_LONG_NAME)) {
        data->curr_player_name = NULL;
        message = "Entered name is too long (max 14 characters).";
    } else {
        message = "This name is valid and unique.";
        terminal_output_mode = TERMINAL_APPROVAL;
    } 

    if (render_terminal(terminal_data, width, true, message, terminal_output_mode) == -1) {
        return ERROR;
    }

    data->curr_player_name_seen_flag = true;
    return SUCCESS;
}

/**
 * Handles the display of statistics when no player is available for the game.
 * 
 * @param width The width of the display.
 * @param data Inner data structure for page loading.
 * @param terminal_data Terminal data for rendering.
 * @return The page return code indicating the outcome of the loading process.
 */
static page_return_code_t handle_stats_for_no_player(px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data)
{
    put_empty_row(1);
    put_text("No statistics available for the game without the player.", width, CENTER);
    put_empty_row(3);
    put_text("PLAY AGAIN [P]", width, CENTER);
    put_text("NEW GAME [N]", width, CENTER);
    put_text("QUIT [Q]", width, CENTER);
    put_empty_row(2);

    if (render_terminal(terminal_data, width, data->terminal_signal, NULL, TERMINAL_N_A) == -1) {
        return ERROR;
    }
    
    return SUCCESS;
}

/**
 * @brief Creates a formatted resources string based on player and level data.
 *
 * This function creates a formatted string that displays the resources' current amounts and request amounts
 * based on the provided player and level data. It formats the string differently depending on whether gold,
 * iron, or copper resources are requested.
 *
 * @param player A pointer to the player_t structure containing resource amounts.
 * @param level A level_row_t structure containing resource request amounts.
 * @return A dynamically allocated string representing the formatted resources string.
 *         The caller is responsible for freeing the memory when done using the string.
 * @warning The created string must be freed by the caller when no longer needed.
 */
static const char *create_level_info_string(player_t *player)
{
    levels_table_t *levels = NULL;
    materials_table_t *materials = NULL;
    if (!load_extern_game_data(GAME_DATA_PATH, &materials, &levels)) {
        return NULL;
    }

    char *level_info = NULL;
    if (player->level + 1 > levels->count - 1) {
        level_info = create_string("You have reached the maximum level! You are now the master of the interstellar space...");
    } else {
        level_info = create_string("For the level %d you need still: STONE (%d/%d), COPPER (%d/%d), IRON (%d/%d) and GOLD (%d/%d).", player->level + 1,
                                                                                                                                     player->stone,
                                                                                                                                     levels->levels[player->level].stone_request,
                                                                                                                                     player->copper,
                                                                                                                                     levels->levels[player->level].copper_request,
                                                                                                                                     player->iron,
                                                                                                                                     levels->levels[player->level].iron_request,
                                                                                                                                     player->gold,
                                                                                                                                     levels->levels[player->level].gold_request);
    }

    release_levels_table(levels);
    release_materials_table(materials);

    return level_info;
}

/**
 * Loads the after-game page, displaying game statistics for the player.
 * 
 * @param height The height of the display.
 * @param width The width of the display.
 * @param data Inner data structure for page loading.
 * @param terminal_data Terminal data for rendering.
 * @return The page return code indicating the outcome of the loading process.
 */
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

    char *game_collected = create_string("In the game you collected: %d STONE, %d COPPER, %d IRON and %d GOLD.", (updated_player->stone - data->player_choosen_to_game->stone < 0) ? updated_player->stone : updated_player->stone - data->player_choosen_to_game->stone,
                                                                                                                 (updated_player->copper - data->player_choosen_to_game->copper < 0) ? updated_player->copper : updated_player->copper - data->player_choosen_to_game->copper,
                                                                                                                 (updated_player->iron - data->player_choosen_to_game->iron < 0) ? updated_player->iron : updated_player->iron - data->player_choosen_to_game->iron,
                                                                                                                 (updated_player->gold - data->player_choosen_to_game->gold < 0) ? updated_player->gold : updated_player->gold - data->player_choosen_to_game->gold);
    if (game_collected == NULL) {
        release_player(updated_player);
        release_player(data->player_choosen_to_game);
        free(intro);
        return ERROR;
    }

    char *level_info = (char*)create_level_info_string(updated_player);
    if (level_info == NULL) {
        release_player(updated_player);
        release_player(data->player_choosen_to_game);
        free(intro);
        return ERROR;
    }

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
    put_text("PLAY AGAIN [P]", width, CENTER);
    put_text("NEW GAME [N]", width, CENTER);
    put_text("QUIT [Q]", width, CENTER);
    put_empty_row(1);

    release_player(updated_player);
    free(game_collected);
    free(level_info);
    free(intro);

    if (render_terminal(terminal_data, width, data->terminal_signal, NULL, TERMINAL_N_A) == -1) {
        return ERROR;
    }
    
    return SUCCESS;
}

/**
 * Initializes a file descriptor monitor for non-blocking input readiness check.
 * 
 * @return The result of the select function for checking input readiness.
 */
static int init_file_descriptor_monitor()
{
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(STDIN_FILENO, &read_fds);

    struct timeval timeout;
    timeout.tv_sec = 0; timeout.tv_usec = 0;

    return select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &timeout);
}

/**
 * Updates the statistics of a target player in a specified file.
 * 
 * @param target_player The player whose statistics need to be updated. The player with name ";" is special mark to
 *                      show that the player was created only temporarily (game without player account), and thus does not need to be updated.
 *                      Function update_players_stats() is able to detect that and to handle the situation.
 * @param file_path The path of the file containing player data.
 * @return 0 if the update is successful, or -1 in case of errors.
 */
static int update_players_stats(player_t *target_player, const char *file_path)
{
    if (STR_EQ(target_player->name, ";")) {
        return 0;
    }

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

/**
 * Displays hearts with a specified player name and number of hearts.
 * 
 * @param player_name The name of the player.
 * @param number_of_hearts The number of hearts to display.
 */
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

/**
 * @brief Creates a formatted resources string based on player and level data.
 *
 * This function creates a formatted string that displays the resources' current amounts and request amounts
 * based on the provided player and level data.
 *
 * @param player A pointer to the player_t structure containing resource amounts.
 * @param level A level_row_t structure containing resource request amounts.
 * @return A dynamically allocated string representing the formatted resources string.
 *         The caller is responsible for freeing the memory when done using the string.
 * @warning The created string must be freed by the caller when no longer needed!
 */
static const char *create_resources_string(player_t *player, level_row_t level)
{
    if (level.gold_request != 0) {
        return create_string("STONE (%d/%d) COPPER(%d/%d) IRON(%d/%d) GOLD(%d/%d)",
                             player->stone, level.stone_request,
                             player->copper, level.copper_request,
                             player->iron, level.iron_request,
                             player->gold, level.gold_request);
    }

    if (level.iron_request != 0) {
        return create_string("STONE (%d/%d) COPPER(%d/%d) IRON(%d/%d)",
                             player->stone, level.stone_request,
                             player->copper, level.copper_request,
                             player->iron, level.iron_request);
    }

    if (level.copper_request != 0) {
        return create_string("STONE (%d/%d) COPPER(%d/%d)",
                             player->stone, level.stone_request,
                             player->copper, level.copper_request);
    }

    return create_string("STONE (%d/%d)", player->stone, level.stone_request);
}

/**
 * Displays resource information for a player.
 * 
 * @param player The player whose resources to display.
 * @param levels The data about the game levels.
 * @param width The width of the display.
 */
static void display_resources(player_t *player, levels_table_t *levels, int width)
{
    put_text("Resources:", width, CENTER);

    const char *string = NULL;
    if (player->level > (levels->count - 1)) {
       string = create_string("STONE(%d/N.A.) COPPER(%d/N.A.) IRON(%d/N.A.) GOLD(%d/N.A.)", player->stone, player->copper, player->iron, player->gold);
    } else {
        string = create_resources_string(player, levels->levels[player->level]);
    }

    if (string == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        put_text("Memory allocation error occured. Data about your resources could not be loaded.", width, CENTER);
        return;
    }

    put_text(string, width, CENTER);
    free((char*)string);
}

/**
 * Displays live game statistics including hearts and resources.
 * 
 * @param game The game data containing player and enemy information.
 */
static void display_live_stats(game_t *game)
{
    write_text("\n");
    display_hearts("Enemy", game->enemy->hearts);
    display_hearts("\t\t\t      Your", game->player->hearts);
    write_text("\n");
    display_resources(game->player, game->levels_table, game->width);
}

/**
 * Loads the game page and handles the game loop.
 * 
 * @param height The height of the display.
 * @param width The width of the display.
 * @param data Inner data structure for page loading.
 * @return The page return code indicating the outcome of the loading process.
 */
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

    bool next_frame_stopped = false;
    while (get_game_state(game) != TERMINATED) {

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

        if (next_frame_stopped) {
            usleep(1350000);
            next_frame_stopped = false;
        }

        if (get_game_state(game) == STOPPED) {
            next_frame_stopped = true;
            start_game(game);
        }
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

/**
 * Displays the game logo in ASCII art format.
 * 
 * @param width The width of the display.
 * @param position The position to display the logo.
 */
static void put_game_logo(px_t width, position_t position)
{
    put_text(".___        __                _________ __         .__  .__                 __________                      ", width, position);
    put_text("|   | _____/  |_  ___________/   _____//  |_  ____ |  | |  | _____ _______  \\______   \\____   ____    ____  ", width, position);
    put_text("|   |/    \\   __\\/ __ \\_  __ \\_____  \\\\   __\\/ __ \\|  | |  | \\__  \\\\_  __ \\  |     ___/  _ \\ /    \\  / ___\\ ", width, position);
    put_text("|   |   |  \\  | \\  ___/|  | \\/        \\|  | \\  ___/|  |_|  |__/ __ \\|  | \\/  |    |  (  <_> )   |  \\/ /_/  >", width, position);
    put_text("|___|___|  /__|  \\___  >__| /_______  /|__|  \\___  >____/____(____  /__|     |____|   \\____/|___|  /\\___  / ", width, position);
    put_text("         \\/          \\/             \\/           \\/               \\/                             \\//_____/  ", width, position);
}
