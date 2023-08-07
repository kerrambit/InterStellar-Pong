/**
 * @file player.h
 * @author Marek Eibel
 * @brief (...)
 * @version 0.1
 * @date 2023-07-30
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef PLAYER_H
#define PLAYER_H

#define INVALID_NAME ";"
#define NOT_UNIQUE_NAME ";;"
#define NO_NAME_ENTERED ";;;"

typedef struct player_t {
    char *name;
    int level;
    int stone;
    int copper;
    int iron;
    int gold;
    int hearts;
} player_t;

typedef struct players_array_t {
    player_t **players;
    int count;
    int length;
} players_array_t;

player_t *create_player(char* name, int level, int stone, int copper, int iron, int gold);
player_t *create_player_from_string(char* string);
void release_player(player_t *player);
players_array_t *create_players_array();
void release_players_array(players_array_t *players_array);
player_t *add_to_players_array(players_array_t *players_array, player_t *player);

#endif