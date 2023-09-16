#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>

#include "draw.h"
#include "log.h"
#include "../interstellar-pong-implementation/interstellar_pong.h"
#include "page_loader.h"
#include "../interstellar-pong-implementation/paths.h"
#include "../interstellar-pong-implementation/player.h"
#include "../interstellar-pong-implementation/interstellar_pong_pages.h"
#include "terminal.h"
#include "utils.h"

// ----------------------------------------- PROGRAM-------------------------------------------- //

page_t find_page(page_t current_page, const char *command, page_loader_inner_data_t *data)
{
    return find_interstellar_page(current_page, command, data);
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
