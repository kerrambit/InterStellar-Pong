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
    MAIN_PAGE,
    QUIT_WITHOUT_CONFIRMATION_PAGE,
    ABOUT_PAGE,
    PREGAME_SETTING_PAGE,
    NOT_FOUND_PAGE
} page_t;

page_t find_page(page_t current_page, const char *command);
const char *convert_page_2_string(page_t page);
int load_page(page_t page, px_t height, px_t width);

#endif