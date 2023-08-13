#include <time.h>

#include "interstellar_pong.h"

// ---------------------------------- STATIC DECLARATIONS--------------------------------------- //

static bool create_rectangle_and_add_it_to_scene(scene_t *scene, px_t position_x, px_t position_y, px_t side_length_1, px_t side_length_2, px_t x_speed, px_t y_speed, colour_t colour, const char *name);
static void simulate_enemy_paddle_movement(rectangle_t *enemy, rectangle_t *ball, px_t height);
static void handle_ball_and_paddle_collision(rectangle_t *ball, rectangle_t *paddle);
static void handle_ball_and_meteor_collision(rectangle_t *meteor, game_t *game);
static bool check_ball_boundary_collision(rectangle_t *ball, game_t *game);
static bool detect_collision(ID_t collision_ID, rectangle_t *object);
static void bounce_ball(rectangle_t *ball, px_t width, px_t height);
static rectangle_t *find_object(game_t *game, const char *name);
static void move_ball(rectangle_t *ball);

static int get_width(game_t *game);
static int get_height(game_t *game);
static ID_t get_ID(rectangle_t *object);
static int get_game_ticks(game_t *game);
static char *get_name(rectangle_t *object);
static int get_x_speed(rectangle_t *object);
static int get_y_speed(rectangle_t *object);
static void increment_game_ticks(game_t *game);
static int get_x_position(rectangle_t *object);
static int get_y_position(rectangle_t *object);
static colour_t get_colour(rectangle_t *object);
static int get_side_length_1(rectangle_t *object);
static int get_side_length_2(rectangle_t *object);
static void set_x_speed(rectangle_t *object, int speed);
static void set_y_speed(rectangle_t *object, int speed);
static void set_colour(rectangle_t *object, colour_t colour);
static void set_x_position(rectangle_t *object, px_t position);
static void set_y_position(rectangle_t *object, px_t position);

// ----------------------------------------- PROGRAM-------------------------------------------- //

