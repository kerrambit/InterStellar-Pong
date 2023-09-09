/**
 * @file page_loader.h
 * @author Marek Eibel
 * @brief This file defines functions and data structures for managing page loading and navigation within a game application.
 *        It provides mechanisms to load various pages, handle user interactions, and transition between different game states.
 * @version 0.1
 * @date 2023-07-19
 * @copyright Copyright (c) 2023
 */

#ifndef PAGE_LOADER_H
#define PAGE_LOADER_H

#include "draw.h"
#include "../interstellar-pong-implementation/player.h"
#include "terminal.h"

/**
 * @enum page_t
 * @brief Enumeration representing different pages within the game interface.
 *
 * The `page_t` enumeration defines various pages within the game interface,
 * allowing for easy navigation and identification of different interface states.
 */
typedef enum page_t {
    NO_PAGE,                                                    /** No specific page. */
    ERROR_PAGE,                                                 /** Error page. */
    MAIN_PAGE,                                                  /** Main menu page. */
    ABOUT_PAGE,                                                 /** About page. */
    QUIT_WITHOUT_CONFIRMATION_PAGE,                             /** Quit page without confirmation. */
    QUIT_WITH_CONFIRMATION_FROM_CREATE_NEW_PLAYER_PAGE_PAGE,    /** Quit page with confirmation from the create new player page. */
    BACK_WITH_CONFIRMATION_FROM_CREATE_NEW_PLAYER_PAGE_PAGE,    /** Back page with confirmation from the create new player page. */
    PREGAME_SETTING_PAGE,                                       /** Pre-game settings page. */
    PRE_CREATE_NEW_PLAYER_PAGE,                                 /** Pre-create new player page. */
    CREATE_NEW_PLAYER_PAGE,                                     /** Create new player page. */
    CHOOSE_PLAYER_PAGE,                                         /** Choose player page. */
    NOT_FOUND_PAGE,                                             /** Page not found. */
    BACK_PAGE,                                                  /** Back page. */
    GAME_PAGE,                                                  /** Game page. */
    AFTER_GAME_PAGE                                             /** After-game page. */
} page_t;

/**
 * @enum page_return_code_t
 * @brief Enumeration representing different return codes from page loader functions.
 *
 * The `page_return_code_t` enumeration defines return codes used by page loader functions,
 * indicating the success or failure of the page transition or loading process.
 */
typedef enum page_return_code_t {
    ERROR = -1,     /** Error code, indicating a failure during page transition or loading. */
    SUCCESS,        /** Success code, indicating successful page transition or loading. */
    SUCCESS_GAME    /** Success code, indicating successful page transition and starting the game. */
} page_return_code_t;

/**
 * @struct page_loader_inner_data_t
 * @brief Structure holding inner data used by the page loader for managing interface states.
 *
 * The `page_loader_inner_data_t` structure contains inner data used by the page loader to manage
 * information related to the current player's page index, players count, chosen player name, player
 * selection for the game, terminal signal status, and more.
 */
typedef struct page_loader_inner_data_t {
    int curr_players_page_index;         /** Current index of the player's page. */
    int players_count;                   /** Count of available players. */
    char *curr_player_name;              /** Current chosen player's name. */
    bool curr_player_name_seen_flag;     /** Flag indicating if the current player's name has been seen. */
    player_t *player_choosen_to_game;    /** Chosen player for the game. */
    bool terminal_signal;                /** Terminal signal status. */
} page_loader_inner_data_t;

/**
 * @brief Loads the content of a specific page based on the page enum.
 *
 * @param page The page to load.
 * @param height The height of the terminal window.
 * @param width The width of the terminal window.
 * @param data Page loader inner data structure.
 * @param terminal_data Terminal data structure for rendering.
 * @return The return code indicating success or error.
 */
page_return_code_t load_page(page_t page, px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data);

/**
 * @brief Finds the next page based on the current page and command.
 *
 * @param current_page The current page enum.
 * @param command The command input.
 * @param data Page loader inner data structure.
 * @return The next page enum based on the command input.
 */
page_t find_page(page_t current_page, const char *command, page_loader_inner_data_t *data);

/**
 * @brief Creates a new page_loader_inner_data_t structure.
 * 
 * This function dynamically allocates memory for a page_loader_inner_data_t structure
 * and initializes its fields to default values.
 * 
 * @return A pointer to the newly allocated page_loader_inner_data_t structure.
 * @retval NULL if memory allocation fails.
 */
page_loader_inner_data_t *create_page_loader_inner_data();

/**
 * @brief Releases the memory used by a page_loader_inner_data_t structure.
 * 
 * This function frees the memory occupied by a page_loader_inner_data_t structure.
 * 
 * @param data A pointer to the page_loader_inner_data_t structure to be released.
 */
void release_page_loader_inner_data(page_loader_inner_data_t *data);

/**
 * @brief Converts a page enum to its corresponding string representation.
 *
 * @param page The page enum to convert.
 * @return The string representation of the page.
 */
const char *convert_page_2_string(page_t page);

#endif
