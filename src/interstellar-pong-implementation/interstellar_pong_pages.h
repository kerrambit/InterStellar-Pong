/**
 * @file interstellar_pong_pages.h
 * @author Marek Eibel
 * @brief Header file for declarations of all pages in the game for the page loader.
 * @version 0.1
 * @date 2023-09-13
 * 
 * @copyright Copyright (c) 2023
 */

#ifndef INTERSTELLAR_PONG_PAGES_H
#define INTERSTELLAR_PONG_PAGES_H

#include "levels.h"
#include "interstellar_pong.h"
#include "../termify/page_loader.h"
#include "../termify/draw.h"

#define GAME_WIDTH 80

/**
 * @brief Finds the next page for Interstellar Pong game based on the current page and command.
 *
 * @param current_page The current page enum.
 * @param command The command input.
 * @param data Page loader inner data structure.
 * @return The next page enum based on the command input.
 */
page_t find_interstellar_page(page_t current_page, const char *command, page_loader_inner_data_t *data);

/**
 * @brief Loads the main page content onto the terminal screen.
 *
 * @param height The height of the terminal window.
 * @param width The width of the terminal window.
 * @param data Page loader inner data structure.
 * @param terminal_data Terminal data structure for rendering.
 * @return The return code indicating success or error.
 */
page_return_code_t load_main_page(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data);

/**
 * @brief Loads an error page.
 *
 * This function displays a page indicating that an error has occurred.
 *
 * @param height The height of the canvas.
 * @param width The width of the canvas.
 * @return Page return code indicating the result of loading the page.
 */
page_return_code_t load_error_page(px_t height, px_t width);

/**
 * @brief Loads a "page not found" page.
 *
 * This function displays a page indicating that the requested page was not found.
 *
 * @param height The height of the canvas.
 * @param width The width of the canvas.
 * @return Page return code indicating the result of loading the page.
 */
page_return_code_t load_not_found_page(px_t height, px_t width);

/**
 * @brief Loads the about page content onto the terminal screen.
 *
 * @param height The height of the terminal window.
 * @param width The width of the terminal window.
 * @param data Page loader inner data structure.
 * @param terminal_data Terminal data structure for rendering.
 * @return The return code indicating success or error.
 */
page_return_code_t load_about_page(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data);

/**
 * Loads the game page and handles the game loop.
 * 
 * @param height The height of the display.
 * @param width The width of the display.
 * @param data Inner data structure for page loading.
 * @return The page return code indicating the outcome of the loading process.
 */
page_return_code_t load_game(px_t height, px_t width, page_loader_inner_data_t *data);

/**
 * Loads the after-game page, displaying game statistics for the player.
 * 
 * @param height The height of the display.
 * @param width The width of the display.
 * @param data Inner data structure for page loading.
 * @param terminal_data Terminal data for rendering.
 * @return The page return code indicating the outcome of the loading process.
 */
page_return_code_t load_after_game_page(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data);

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
page_return_code_t load_choose_player_page(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data);

/**
 * Loads the create new player page, allowing the user to enter a new player name and validating its uniqueness.
 * 
 * @param height The height of the display.
 * @param width The width of the display.
 * @param data Inner data structure for page loading.
 * @param terminal_data Terminal data for rendering.
 * @return The page return code indicating the outcome of the loading process.
 */
page_return_code_t load_create_new_player_page(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data);

/**
 * Loads the pre-create new player page, displaying information for creating a new player or going back to the main menu.
 * 
 * @param height The height of the display.
 * @param width The width of the display.
 * @param data Inner data structure for page loading.
 * @param terminal_data Terminal data for rendering.
 * @return The page return code indicating the outcome of the loading process.
 */
page_return_code_t load_pre_create_new_player_page(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data);

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
page_return_code_t load_quit_or_back_with_confirmation(px_t height, px_t width, page_loader_inner_data_t *data, terminal_data_t *terminal_data);

#endif