game_t *init_game(player_t *player, px_t height, px_t width)
{
    game_t *game = malloc(sizeof(game_t));
    if (game == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        return NULL;
    }

    game->height = height;
    game->width = width;
    game->game_state = STOPPED;
    game->scene = NULL;
    game->game_ticks = 0;
    game->enemy = create_player("enemy", 0, 0, 0, 0, 0);
    if (game->enemy == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        free(game);
        return NULL;
    } 

    if (player == NULL) {
        game->player = create_player(";", 0, 0, 0, 0, 0);
    } else {
        game->player = create_player(player->name, player->level, player->stone, player->copper, player->iron, player->gold);
    }

    if (game->player == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        free(game->enemy);
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
        release_player(game->enemy);
        release_scene(game->scene);
        free(game);
    }
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

scene_t *update_scene(game_t *game, pixel_buffer_t *pixel_buffer)
{
    move_ball(find_object(game, "ball"));
    bounce_ball(find_object(game, "ball"), game->width, game->height);
    simulate_enemy_paddle_movement(find_object(game, "enemy"), find_object(game, "ball"), game->height);

    // put objects pixel in pixel buffer
    (void)compute_object_pixels_in_buffer(pixel_buffer, find_object(game, "player"));
    (void)compute_object_pixels_in_buffer(pixel_buffer, find_object(game, "meteor_1"));
    (void)compute_object_pixels_in_buffer(pixel_buffer, find_object(game, "meteor_2"));
    (void)compute_object_pixels_in_buffer(pixel_buffer, find_object(game, "enemy"));

    // collision detection and handling
    ID_t collision_ID = compute_object_pixels_in_buffer(pixel_buffer, find_object(game, "ball"));
    if (detect_collision(collision_ID, find_object(game, "player"))) {
        increment_game_ticks(game);
        handle_ball_and_paddle_collision(find_object(game, "ball"), find_object(game, "player"));
    }
    if (detect_collision(collision_ID, find_object(game, "enemy"))) {
        increment_game_ticks(game);
        handle_ball_and_paddle_collision(find_object(game, "ball"), find_object(game, "enemy"));
    }
    if (detect_collision(collision_ID, find_object(game, "meteor_1"))) {
        handle_ball_and_meteor_collision(find_object(game, "meteor_1"), game);
    }
    if (detect_collision(collision_ID, find_object(game, "meteor_2"))) {
        handle_ball_and_meteor_collision(find_object(game, "meteor_2"), game);
    }

    // check ball and bouderies
    if (!check_ball_boundary_collision(find_object(game, "ball"), game)) {
        increment_game_ticks(game);
    }

    // end game
    if (game->enemy->hearts <= 0 || game->player->hearts <= 0) {
        end_game(game);
    }

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

/**
 * @brief Moves the ball by updating its position based on its speed.
 * 
 * @param ball The ball rectangle to move.
 */
static void move_ball(rectangle_t *ball)
{
    set_x_position(ball, get_x_position(ball) + get_x_speed(ball));
    set_y_position(ball, get_y_position(ball) + get_y_speed(ball));
}

/**
 * @brief Bounces the ball off the game boundaries if it hits them.
 * 
 * @param ball The ball rectangle to check and update.
 * @param width The width of the game area.
 * @param height The height of the game area.
 */
static void bounce_ball(rectangle_t *ball, px_t width, px_t height)
{
    if(get_x_position(ball) >= width - 2 || get_x_position(ball) <= 0) {
        set_x_speed(ball, get_x_speed(ball) * (-1));
    }
    if (get_y_position(ball) >= height - get_side_length_2(ball) || get_y_position(ball) <= 0) {
        set_y_speed(ball, get_y_speed(ball) * (-1));
    }     
}

/**
 * @brief Simulates the movement of the enemy paddle based on ball position.
 * 
 * @param enemy The enemy paddle rectangle.
 * @param ball The ball rectangle.
 * @param height The height of the game area.
 */
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

/**
 * @brief Detects if a collision with the specified collision ID occurred.
 * 
 * @param collision_ID The collision ID to compare with.
 * @param object The rectangle object to check for collision.
 * @return True if collision occurred, false otherwise.
 */
static bool detect_collision(ID_t collision_ID, rectangle_t *object)
{
    return collision_ID == get_ID(object);
}

/**
 * @brief Handles the collision between the ball and a paddle.
 * 
 * @param ball The ball rectangle.
 * @param paddle The paddle rectangle.
 */
static void handle_ball_and_paddle_collision(rectangle_t *ball, rectangle_t *paddle)
{
    int paddle_center = get_y_position(paddle) + (get_side_length_2(paddle) / 2);
    int ball_center = get_y_position(ball) + (get_side_length_2(ball) / 2);
    int vertical_distance = ball_center - paddle_center;

    if (get_y_speed(ball) == 0) {
        set_y_speed(ball, 1);
    }
    if (vertical_distance > 0) {
        set_y_speed(ball, abs(get_y_speed(ball)));
    } else if (vertical_distance < 0) {
        set_y_speed(ball, -1 * abs(get_y_speed(ball)));
    } else {
        set_y_speed(ball, (rand() % 1) * (((rand() % 2) == 0) ? 1 : -1));
    }

    set_x_speed(ball, -1 * get_x_speed(ball));
}

/**
 * @brief Checks if the ball collides with the game boundaries.
 *        Updates player and enemy hearts accordingly.
 * 
 * @param ball The ball rectangle.
 * @param game The game instance.
 * @return True if the ball is within boundaries, false otherwise.
 */
static bool check_ball_boundary_collision(rectangle_t *ball, game_t *game)
{
    bool is_in_bound = true;
    if (get_x_position(ball) <= 2) {
        is_in_bound = false;
    } else if (get_x_position(ball) + get_side_length_1(ball) >= game->width) {
        is_in_bound = false;
    }

    if (!is_in_bound) {
        if (get_game_ticks(game) % 2 == 0) {
            game->player->hearts--;
        } else {
            game->enemy->hearts--;
        }
    }

    return is_in_bound;
}

/**
 * @brief Handles the collision between the ball and a meteor object.
 * 
 * @param meteor The meteor rectangle.
 * @param game The game instance.
 */
static void handle_ball_and_meteor_collision(rectangle_t *meteor, game_t *game)
{
    if (get_game_ticks(game) % 2 != 0) {
        game->player->stone++;
    }

    set_colour(meteor, DARK_GRAY);
    set_x_position(meteor, (rand() % (get_width(game) - 10)) + 5);
    set_y_position(meteor, (rand() % (get_height(game) - 10)) + 5);
}

/**
 * @brief Finds and returns the rectangle object with the specified name in the game scene.
 * 
 * @param game The game instance.
 * @param name The name of the rectangle object to find.
 * @return A pointer to the found rectangle object, or NULL if not found.
 */
static rectangle_t *find_object(game_t *game, const char *name)
{
    for (int i = 0; i < game->scene->number_of_objects; ++i) {
        if (STR_EQ(game->scene->scene[i]->name, name)) {
            return game->scene->scene[i];
        }
    }
    return NULL;
}

/**
 * @brief Creates a rectangle object, adds it to the scene, and checks for success.
 * 
 * @param scene The scene to which to add the rectangle object.
 * @param position_x The x-coordinate of the rectangle's position.
 * @param position_y The y-coordinate of the rectangle's position.
 * @param side_length_1 The length of the rectangle's first side.
 * @param side_length_2 The length of the rectangle's second side.
 * @param x_speed The horizontal speed of the rectangle.
 * @param y_speed The vertical speed of the rectangle.
 * @param colour The colour of the rectangle.
 * @param name The name of the rectangle object.
 * @return True if creation and addition were successful, false otherwise.
 */
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

/**
 * @brief Gets the ID of the rectangle object.
 * 
 * @param object The rectangle object.
 * @return The object's ID.
 */
static ID_t get_ID(rectangle_t *object)
{
    return object->ID;
}

/**
 * @brief Sets the horizontal speed of the specified rectangle object.
 * 
 * @param object The rectangle object to modify.
 * @param speed The new horizontal speed to set.
 */
static void set_x_speed(rectangle_t *object, int speed)
{
    object->x_speed = speed;
}

/**
 * @brief Retrieves the horizontal speed of the specified rectangle object.
 * 
 * @param object The rectangle object to query.
 * @return The horizontal speed of the object.
 */
static int get_x_speed(rectangle_t *object)
{
    return object->x_speed;
}

/**
 * @brief Sets the vertical speed of the specified rectangle object.
 * 
 * @param object The rectangle object to modify.
 * @param speed The new vertical speed to set.
 */
static void set_y_speed(rectangle_t *object, int speed)
{
    object->y_speed = speed;
}

/**
 * @brief Retrieves the vertical speed of the specified rectangle object.
 * 
 * @param object The rectangle object to query.
 * @return The vertical speed of the object.
 */
static int get_y_speed(rectangle_t *object)
{
    return object->y_speed;
}

/**
 * @brief Retrieves the x-coordinate position of the specified rectangle object.
 * 
 * @param object The rectangle object to query.
 * @return The x-coordinate position of the object.
 */
static int get_x_position(rectangle_t *object)
{
    return object->position_x;
}

/**
 * @brief Sets the x-coordinate position of the specified rectangle object.
 * 
 * @param object The rectangle object to modify.
 * @param position The new x-coordinate position to set.
 */
static void set_x_position(rectangle_t *object, px_t position)
{
    object->position_x = position;
}

/**
 * @brief Retrieves the y-coordinate position of the specified rectangle object.
 * 
 * @param object The rectangle object to query.
 * @return The y-coordinate position of the object.
 */
static int get_y_position(rectangle_t *object)
{
    return object->position_y;
}

/**
 * @brief Sets the y-coordinate position of the specified rectangle object.
 * 
 * @param object The rectangle object to modify.
 * @param position The new y-coordinate position to set.
 */
static void set_y_position(rectangle_t *object, px_t position)
{
    object->position_y = position;
}

/**
 * @brief Retrieves the length of the first side of the specified rectangle object.
 * 
 * @param object The rectangle object to query.
 * @return The length of the first side of the object.
 */
static int get_side_length_1(rectangle_t *object)
{
    return object->side_length_1;
}

/**
 * @brief Retrieves the length of the second side of the specified rectangle object.
 * 
 * @param object The rectangle object to query.
 * @return The length of the second side of the object.
 */
static int get_side_length_2(rectangle_t *object)
{
    return object->side_length_2;
}

/**
 * @brief Retrieves the number of game ticks (updates) that have occurred.
 * 
 * @param game The game instance to query.
 * @return The number of game ticks.
 */
static int get_game_ticks(game_t *game)
{
    return game->game_ticks;
}

/**
 * @brief Increments the number of game ticks (updates) for the specified game instance.
 * 
 * @param game The game instance to update.
 */
static void increment_game_ticks(game_t *game)
{
    game->game_ticks++;
}

/**
 * @brief Retrieves the colour of the specified rectangle object.
 * 
 * @param object The rectangle object to query.
 * @return The colour of the object.
 */
static colour_t get_colour(rectangle_t *object)
{
    return object->colour;
}

/**
 * @brief Sets the colour of the specified rectangle object.
 * 
 * @param object The rectangle object to modify.
 * @param colour The new colour to set.
 */
static void set_colour(rectangle_t *object, colour_t colour)
{
    object->colour = colour;
}

/**
 * @brief Retrieves a dynamically allocated copy of the name of the specified rectangle object.
 * 
 * @param object The rectangle object to query.
 * @return A pointer to the dynamically allocated name string.
 * @note The returned pointer should be freed when no longer needed.
 */
static char *get_name(rectangle_t *object)
{
    char *name = malloc(strlen(object->name) + 1);
    if (name == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        return NULL;
    }

    strcpy(name, object->name);
}

/**
 * @brief Retrieves the width of the game window.
 * 
 * @param game The game object to query.
 * @return The width of the game.
 */
static int get_width(game_t *game)
{
    return game->width;
}

/**
 * @brief Retrieves the height of the game window.
 * 
 * @param game The game object to query.
 * @return The height of the game.
 */
static int get_height(game_t *game)
{
    return game->height;
}
