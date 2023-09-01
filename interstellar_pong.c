#include <time.h>
#include <unistd.h>

#include "interstellar_pong.h"
#include "paths.h"

// ---------------------------------------- MACROS --------------------------------------------- //

#define BALL_INIT_X_COORD 37
#define BALL_INIT_Y_COORD 8
#define PLAYER_INIT_X_COORD (game->width - 5)
#define PLAYER_INIT_Y_COORD 5
#define ENEMY_INIT_X_COORD 5
#define ENEMY_INIT_Y_COORD 5

#define SMALL_METEOR_SIZE 1
#define BIG_METEOR_SIZE 2

// ---------------------------------- STATIC DECLARATIONS--------------------------------------- //

static bool create_rectangle_and_add_it_to_scene(scene_t *scene, px_t position_x, px_t position_y, px_t width, px_t height, px_t x_speed, px_t y_speed, colour_t colour, const char *name);
static void set_meteor_properties(rectangle_t *meteor, int player_level, levels_table_t *levels, materials_table_t *materials, int width, int height);
static material_type_t count_meteor_material_from_level(level_row_t level);
static void simulate_enemy_paddle_movement(rectangle_t *enemy, rectangle_t *ball, px_t height);
static bool convert_line_into_material_data(materials_table_t *table, char *line, int counter);
static void handle_ball_and_paddle_collision(rectangle_t *ball, rectangle_t *paddle);
static void handle_ball_and_meteor_collision(rectangle_t *meteor, game_t *game);
static int compute_cumulative_distribution(const int *probabilities, int count);
static void set_meteor_shape(rectangle_t *meteor, materials_table_t *materials);
static void set_meteor_size(rectangle_t *meteor, materials_table_t *materials);
static bool convert_line_into_level_data(levels_table_t *table, char *line);
static bool check_ball_boundary_collision(rectangle_t *ball, game_t *game);
static void update_player_resources(rectangle_t *meteor, player_t *player);
static int get_index_based_on_material_type(material_type_t material);
static bool detect_collision(ID_t collision_ID, rectangle_t *object);
static void bounce_ball(rectangle_t *ball, px_t width, px_t height);
static material_type_t get_material_type_based_on_index(int index);
static rectangle_t *find_object(game_t *game, const char *name);
static material_shape_t get_meteors_shape(rectangle_t *meteor);
static void set_objects_to_initial_position(game_t *game);
static void test_meteors_generator(int tested_level); // TODO this should not be compiled in release mode
static void swap_sides(rectangle_t *meteor);
static void reset_game_ticks(game_t *game);
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
static int get_rectangle_width(rectangle_t *object);
static int get_rectangle_height(rectangle_t *object);
static void set_x_speed(rectangle_t *object, int speed);
static void set_y_speed(rectangle_t *object, int speed);
static void set_colour(rectangle_t *object, colour_t colour);
static void set_rectangle_width(rectangle_t *object, px_t size);
static void set_rectangle_height(rectangle_t *object, px_t size);
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
    game->game_state = TERMINATED;
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

    game->materials_table = NULL;
    game->levels_table = NULL;

    if (!load_extern_game_data(GAME_DATA_PATH, &game->materials_table, &game->levels_table)) {
        release_player(game->player);
        release_player(game->enemy);
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
    game->game_state = TERMINATED;
}

void stop_game(game_t *game)
{
    game->game_state = STOPPED;
}

