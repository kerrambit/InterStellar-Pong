/**
 * @file page_loader.h
 * @author Marek Eibel
 * @brief 
 * @version 0.1
 * @date 2023-07-19
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef PAGE_LOADER_H
#define PAGE_LOADER_H

#include "draw.h"

typedef enum page_t {
    NO_PAGE,
    ERROR_PAGE,
    MAIN_PAGE,
    ABOUT_PAGE,
    QUIT_WITHOUT_CONFIRMATION_PAGE,
    QUIT_WITH_CONFIRMATION_FROM_CREATE_NEW_PLAYER_PAGE_PAGE,
    BACK_WITH_CONFIRMATION_FROM_CREATE_NEW_PLAYER_PAGE_PAGE,
    PREGAME_SETTING_PAGE,
    PRE_CREATE_NEW_PLAYER_PAGE,
    CREATE_NEW_PLAYER_PAGE,
    CHOOSE_PLAYER_PAGE,
    NOT_FOUND_PAGE,
    BACK_PAGE,
    GAME_PAGE,
    AFTER_GAME_PAGE
} page_t;

typedef enum page_return_code_t {
    ERROR=-1,
    SUCCES,
    SUCCESS_GAME
} page_return_code_t;

page_t find_page(page_t current_page, const char *command);
const char *convert_page_2_string(page_t page);
page_return_code_t load_page(page_t page, px_t height, px_t width, bool terminal_signal_unkwown_commands);

#endif