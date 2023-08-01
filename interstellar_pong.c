#include <time.h>

#include "interstellar_pong.h"

// --------------------------------------------------------------------------------------------- //

static rectangle_t *find_object(game_t *game, const char *name);
static bool create_rectangle_and_add_it_to_scene(scene_t *scene, px_t position_x, px_t position_y, px_t side_length_1, px_t side_length_2, px_t x_speed, px_t y_speed, colour_t colour, const char *name);
static void simulate_enemy_paddle_movement(rectangle_t *enemy, rectangle_t *ball, px_t height);
static void move_ball(rectangle_t *ball);
static bool detect_collision(ID_t collision_ID, rectangle_t *object);
static void handleBallAndPaddleCollision(rectangle_t *ball, rectangle_t *paddle);
static bool checkBallBoundaryCollision(rectangle_t *ball, px_t width);
static void handleBallAndMeteorCollision(rectangle_t *meteor, px_t width, px_t height);

static ID_t get_ID(rectangle_t *object);
static void set_x_speed(rectangle_t *object, int speed);
static void set_y_speed(rectangle_t *object, int speed);
static int get_x_speed(rectangle_t *object);
static int get_y_speed(rectangle_t *object);
static int get_x_position(rectangle_t *object);
static void set_x_position(rectangle_t *object, px_t position);
static int get_y_position(rectangle_t *object);
static void set_y_position(rectangle_t *object, px_t position);
static int get_side_length_1(rectangle_t *object);
static int get_side_length_2(rectangle_t *object);
static colour_t get_colour(rectangle_t *object);
static void set_colour(rectangle_t *object, colour_t colour);
static char *get_name(rectangle_t *object);

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
        game->player = create_player("-", 0, 0, 0, 0, 0);
    } else {
        game->player = create_player(player->name, player->level, player->stone, player->copper, player->iron, player->gold);
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
        !create_rectangle_and_add_it_to_scene(scene, 15, 13, 2, 2, 0, 0, DARK_GRAY, "meteor_1") ||
        !create_rectangle_and_add_it_to_scene(scene, 50, 5, 1, 2, 0, 0, DARK_GRAY, "meteor_2") ||
        !create_rectangle_and_add_it_to_scene(scene, 5, 5, 1, 5, 0, 0, RED, "enemy")) {
        return NULL;
    }

    game->scene = scene;
    return game->scene;
}

void handle_event(game_t *game, char c)
{
    if (KEYBOARD_PRESSED(c, 'w') || KEYBOARD_PRESSED(c, 'W')) {

        set_y_position(find_object(game, "player"), get_y_position(find_object(game, "player")) - 2);
        if (get_y_position(find_object(game, "player")) - 2 < 0) {
            set_y_position(find_object(game, "player"), 0);
        }

    } else if (KEYBOARD_PRESSED(c, 's') || KEYBOARD_PRESSED(c, 'S')) {

        set_y_position(find_object(game, "player"), get_y_position(find_object(game, "player")) + 2);
        if (get_y_position(find_object(game, "player")) > game->height - get_side_length_2(find_object(game, "player"))) {
            set_y_position(find_object(game, "player"), game->height - get_side_length_2(find_object(game, "player")));
        }

    } else if (KEYBOARD_PRESSED(c, 'q') || KEYBOARD_PRESSED(c, 'Q')) {
        end_game(game);
    }
}

static void move_ball(rectangle_t *ball)
{
    ball->position_x += ball->x_speed;
    ball->position_y += ball->y_speed;
}

static void bounce_ball(rectangle_t *ball, px_t width, px_t height)
{
    if(ball->position_x >= width - 2 || ball->position_x <= 0) {
        set_x_speed(ball, get_x_speed(ball) * (-1));
    }
    if (ball->position_y >= height - ball->side_length_2 || ball->position_y <= 0) {
        set_y_speed(ball, get_y_speed(ball) * (-1));
    }     
}

static void simulate_enemy_paddle_movement(rectangle_t *enemy, rectangle_t *ball, px_t height)
{
    int ball_center = get_y_position(ball) + (get_side_length_2(ball) / 2);
    int paddle_center = get_y_position(enemy) + (get_side_length_2(enemy) / 2);

    if (ball_center < paddle_center) {
        set_y_position(enemy, get_y_position(enemy) - 2);
    } else if (ball_center > paddle_center) {
        set_y_position(enemy, get_y_position(enemy) + 2);
    }

    set_y_position(enemy, get_y_position(enemy) + rand() % 3);

    if (get_y_position(enemy) < 0) {
        set_y_position(enemy, 0);
    } else if (get_y_position(enemy) + get_side_length_2(enemy) > height) {
        set_y_position(enemy, height - get_side_length_2(enemy));
    }
}

