#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../termify/errors.h"
#include "player.h"
#include "../termify/utils.h"

// ---------------------------------------- MACROS --------------------------------------------- //

#define CLEAN_AND_RETURN_WITH_FAILURE free(line_copy); free(name); return NULL

// ---------------------------------- STATIC DECLARATIONS--------------------------------------- //

static bool save_number(char *token, int *data_holder);

// ----------------------------------------- PROGRAM-------------------------------------------- //

player_t *create_player(char* name, int level, int stone, int copper, int iron, int gold)
{
    const int NUMBER_OF_PLAYER_HEARTS = 3;

    player_t *player = malloc(sizeof(player_t));
    if (player == NULL) {
        return NULL;
    }

    player->name = malloc(strlen(name) + 1);
    if (player->name == NULL) {
        free(player);
        return NULL;
    }

    strcpy(player->name, name);

    player->level = level; player->stone = stone; player->copper = copper; player->iron = iron; player->gold = gold; player->hearts = NUMBER_OF_PLAYER_HEARTS;
    return player;
}

void release_player(player_t *player)
{
    if (player != NULL) {
        if (player->name != NULL) {
            free(player->name);
        }
        free(player);
    }
}

player_t *create_player_from_string(char* string)
{
    const char* DELIMITER = ";";

    char* token;
    char* line_copy = strdup(string);
    int level, stone, copper, iron, gold;

    // check name is present
    token = strtok(line_copy, DELIMITER);
    if (token == NULL) {
        free(line_copy);
        resolve_error(INVALID_DATA_IN_FILE);
        return NULL;
    }
    char *name = strdup(token);

    // check level
    token = strtok(NULL, DELIMITER);
    if (!save_number(token, &level)) {
        CLEAN_AND_RETURN_WITH_FAILURE;
    }

    // check stone
    token = strtok(NULL, DELIMITER);
    if (!save_number(token, &stone)) {
        CLEAN_AND_RETURN_WITH_FAILURE;
    }

    // check copper
    token = strtok(NULL, DELIMITER);
    if (!save_number(token, &copper)) {
        CLEAN_AND_RETURN_WITH_FAILURE;
    }

    // check iron
    token = strtok(NULL, DELIMITER);
    if (!save_number(token, &iron)) {
        CLEAN_AND_RETURN_WITH_FAILURE;
    }

    // check gold
    token = strtok(NULL, DELIMITER);
    strip_newline(token);
    if (!save_number(token, &gold)) {
        CLEAN_AND_RETURN_WITH_FAILURE;
    }

    // check that line is completely read
    if (strtok(NULL, DELIMITER) != NULL) {
        resolve_error(INVALID_DATA_IN_FILE);
        CLEAN_AND_RETURN_WITH_FAILURE;
    }
    
    player_t *player = create_player(name, level, stone, copper, iron, gold);
    free(line_copy); free(name);

    if (player == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        return NULL;
    }

    return player;
}

players_array_t *create_players_array()
{
    const int BEGIN_ARRAY_SIZE = 4;

    players_array_t *array = malloc(sizeof(players_array_t));
    if (array == NULL) {
        return NULL;
    }

    array->count = 0;
    array->length = BEGIN_ARRAY_SIZE;
    array->players = malloc(sizeof(player_t*) * array->length);

    if (array->players == NULL) {
        free(array);
        return NULL;
    }

    return array;
}

void release_players_array(players_array_t *players_array)
{
    if (players_array == NULL) {
        return;
    }

    for (int i = 0; i < players_array->count; ++i) {
        release_player(players_array->players[i]);
    }

    free(players_array->players);
    free(players_array);
}

player_t *add_to_players_array(players_array_t *players_array, player_t *player)
{
    const int GROWTH_FACTOR = 2;

    if (players_array == NULL || player == NULL) {
        return NULL;
    }

    if (players_array->count >= players_array->length) {
        players_array->length *= GROWTH_FACTOR;
        player_t **new_players_array = realloc(players_array->players, sizeof(player_t*) * players_array->length);
        if (new_players_array == NULL) {
            return NULL;
        }
        players_array->players = new_players_array;
    }

    players_array->players[players_array->count++] = player;
    return player;
}

/**
 * @brief Saves a number from a string token to a data holder.
 *
 * This function takes a string token as input and attempts to convert it
 * to an integer. If the conversion is successful and the resulting integer is non-negative,
 * it is stored in the provided data holder.
 *
 * @param token The string token containing the number to be converted and saved.
 * @param data_holder A pointer to an integer to store the converted value.
 * @return Returns true if the conversion and saving are successful, or false otherwise.
 */
static bool save_number(char *token, int *data_holder)
{
    if (token == NULL || !convert_string_2_int(token, data_holder) || *data_holder < 0) {
        resolve_error(INVALID_DATA_IN_FILE);
        return false;
    }

    return true;
}
