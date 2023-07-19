#include <stdio.h>

#include "page_loader.h"
#include "utils.h"

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
    default:
        return NO_PAGE;
    }
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
    default:
        break;
    }
}