void release_game(game_t *game)
{
    if (game != NULL) {
        release_player(game->player);
        release_player(game->enemy);
        release_scene(game->scene);
        release_materials_table(game->materials_table);
        release_levels_table(game->levels_table);
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

    if (!create_rectangle_and_add_it_to_scene(scene, BALL_INIT_X_COORD, BALL_INIT_Y_COORD, 1, 1, 2, 1, WHITE, "ball") ||
        !create_rectangle_and_add_it_to_scene(scene, PLAYER_INIT_X_COORD, PLAYER_INIT_Y_COORD, 1, 5, 0, 0, GREEN, "player") ||
        !create_rectangle_and_add_it_to_scene(scene, 15, 13, 0, 0, 0, 0, (colour_t)STONE, "meteor_1") ||
        !create_rectangle_and_add_it_to_scene(scene, 50, 5, 0, 0, 0, 0, (colour_t)STONE, "meteor_2") ||
        !create_rectangle_and_add_it_to_scene(scene, ENEMY_INIT_X_COORD, ENEMY_INIT_Y_COORD, 1, 5, 0, 0, RED, "enemy")) {
        return NULL;
    }

    game->scene = scene;

    set_meteor_properties(find_object(game, "meteor_1"), game->player->level, game->levels_table, game->materials_table, get_width(game), get_height(game));
    set_meteor_properties(find_object(game, "meteor_2"), game->player->level, game->levels_table, game->materials_table, get_width(game), get_height(game));

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

    // update meteor if needed
    if (game->game_ticks > 4) {
        increment_game_ticks(game);
        reset_game_ticks(game);
        set_meteor_properties(find_object(game, "meteor_1"), game->player->level, game->levels_table, game->materials_table, get_width(game), get_height(game));
        set_meteor_properties(find_object(game, "meteor_2"), game->player->level, game->levels_table, game->materials_table, get_width(game), get_height(game));
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
        if (get_y_position(find_object(game, "player")) > game->height - get_rectangle_height(find_object(game, "player"))) {
            set_y_position(find_object(game, "player"), game->height - get_rectangle_height(find_object(game, "player")));
        }

    } else if (KEYBOARD_PRESSED(c, 'q') || KEYBOARD_PRESSED(c, 'Q')) {
        end_game(game);
    }
}

bool load_extern_game_data(const char *file_path, materials_table_t **materials_table, levels_table_t **levels_table)
{
    const char COMMENT = '#';

    *materials_table = create_materials_table();
    if (materials_table == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        return false;
    }

    *levels_table = create_levels_table();
    if (levels_table == NULL) {
        release_materials_table(*materials_table);
        resolve_error(MEM_ALOC_FAILURE);
        return false;
    }

    if (access(file_path, F_OK) != 0) {
        resolve_error(MISSING_DATA_FILE);
        release_materials_table(*materials_table);
        release_levels_table(*levels_table);
        return false;
    }

    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        resolve_error(UNOPENABLE_FILE);
        release_materials_table(*materials_table);
        release_levels_table(*levels_table);
        return false;
    }

    char* line = NULL;
    size_t line_length = 0;
    ssize_t bytes_read;
    int counter = 0;

    while ((bytes_read = getline(&line, &line_length, file)) != -1) {

        complete_strip(line);

        if (STR_EQ(line, "") || line[0] == COMMENT) {
            continue;
        }

        bool converting_return_code;
        if (counter < 4) {
            converting_return_code = convert_line_into_material_data(*materials_table, line, counter);
        } else if (counter > 3 && counter < 8) {
            converting_return_code = convert_line_into_level_data(*levels_table, line);
        } else {
            converting_return_code = false;
            resolve_error(INVALID_DATA_IN_FILE);
        }

        if (!converting_return_code) {
            free(line);
            release_materials_table(*materials_table);
            release_levels_table(*levels_table);
            fclose(file);
            return false;
        }
        counter++;
    }

    free(line);
    fclose(file);

    if (counter != 8) {
        resolve_error(INVALID_DATA_IN_FILE);
        release_materials_table(*materials_table);
        release_levels_table(*levels_table);
        return false;
    }

    return true;
}

/**
 * @brief Resets the game's tick count.
 *
 * This function resets the game's tick count. If the current tick count is even,
 * it sets the tick count to -1. Otherwise, it sets it to 0.
 *
 * @param game A pointer to the game structure.
 */
static void reset_game_ticks(game_t *game)
{
    if (get_game_ticks(game) % 2 == 0) {
        game->game_ticks = -1;
    } else {
        game->game_ticks = 0;
    }
}

/**
 * @brief Gets the material type based on the given index.
 *
 * This function maps an index to a specific material type.
 *
 * @param index The index to map to a material type.
 * @return The material type corresponding to the index. If the index is out of range, STONE is returned as a default.
 */
static material_type_t get_material_type_based_on_index(int index)
{
    switch (index)
    {
    case 0: return STONE;
    case 1: return COPPER;
    case 2: return IRON;
    case 3: return GOLD;
    default: return STONE;
    }
}

/**
 * @brief Gets the index based on the given material type.
 *
 * This function maps a materil type to a specific index.
 *
 * @param material The material type to map to an index.
 * @return The index corresponding to the material type. If the material type is not found, 0 is returned as a default.
 */
static int get_index_based_on_material_type(material_type_t material)
{
    switch (material)
    {
    case STONE: return 0;
    case COPPER: return 1;
    case IRON: return 2;
    case GOLD: return 3;
    default: return 0;
    }
}

