#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "page_loader.h"
#include "utils.h"
#include "draw.h"
#include "errors.h"
#include "terminal.h"

// --------------------------------------------------------------------------------------------- //

#define GAME_WIDTH 80

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
        if (STR_EQ(command, "s") || STR_EQ(command, "S") || STR_EQ(command, "start game") || STR_EQ(command, "START GAME")) {
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
        return load_game(height, GAME_WIDTH);
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

static page_return_code_t load_game(px_t height, px_t width)
{
    srand(time(NULL));

    pixel_buffer_t *pixel_buffer1 = create_pixel_buffer(height, width);
    pixel_buffer_t *pixel_buffer2 = create_pixel_buffer(height, width);

    if (pixel_buffer1 == NULL) { resolve_error(MEM_ALOC_FAILURE); return ERROR; }
    if (pixel_buffer2 == NULL) { resolve_error(MEM_ALOC_FAILURE); release_pixel_buffer(pixel_buffer1); return ERROR; }

    rectangle_t *ball = create_rectangle(37, 8, 1, 1, 2, 1, WHITE, "ball");
    rectangle_t *player = create_rectangle(width - 5, 5, 1, 9, 0, 0, GREEN, "player");
    rectangle_t *meteor = create_rectangle(15, 13, 2, 2, 0, 0, YELLOW, "meteor 1");
    rectangle_t *enemy = create_rectangle(5, 5, 1, 9, 0, 0, RED, "enemy");

    scene_t *scene = create_scene();
    add_to_scene(scene, ball); // TODO: make here and also when creating object macro which cleans all the mess
    add_to_scene(scene, player);
    add_to_scene(scene, meteor);
    add_to_scene(scene, enemy);

    clear_canvas();

    bool game_running = true;
    while (game_running) {

        set_cursor_at_beginning_of_canvas();
        reset_pixel_buffer(pixel_buffer2);

        int ready_fds = init_file_descriptor_monitor();
        if (ready_fds > 0) {

            int c = getchar();

            if (c == 'w' || c == 'W') {
                if ((int)player->position_y - 2 < 0) {
                    player->position_y = 0;
                } else {
                    player->position_y -= 2;
                }
            } else if (c == 's' || c == 'S') {
                player->position_y += 2;
                if (player->position_y > pixel_buffer1->height - player->side_length_2) {
                    player->position_y = pixel_buffer1->height - player->side_length_2;
                }
            } else if (c == 'q' || c == 'Q') {
                game_running = false;
            }
        }

        ball->position_x += ball->x_speed;
        ball->position_y += ball->y_speed;
        if(ball->position_x >= pixel_buffer1->width - 2 || ball->position_x <= 0) {
            ball->x_speed *= -1;
        }
        if (ball->position_y >= pixel_buffer1->height - ball->side_length_2 || ball->position_y <= 0) {
            ball->y_speed *= -1; 
        }        

        int ball_center = ball->position_y + (ball->side_length_2 / 2);
        int paddle_center = enemy->position_y + (enemy->side_length_2 / 2);

        if (ball_center < paddle_center) {
            enemy->position_y -= 2;
        } else if (ball_center > paddle_center) {
            enemy->position_y += 2;
        }

        enemy->position_y += rand() % 3;

        if (enemy->position_y < 0) {
            enemy->position_y = 0;
        } else if (enemy->position_y + enemy->side_length_2 > pixel_buffer1->height) {
            enemy->position_y = pixel_buffer1->height - enemy->side_length_2;
        }

        compute_object_pixels_in_buffer(pixel_buffer2, player, RECTANGLE);
        compute_object_pixels_in_buffer(pixel_buffer2, meteor, RECTANGLE);
        compute_object_pixels_in_buffer(pixel_buffer2, enemy, RECTANGLE);
        ID_t collision_ID = compute_object_pixels_in_buffer(pixel_buffer2, ball, RECTANGLE);

        if (collision_ID == enemy->ID || collision_ID == player->ID) {

            int paddle_center = player->position_y + (player->side_length_2 / 2);
            int ball_center = ball->position_y + (ball->side_length_2 / 2);
            int vertical_distance = ball_center - paddle_center;

            if (ball->y_speed == 0) {
                ball->y_speed = 1;
            }
            if (vertical_distance > 0) {
                ball->y_speed = abs(ball->y_speed);
            } else if (vertical_distance < 0) {
                ball->y_speed = -1 * abs(ball->y_speed);
            } else {
                ball->y_speed = 0 + rand() % 1;
            }

            ball->x_speed = -ball->x_speed;
        }

        if (ball->position_x <= 2) {
            game_running = false;
        } else if (ball->position_x + ball->side_length_1 >= pixel_buffer1->width) {
            game_running = false;
        }

        if (collision_ID == meteor->ID) {
            meteor->colour = BLACK;
            meteor->position_x = (rand() % (width - 10)) + 5;
            meteor->position_y = (rand() % (height + 10)) + 5;
            meteor->colour = YELLOW;
        }

        pixel_buffer_t *tmp_buffer = pixel_buffer1;
        pixel_buffer1 = pixel_buffer2;
        pixel_buffer2 = tmp_buffer;
        render_graphics(pixel_buffer1, scene);

        usleep(70000);
    }

    release_scene(scene);
    release_pixel_buffer(pixel_buffer1);
    release_pixel_buffer(pixel_buffer2);

    return SUCCESS_GAME;
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
    case NOT_FOUND_PAGE: return "Not Found page";
    case GAME_PAGE: return "Game page";
    case BACK_PAGE: return "Back page";
    case AFTER_GAME_PAGE: return "After Game page";
    default:
        break;
    }
}