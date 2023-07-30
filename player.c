#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "player.h"
#include "errors.h"
#include "utils.h"

player_t *create_player(char* name, int level, int copper, int iron, int gold)
{
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

    player->level = level; player->copper = copper; player->iron = iron; player->gold = gold;
    return player;
}

player_t *create_player_from_string(char* string)
{
    const char* DELIMITER = ";";

    char* token;
    char* line_copy = strdup(string);
    int level, copper, iron, gold;

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
    if (token == NULL || !convert_string_2_int(token, &level) || level < 0) {
        free(line_copy); free(name);
        resolve_error(INVALID_DATA_IN_FILE);
        return NULL;
    }

    // check copper
    token = strtok(NULL, DELIMITER);
    if (token == NULL || !convert_string_2_int(token, &copper) || copper < 0) {
        free(line_copy); free(name);
        resolve_error(INVALID_DATA_IN_FILE);
        return NULL;
    }

    // check iron
    token = strtok(NULL, DELIMITER);
    if (token == NULL || !convert_string_2_int(token, &iron) || iron < 0) {
        free(line_copy); free(name);
        resolve_error(INVALID_DATA_IN_FILE);
        return NULL;
    }

    // check gold
    token = strtok(NULL, DELIMITER);
    strip_newline(token);
    if (token == NULL || !convert_string_2_int(token, &gold) || gold < 0) {
        free(line_copy); free(name);
        resolve_error(INVALID_DATA_IN_FILE);
        return NULL;
    }

    // check that line is completely read
    if (strtok(NULL, DELIMITER) != NULL) {
        free(line_copy); free(name);
        resolve_error(INVALID_DATA_IN_FILE);
        return NULL;
    }
    
    player_t *player = create_player(name, level, copper, iron, gold);
    free(line_copy); free(name);

    if (player == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        return NULL;
    }

    return player;
}

void release_player(player_t *player)
{
    if (player != NULL) {
        free(player->name);
        free(player);
    }
}

players_array_t *create_players_array()
{
    players_array_t *array = malloc(sizeof(players_array_t));
    if (array == NULL) {
        return NULL;
    }

    array->count = 0;
    array->length = 4;
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
    if (players_array == NULL || player == NULL) {
        return NULL;
    }

    if (players_array->count >= players_array->length) {
        players_array->length *= 2;
        player_t **new_players_array = realloc(players_array->players, sizeof(player_t*) * players_array->length);
        if (new_players_array == NULL) {
            return NULL;
        }
        players_array->players = new_players_array;
    }

    players_array->players[players_array->count++] = player;
    return player;
}