#include <time.h>

#include "interstellar_pong.h"

// --------------------------------------------------------------------------------------------- //

static rectangle_t *find_object(game_t *game, const char *name);
static bool create_rectangle_and_add_it_to_scene(scene_t *scene, px_t position_x, px_t position_y, px_t side_length_1, px_t side_length_2, px_t x_speed, px_t y_speed, colour_t colour, const char *name);

// --------------------------------------------------------------------------------------------- //

game_t *init_game(player_t *player, px_t height, px_t width)
{
    game_t *game = malloc(sizeof(game_t));
    if (game == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        return NULL;
    }

    game->game_state = STOPPED;
    game->height = height;
    game->width = width;
    game->scene = NULL;

    if (player == NULL) {
        game->player = create_player("-", 0, 0, 0, 0);
    } else {
        game->player = create_player(player->name, player->level, player->copper, player->iron, player->gold);
    }

    if (game->player == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        free(game);
        return NULL;
    }

    srand(time(NULL));
    return game;
}

void start_game(game_t *game)
{
    game->game_state = RUNNING;
}

scene_t *init_scene(game_t *game)
{
    scene_t *scene = create_scene();
    if (scene == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        return NULL;
    }

    if (!create_rectangle_and_add_it_to_scene(scene, 37, 8, 1, 1, 2, 1, WHITE, "ball") ||
        !create_rectangle_and_add_it_to_scene(scene, game->width - 5, 5, 1, 5, 0, 0, GREEN, "player") ||
        !create_rectangle_and_add_it_to_scene(scene, 15, 13, 2, 2, 0, 0, DARK_GRAY, "meteor") ||
        !create_rectangle_and_add_it_to_scene(scene, 5, 5, 1, 5, 0, 0, RED, "enemy")) {
        return NULL;
    }

    game->scene = scene;
    return game->scene;
}

void handle_event(game_t *game, char c)
{
    if (c == 'w' || c == 'W') {
        if ((int)find_object(game, "player")->position_y - 2 < 0) {
            find_object(game, "player")->position_y = 0;
        } else {
            find_object(game, "player")->position_y -= 2;
        }

    } else if (c == 's' || c == 'S') {
        find_object(game, "player")->position_y += 2;
        if (find_object(game, "player")->position_y > game->height - find_object(game, "player")->side_length_2) {
            find_object(game, "player")->position_y = game->height - find_object(game, "player")->side_length_2;
        }

    } else if (c == 'q' || c == 'Q') {
        end_game(game);
    }
}

scene_t *update_scene(game_t *game, pixel_buffer_t *pixel_buffer)
{
    // move ball
    find_object(game, "ball")->position_x += find_object(game, "ball")->x_speed;
    find_object(game, "ball")->position_y += find_object(game, "ball")->y_speed;

    // ball bounce off walls
    if(find_object(game, "ball")->position_x >= game->width - 2 || find_object(game, "ball")->position_x <= 0) {
        find_object(game, "ball")->x_speed *= -1;
    }
    if (find_object(game, "ball")->position_y >= game->height - find_object(game, "ball")->side_length_2 || find_object(game, "ball")->position_y <= 0) {
        find_object(game, "ball")->y_speed *= -1; 
    }        

    // move enemy paddle
    int ball_center = find_object(game, "ball")->position_y + (find_object(game, "ball")->side_length_2 / 2);
    int paddle_center = find_object(game, "enemy")->position_y + (find_object(game, "enemy")->side_length_2 / 2);

    if (ball_center < paddle_center) {
        find_object(game, "enemy")->position_y -= 2;
    } else if (ball_center > paddle_center) {
        find_object(game, "enemy")->position_y += 2;
    }

    find_object(game, "enemy")->position_y += rand() % 3;

    if (find_object(game, "enemy")->position_y < 0) {
        find_object(game, "enemy")->position_y = 0;
    } else if (find_object(game, "enemy")->position_y + find_object(game, "enemy")->side_length_2 > game->height) {
        find_object(game, "enemy")->position_y = game->height - find_object(game, "enemy")->side_length_2;
    }

    // put objects pixel in pixel buffer
    compute_object_pixels_in_buffer(pixel_buffer, find_object(game, "player"), RECTANGLE);
    compute_object_pixels_in_buffer(pixel_buffer, find_object(game, "meteor"), RECTANGLE);
    compute_object_pixels_in_buffer(pixel_buffer, find_object(game, "enemy"), RECTANGLE);

    // collision detection
    ID_t collision_ID = compute_object_pixels_in_buffer(pixel_buffer, find_object(game, "ball"), RECTANGLE);

    if (collision_ID == find_object(game, "player")->ID || collision_ID == find_object(game, "enemy")->ID) {

        int paddle_center = find_object(game, "player")->position_y + (find_object(game, "player")->side_length_2 / 2);
        int ball_center = find_object(game, "ball")->position_y + (find_object(game, "ball")->side_length_2 / 2);
        int vertical_distance = ball_center - paddle_center;

        if (find_object(game, "ball")->y_speed == 0) {
            find_object(game, "ball")->y_speed = 1;
        }
        if (vertical_distance > 0) {
            find_object(game, "ball")->y_speed = abs(find_object(game, "ball")->y_speed);
        } else if (vertical_distance < 0) {
            find_object(game, "ball")->y_speed = -1 * abs(find_object(game, "ball")->y_speed);
        } else {
            find_object(game, "ball")->y_speed = 0 + rand() % 1;
        }

        find_object(game, "ball")->x_speed = -find_object(game, "ball")->x_speed;
    }

    // ball gets out of bounds
    if (find_object(game, "ball")->position_x <= 2) {
        end_game(game);
    } else if (find_object(game, "ball")->position_x + find_object(game, "ball")->side_length_1 >= game->width) {
        end_game(game);
    }

    // meteor collision
    if (collision_ID == find_object(game, "meteor")->ID) {
        find_object(game, "meteor")->colour = BLACK;
        find_object(game, "meteor")->position_x = (rand() % (game->width - 10)) + 5;
        find_object(game, "meteor")->position_y = (rand() % (game->height + 10)) + 5;
        find_object(game, "meteor")->colour = DARK_GRAY;
    }

    return game->scene;
}

game_state_t get_game_state(game_t *game)
{
    return game->game_state;
}

void end_game(game_t *game)
{
    game->game_state = STOPPED;
}

void release_game(game_t *game)
{
    if (game != NULL) {
        release_player(game->player);
        release_scene(game->scene);
        free(game);
    }
}

static rectangle_t *find_object(game_t *game, const char *name)
{
    for (int i = 0; i < game->scene->number_of_objects; ++i) {
        if (STR_EQ(game->scene->scene[i]->name, name)) {
            return game->scene->scene[i];
        }
    }
    return NULL;
}

static bool create_rectangle_and_add_it_to_scene(scene_t *scene, px_t position_x, px_t position_y, px_t side_length_1, px_t side_length_2, px_t x_speed, px_t y_speed, colour_t colour, const char *name)
{
    rectangle_t *object = create_rectangle(position_x, position_y, side_length_1, side_length_2, x_speed, y_speed, colour, name);
    if (object == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        return false;
    }

    if (add_to_scene(scene, object) == NULL) {
        release_scene(scene);
        resolve_error(MEM_ALOC_FAILURE);
        return false;
    }

    return true;
}