static bool detect_collision(ID_t collision_ID, rectangle_t *object)
{
    return collision_ID == get_ID(object);
}

static void handleBallAndPaddleCollision(rectangle_t *ball, rectangle_t *paddle)
{
    int paddle_center = paddle->position_y + (paddle->side_length_2 / 2);
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
        ball->y_speed = 0 + (rand() % 1) * (((rand() % 2) == 0) ? 1 : -1);
    }

    ball->x_speed = -1 * ball->x_speed;
}

static bool checkBallBoundaryCollision(rectangle_t *ball, px_t width)
{
    if (get_x_position(ball) <= 2) {
        return false;
    } else if (get_x_position(ball) + get_side_length_1(ball) >= width) {
        return false;
    }

    return true;
}

static void handleBallAndMeteorCollision(rectangle_t *meteor, px_t width, px_t height)
{
    set_colour(meteor, DARK_GRAY);
    set_x_position(meteor, (rand() % (width - 10)) + 5);
    set_y_position(meteor, (rand() % (height - 10)) + 5);
}

scene_t *update_scene(game_t *game, pixel_buffer_t *pixel_buffer)
{
    move_ball(find_object(game, "ball"));
    bounce_ball(find_object(game, "ball"), game->width, game->height);
    simulate_enemy_paddle_movement(find_object(game, "enemy"), find_object(game, "ball"), game->height);

    // put objects pixel in pixel buffer
    (void)compute_object_pixels_in_buffer(pixel_buffer, find_object(game, "player"), RECTANGLE);
    (void)compute_object_pixels_in_buffer(pixel_buffer, find_object(game, "meteor_1"), RECTANGLE);
    (void)compute_object_pixels_in_buffer(pixel_buffer, find_object(game, "meteor_2"), RECTANGLE);
    (void)compute_object_pixels_in_buffer(pixel_buffer, find_object(game, "enemy"), RECTANGLE);

    // collision detection and handling
    ID_t collision_ID = compute_object_pixels_in_buffer(pixel_buffer, find_object(game, "ball"), RECTANGLE);
    if (detect_collision(collision_ID, find_object(game, "player"))) {
        handleBallAndPaddleCollision(find_object(game, "ball"), find_object(game, "player"));
    }
    if (detect_collision(collision_ID, find_object(game, "enemy"))) {
        handleBallAndPaddleCollision(find_object(game, "ball"), find_object(game, "enemy"));
    }
    if (detect_collision(collision_ID, find_object(game, "meteor_1"))) {
        handleBallAndMeteorCollision(find_object(game, "meteor_1"), game->width, game->height);
    }
    if (detect_collision(collision_ID, find_object(game, "meteor_2"))) {
        handleBallAndMeteorCollision(find_object(game, "meteor_2"), game->width, game->height);
    }

    // check ball and bouderies
    if (!checkBallBoundaryCollision(find_object(game, "ball"), game->width)) {
        end_game(game);
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

// ------------------------------------------- GETTERS & SETTERS ------------------------------- //

static ID_t get_ID(rectangle_t *object)
{
    return object->ID;
}

static void set_x_speed(rectangle_t *object, int speed)
{
    object->x_speed = speed;
}

static int get_x_speed(rectangle_t *object)
{
    return object->x_speed;
}

static void set_y_speed(rectangle_t *object, int speed)
{
    object->y_speed = speed;
}

static int get_y_speed(rectangle_t *object)
{
    return object->y_speed;
}

static int get_x_position(rectangle_t *object)
{
    return object->position_x;
}

static void set_x_position(rectangle_t *object, px_t position)
{
    object->position_x = position;
}

static int get_y_position(rectangle_t *object)
{
    return object->position_y;
}

static void set_y_position(rectangle_t *object, px_t position)
{
    object->position_y = position;
}

static int get_side_length_1(rectangle_t *object)
{
    return object->side_length_1;
}

static int get_side_length_2(rectangle_t *object)
{
    return object->side_length_2;
}

static colour_t get_colour(rectangle_t *object)
{
    return object->colour;
}

static void set_colour(rectangle_t *object, colour_t colour)
{
    object->colour = colour;
}

static char *get_name(rectangle_t *object)
{
    char *name = malloc(strlen(object->name) + 1);
    if (name == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        return NULL;
    }

    strcpy(name, object->name);
}