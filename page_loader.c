#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "page_loader.h"
#include "utils.h"
#include "draw.h"
#include "errors.h"
#include "terminal.h"

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
        return load_game(height, width);
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

void printArrayToFile(const char* filename, pixel_buffer_t *pixel_buffer) {

    FILE* file = fopen(filename, "w");

    if (file == NULL) {
        printf("Error opening the file.\n");
        return;
    }

    for (unsigned int i = 0; i < pixel_buffer->height; ++i) {
        for (unsigned int j = 0; j < pixel_buffer->width; ++j) {

            int pixel = pixel_buffer->buff[i * pixel_buffer->width + j];
            
            switch (pixel) {
                case BLACK:
                    fprintf(file, " "); break;
                case WHITE:
                    fprintf(file, "1"); break;
                case RED:
                    fprintf(file, "2"); break;
                case GREEN:
                    fprintf(file, "3"); break;
                case BLUE:
                    fprintf(file, "4"); break;
                case YELLOW:
                    fprintf(file, "5"); break;
                default:
                    fprintf(file, " "); break;
            }
        }
        putchar('\n');
    }

    fclose(file);
}

static page_return_code_t load_game(px_t height, px_t width)
{
    pixel_buffer_t *pixel_buffer1 = create_pixel_buffer(height, width);
    pixel_buffer_t *pixel_buffer2 = create_pixel_buffer(height, width);

    if (pixel_buffer1 == NULL) { resolve_error(MEM_ALOC_FAILURE); return ERROR; }
    if (pixel_buffer2 == NULL) { resolve_error(MEM_ALOC_FAILURE); release_pixel_buffer(pixel_buffer1); return ERROR; }

    rectangle_t *ball = create_rectangle(37, 8, 1, 1, 1, 2, WHITE);
    if (ball == NULL) { resolve_error(MEM_ALOC_FAILURE); release_pixel_buffer(pixel_buffer1), release_pixel_buffer(pixel_buffer2); return ERROR; }

    rectangle_t *player = create_rectangle(102, 5, 1, 7, 0, 0, GREEN);
    if (player == NULL) { resolve_error(MEM_ALOC_FAILURE); release_pixel_buffer(pixel_buffer1), release_pixel_buffer(pixel_buffer2); release_rectangle(ball); return ERROR; }

    rectangle_t *meteror = create_rectangle(40, 10, 2, 2, 0, 0, YELLOW);
    rectangle_t *enemy = create_rectangle(5, 5, 1, 7, 0, 0, RED);

    clear_canvas();

    bool game_running = true;
    while (game_running) {

        set_cursor_at_beginning_of_canvas();
        reset_pixel_buffer(pixel_buffer2);

        int ready_fds = init_file_descriptor_monitor();
        if (ready_fds > 0) { // is there is some activity on standart input

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

        compute_object_pixels_in_buffer(pixel_buffer2, player, RECTANGLE);
        compute_object_pixels_in_buffer(pixel_buffer2, meteror, RECTANGLE);
        compute_object_pixels_in_buffer(pixel_buffer2, enemy, RECTANGLE);
        colour_t collision_colour = compute_object_pixels_in_buffer(pixel_buffer2, ball, RECTANGLE);

        if (collision_colour == GREEN || collision_colour == RED) {

            int paddle_center = ball->position_y + (ball->side_length_2 / 2);
            int ball_center = ball->position_y + (ball->side_length_2);
            int vertical_distance = ball_center - paddle_center;

            if (vertical_distance > 0) {
                ball->y_speed = abs(ball->y_speed);
            } else {
                ball->y_speed = -abs(ball->y_speed);
            }

            ball->x_speed = -ball->x_speed;
        }

        if (collision_colour == YELLOW) {
            meteror->colour = BLACK;
        }

        pixel_buffer_t *tmp_buffer = pixel_buffer1;
        pixel_buffer1 = pixel_buffer2;
        pixel_buffer2 = tmp_buffer;
        render_graphics(pixel_buffer1);

        usleep(70000);
    }

    release_rectangle(ball);
    release_rectangle(player);
    release_rectangle(meteror);
    release_rectangle(enemy);
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