/**
 * @brief Computes the cumulative distribution and choose a random index based on probabilities.
 *
 * This function computes the cumulative distribution of the given probabilities array and then chooses a random index
 * based on these cumulative probabilities. The probabilities array is assumed to contain integer values representing
 * probabilities for each element.
 *
 * @param probabilities An array of probabilities for each element.
 * @param count The number of elements in the probabilities array.
 * @return The chosen index based on probabilities (starting on 0), or -1 if an error occurs.
 */
static int compute_cumulative_distribution(const int *probabilities, int count)
{
    int cumulative_probabilities[count];

    int cumulative = 0;
    for (int i = 0; i < count; ++i) {
        cumulative += probabilities[i];
        cumulative_probabilities[i] = cumulative;
    }

    int random_number = rand() % 100 + 1;

    for (int i = 0; i < count; ++i) {
        if (random_number <= cumulative_probabilities[i]) {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Sets the size of a meteor object based on material probabilities.
 *
 * This function calculates the size of a meteor object based on the given materials' probabilities.
 *
 * @param meteor Pointer to the rectangle_t structure representing the meteor object.
 * @param materials Pointer to the materials_table_t structure containing material probabilities.
 */
static void set_meteor_size(rectangle_t *meteor, materials_table_t *materials)
{
    const int SIZES_COUNT = 2;
    int probabilities[SIZES_COUNT];
    probabilities[0] = materials->materials[get_index_based_on_material_type((material_type_t)get_colour(meteor))].prob_size_1_px_t;
    probabilities[1] = materials->materials[get_index_based_on_material_type((material_type_t)get_colour(meteor))].prob_size_2_px_t;
    
    int size_index = compute_cumulative_distribution(probabilities, SIZES_COUNT);

    int width, height;
    switch (size_index)
    {
    case 0:
        width = SMALL_METEOR_SIZE;
        height = SMALL_METEOR_SIZE;
        break;
    case 1:
        width = BIG_METEOR_SIZE;
        height = BIG_METEOR_SIZE;
        break;
    default:
        width = SMALL_METEOR_SIZE;
        height = SMALL_METEOR_SIZE;
        break;
    }

    set_rectangle_width(meteor, width);
    set_rectangle_height(meteor, height);
}

/**
 * Swaps the sides of a meteor with a given probability 50%.
 *
 * @param meteor A pointer to the meteor whose sides may be swapped.
 */
static void swap_sides(rectangle_t *meteor)
{
    int probabilities[2] = { 50, 50 };
    int index = compute_cumulative_distribution(probabilities, 2);

    if (index == 0) {
        int lengt_1 = get_rectangle_width(meteor);
        set_rectangle_width(meteor, get_rectangle_height(meteor));
        set_rectangle_height(meteor, lengt_1);
    }
}

/**
 * @brief Sets the shape of a meteor object based on material probabilities and current size.
 *
 * This function calculates the shape of a meteor object based on the provided materials' probabilities
 * and the current size of the meteor.
 *
 * @param meteor Pointer to the rectangle_t structure representing the meteor object.
 * @param materials Pointer to the materials_table_t structure containing material probabilities.
 */
static void set_meteor_shape(rectangle_t *meteor, materials_table_t *materials)
{
    const int SHAPE_COUNT = 2;
    int probabilities[SHAPE_COUNT];
    probabilities[0] = materials->materials[get_index_based_on_material_type((material_type_t)get_colour(meteor))].prob_rectangle_shape;
    probabilities[1] = materials->materials[get_index_based_on_material_type((material_type_t)get_colour(meteor))].prob_square_shape;

    int size_index = compute_cumulative_distribution(probabilities, SHAPE_COUNT);

    switch (size_index)
    {
    case RECTANGLE:
        set_rectangle_width(meteor, (get_rectangle_width(meteor) - 1 <= 0) ? 2 : (get_rectangle_width(meteor)));
        swap_sides(meteor);
        break;
    case SQUARE:
        break;
    default:
        break;
    }
}

/**
 * @brief Determines the meteor material type based on the given level's probabilities.
 *
 * This function calculates the meteor material type based on the probabilities provided in the given level.
 *
 * @param level The level_row_t structure containing probabilities for different materials.
 * @return The material_type_t of the meteor material chosen based on probabilities.
 */
static material_type_t count_meteor_material_from_level(level_row_t level)
{
    int probabilities[MATERIALS_COUNT] = { level.prob_stone, level.prob_copper, level.prob_iron, level.prob_gold };
    int material_index = compute_cumulative_distribution(probabilities, MATERIALS_COUNT);

    return get_material_type_based_on_index(material_index);
}

/**
 * @brief Sets the properties of a meteor object based on player level, level data, and material data.
 *
 * This function sets the properties of a meteor object, including its color, size, and shape, based on
 * the player's level, level data, and material data.
 *
 * @param meteor Pointer to the rectangle_t structure representing the meteor object.
 * @param player_level The player's level.
 * @param levels Pointer to the levels_table_t structure containing level data.
 * @param materials Pointer to the materials_table_t structure containing material data.
 */
static void set_meteor_properties(rectangle_t *meteor, int player_level, levels_table_t *levels, materials_table_t *materials, int width, int height)
{
    if (player_level > levels->count - 1) {
        player_level = levels->count - 1;
    }

    set_x_position(meteor, (rand() % (width - 10)) + 5);
    set_y_position(meteor, (rand() % (height - 10)) + 5);
    set_colour(meteor, (colour_t)count_meteor_material_from_level(levels->levels[player_level]));
    set_meteor_size(meteor, materials);
    set_meteor_shape(meteor, materials);
}

/**
 * @brief Converts a line of material data from a file into material row data.
 * 
 * @param table A pointer to the materials table to which the material row will be added.
 * @param line The line containing material data to be converted.
 * @param counter The counter indicating the position of the material data.
 * @return true if the conversion and addition are successful, false otherwise.
 */
static bool convert_line_into_material_data(materials_table_t *table, char *line, int counter)
{
    const char* DELIMITER = ";";

    char* token;
    char* line_copy = strdup(line);
    int prob_size_1_px_t; int prob_size_2_px_t; int prob_rectangle_shape; int prob_square_shape;

    token = strtok(line_copy, DELIMITER);
    if (token == NULL || !convert_string_2_int(token, &prob_size_1_px_t)) {
        free(line_copy);
        resolve_error(INVALID_DATA_IN_FILE);
        return false;
    }

    token = strtok(NULL, DELIMITER);
    if (token == NULL || !convert_string_2_int(token, &prob_size_2_px_t)) {
        free(line_copy);
        resolve_error(INVALID_DATA_IN_FILE);
        return false;
    }

    token = strtok(NULL, DELIMITER);
    if (token == NULL || !convert_string_2_int(token, &prob_rectangle_shape)) {
        free(line_copy);
        resolve_error(INVALID_DATA_IN_FILE);
        return false;
    }

    token = strtok(NULL, DELIMITER);
    if (token == NULL || !convert_string_2_int(token, &prob_square_shape)) {
        free(line_copy);
        resolve_error(INVALID_DATA_IN_FILE);
        return false;
    }

    // check that line is completely read
    if (strtok(NULL, DELIMITER) != NULL) {
        free(line_copy);
        resolve_error(INVALID_DATA_IN_FILE);
        return false;
    }

    free(line_copy);

    material_row_t material_row = create_material_row(counter, prob_size_1_px_t, prob_size_2_px_t, prob_rectangle_shape, prob_square_shape);
    if (add_material(table, material_row) == NULL) {
        return false;
    }
    return true;
}

/**
 * @brief Converts a line of level data from a file into level row data.
 * 
 * @param table A pointer to the levels table to which the level row will be added.
 * @param line The line containing level data to be converted.
 * @return true if the conversion and addition are successful, false otherwise.
 */
static bool convert_line_into_level_data(levels_table_t *table, char *line)
{
    const char* DELIMITER = ";";

    char* token;
    char* line_copy = strdup(line);
    int stone_request; int copper_request; int iron_request; int gold_request; int prob_stone; int prob_copper; int prob_iron; int prob_gold;   

    token = strtok(line_copy, DELIMITER);
    if (token == NULL || !convert_string_2_int(token, &stone_request)) {
        free(line_copy);
        resolve_error(INVALID_DATA_IN_FILE);
        return false;
    }

    token = strtok(NULL, DELIMITER);
    if (token == NULL || !convert_string_2_int(token, &copper_request)) {
        free(line_copy);
        resolve_error(INVALID_DATA_IN_FILE);
        return false;
    }

    token = strtok(NULL, DELIMITER);
    if (token == NULL || !convert_string_2_int(token, &iron_request)) {
        free(line_copy);
        resolve_error(INVALID_DATA_IN_FILE);
        return false;
    }

    token = strtok(NULL, DELIMITER);
    if (token == NULL || !convert_string_2_int(token, &gold_request)) {
        free(line_copy);
        resolve_error(INVALID_DATA_IN_FILE);
        return false;
    }

    token = strtok(NULL, DELIMITER);
    if (token == NULL || !convert_string_2_int(token, &prob_stone)) {
        free(line_copy);
        resolve_error(INVALID_DATA_IN_FILE);
        return false;
    }

    token = strtok(NULL, DELIMITER);
    if (token == NULL || !convert_string_2_int(token, &prob_copper)) {
        free(line_copy);
        resolve_error(INVALID_DATA_IN_FILE);
        return false;
    }

    token = strtok(NULL, DELIMITER);
    if (token == NULL || !convert_string_2_int(token, &prob_iron)) {
        free(line_copy);
        resolve_error(INVALID_DATA_IN_FILE);
        return false;
    }

    token = strtok(NULL, DELIMITER);
    if (token == NULL || !convert_string_2_int(token, &prob_gold)) {
        free(line_copy);
        resolve_error(INVALID_DATA_IN_FILE);
        return false;
    }

    // check that line is completely read
    if (strtok(NULL, DELIMITER) != NULL) {
        free(line_copy);
        resolve_error(INVALID_DATA_IN_FILE);
        return false;
    }

    free(line_copy);

    level_row_t level_row = create_level_row(stone_request, copper_request, iron_request, gold_request, prob_stone, prob_copper, prob_iron, prob_gold);
    if (add_level(table, level_row) == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        return false;
    }
    return true;
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
    if (get_y_position(ball) >= height - get_rectangle_height(ball) || get_y_position(ball) <= 0) {
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
    int ball_center = get_y_position(ball) + (get_rectangle_height(ball) / 2);
    int paddle_center = get_y_position(enemy) + (get_rectangle_height(enemy) / 2);
    int paddle_speed = (abs(ball_center - paddle_center) < 9) ? 1 : 2;

    if (ball_center < paddle_center) {
        set_y_position(enemy, get_y_position(enemy) - paddle_speed);
    } else if (ball_center > paddle_center) {
        set_y_position(enemy, get_y_position(enemy) + paddle_speed);
    }

    set_y_position(enemy, get_y_position(enemy) + rand() % 3 - 1);

    if (get_y_position(enemy) < 0) {
        set_y_position(enemy, 0);
    } else if (get_y_position(enemy) + get_rectangle_height(enemy) > height) {
        set_y_position(enemy, height - get_rectangle_height(enemy));
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
    int paddle_center = get_y_position(paddle) + (get_rectangle_height(paddle) / 2);
    int ball_center = get_y_position(ball) + (get_rectangle_height(ball) / 2);
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
    } else if (get_x_position(ball) + get_rectangle_width(ball) >= game->width) {
        is_in_bound = false;
    }

    if (!is_in_bound) {

        if (get_game_ticks(game) % 2 == 0) {
            game->player->hearts--;
            set_x_speed(ball, 2);
        } else {
            game->enemy->hearts--;
            set_x_speed(ball, -2);
        }

        reset_game_ticks(game);
        set_objects_to_initial_position(game);
        game->game_state = STOPPED;
    }

    return is_in_bound;
}

/**
 * @brief Sets the game objects (ball, player, enemy) to their initial positions and speeds.
 *
 * This helper function resets the positions and speeds of the ball, player, and enemy objects
 * to their initial values defined by constants.
 *
 * @param game A pointer to the game structure containing the objects to be reset.
 */
static void set_objects_to_initial_position(game_t *game)
{
    set_x_position(find_object(game, "ball"), BALL_INIT_X_COORD);
    set_y_position(find_object(game, "ball"), BALL_INIT_Y_COORD);
    set_y_speed(find_object(game, "ball"), 1);

    set_x_position(find_object(game, "player"), PLAYER_INIT_X_COORD);
    set_y_position(find_object(game, "player"), PLAYER_INIT_Y_COORD);
    set_y_speed(find_object(game, "player"), 0);

    set_x_position(find_object(game, "enemy"), ENEMY_INIT_X_COORD);
    set_y_position(find_object(game, "enemy"), ENEMY_INIT_Y_COORD);
    set_y_speed(find_object(game, "enemy"), 0);
}

/**
 * Determines the shape of a meteor based on its dimensions.
 *
 * This function determines the shape of a meteor (square or rectangle) based on
 * its dimensions.
 *
 * @param meteor A pointer to the meteor whose shape is to be determined.
 * @return The shape of the meteor, either SQUARE or RECTANGLE.
 */
static material_shape_t get_meteors_shape(rectangle_t *meteor)
{
    if (get_rectangle_width(meteor) == get_rectangle_height(meteor)) {
        return SQUARE;
    }
    return RECTANGLE;
}

/**
 * Updates the player's resources based on the properties of a meteor.
 *
 * This function updates the player's resource counts based on the properties of
 * the specified meteor. The meteor's size and material type determine the
 * resource increments.
 *
 * @param meteor A pointer to the meteor whose properties are considered.
 * @param player A pointer to the player whose resources are updated.
 */
static void update_player_resources(rectangle_t *meteor, player_t *player)
{
    int increment;

    if (get_rectangle_width(meteor) == SMALL_METEOR_SIZE) {
        increment = 1;
    } else {
        increment = 3;
    }

    if (get_meteors_shape(meteor) == SQUARE) {
        increment++;
    }

    switch (meteor->colour)
    {
    case STONE:
        player->stone += increment;
        break;
    case COPPER:
        player->copper += increment;
        break;
    case IRON:
        player->iron += increment;
        break;
    case GOLD:
        player->gold += increment;
        break;
    default:
        break;
    }
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
        update_player_resources(meteor, game->player);
    }

    set_meteor_properties(meteor, game->player->level, game->levels_table, game->materials_table, get_width(game), get_height(game));
    increment_game_ticks(game);
    reset_game_ticks(game);
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
static bool create_rectangle_and_add_it_to_scene(scene_t *scene, px_t position_x, px_t position_y, px_t width, px_t height, px_t x_speed, px_t y_speed, colour_t colour, const char *name)
{
    rectangle_t *object = create_rectangle(position_x, position_y, width, height, x_speed, y_speed, colour, name);
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

/**
 * Tests the meteor generator by generating and logging meteor properties.
 *
 * This function generates meteor properties based on the specified tested level
 * and logs them to a file named "meteors_generator_output.log". The test is
 * performed for a total of 1000 meteor objects.
 *
 * @param tested_level The level for which meteor properties are tested.
 */
static void test_meteors_generator(int tested_level)
{
    const char *TEST_FILE_NAME = "test_meteors_generator_output.log";
    materials_table_t *materials = NULL;
    levels_table_t *levels = NULL;
    if (!load_extern_game_data(GAME_DATA_PATH, &materials, &levels)) {
        return;
    }

    if (tested_level > levels->count - 1) {
        tested_level = levels->count - 1;
    }

    if (access(TEST_FILE_NAME, F_OK) == 0) {
        remove(TEST_FILE_NAME);
    }

    FILE *file = fopen(TEST_FILE_NAME, "a");
    fprintf(file, "Tested level: %d.\n\n", tested_level);

    for (int i = 0; i < 1000; ++i) {
        rectangle_t *testing_meteor = create_rectangle(0, 0, 0, 0, 0, 0, WHITE, "testing meteor");
        set_colour(testing_meteor, (colour_t)count_meteor_material_from_level(levels->levels[tested_level]));
        set_meteor_size(testing_meteor, materials);
        set_meteor_shape(testing_meteor, materials);
        fprintf(file, "%.4d: %d %d %s\n", i + 1, get_rectangle_width(testing_meteor), get_rectangle_height(testing_meteor), colour_2_string(get_colour(testing_meteor)));
        release_rectangle(testing_meteor);
    }

    fclose(file);

    release_materials_table(materials);
    release_levels_table(levels);
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
static int get_rectangle_width(rectangle_t *object)
{
    return object->width / 2;
}

/**
 * @brief Sets the value of the first side length of a rectangle object.
 * 
 * @param object A pointer to the rectangle object whose first side length is to be set.
 * @param size The value to set as the first side length.
 */
static void set_rectangle_width(rectangle_t *object, px_t size)
{
    object->width = size * 2;
}

/**
 * @brief Sets the value of the second side length of a rectangle object.
 *
 * @param object A pointer to the rectangle object whose second side length is to be set.
 * @param size The value to set as the second side length.
 */
static void set_rectangle_height(rectangle_t *object, px_t size)
{
    object->height = size;
}

/**
 * @brief Retrieves the length of the second side of the specified rectangle object.
 * 
 * @param object The rectangle object to query.
 * @return The length of the second side of the object.
 */
static int get_rectangle_height(rectangle_t *object)
{
    return object->height;
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
