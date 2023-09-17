/**
 * @file player.h
 * @author Marek Eibel
 * @brief Declarations and structures related to player data management.
 * 
 * This header file provides declarations for structures and functions related
 * to managing player data in the InterStellar Pong game. It defines the player_t
 * structure to hold player attributes, as well as the players_array_t structure
 * to manage an array of player instances. Additionally, it declares functions
 * for creating, releasing, and manipulating player instances and player arrays.
 * 
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
#define TOO_LONG_NAME ";;;;"

/**
 * @struct player_t
 * @brief Data structure representing a player in Interstellar Pong.
 *
 * The `player_t` structure defines attributes that represent a player's information. It is used to store
 * information about individual players in the game.
 */
typedef struct player_t {
    char *name;    /** The player's name. */
    int level;     /** The player's level in the game. */
    int stone;     /** The quantity of stone resources collected by the player. */
    int copper;    /** The quantity of copper resources collected by the player. */
    int iron;      /** The quantity of iron resources collected by the player. */
    int gold;      /** The quantity of gold resources collected by the player. */
    int hearts;    /** The number of remaining hearts (health) for the player. */
} player_t;

/**
 * @struct players_array_t
 * @brief Data structure representing an array of players in Interstellar Pong.
 *
 * The `players_array_t` structure is used to manage an array of player instances.
 * It contains an array of player pointers, along with information about the number
 * of players in the array (`count`) and the allocated length of the array (`length`).
 */
typedef struct players_array_t {
    player_t **players;   /** An array of pointers to player instances. */
    int count;            /** The current number of players in the array. */
    int length;           /** The allocated length of the player array. */
} players_array_t;

/**
 * @brief Creates a new player.
 *
 * This function dynamically allocates memory for a new player instance and initializes
 * its attributes with the provided values.
 *
 * @param name The player's name.
 * @param level The player's level.
 * @param stone The player's stone resource count.
 * @param copper The player's copper resource count.
 * @param iron The player's iron resource count.
 * @param gold The player's gold resource count.
 * @return A pointer to the newly created player instance, or NULL on failure.
 */
player_t *create_player(char* name, int level, int stone, int copper, int iron, int gold);

/**
 * @brief Releasse memory occupied by a player instance.
 *
 * This function releases the memory occupied by a player instance.
 *
 * @param player A pointer to the player instance to be released.
 */
void release_player(player_t *player);

/**
 * @brief Creates a player instance from a string.
 *
 * This function parses a string representing player data and creates a player instance
 * based on the parsed information.
 *
 * @param string The input string containing player data.
 * @param file_path Path to the file.
 * @return A pointer to the newly created player instance, or NULL on failure.
 */
player_t *create_player_from_string(char* string, const char *file_path);

/**
 * @brief Creates an array to hold player instances.
 *
 * This function creates an array structure to hold player instances.
 *
 * @return A pointer to the newly created players array, or NULL on failure.
 */
players_array_t *create_players_array();

/**
 * @brief Releases memory occupied by a players array.
 *
 * This function releases the memory occupied by a players array.
 *
 * @param players_array A pointer to the players array to be released.
 */
void release_players_array(players_array_t *players_array);

/**
 * @brief Adds a player to the players array.
 *
 * This function adds a player instance to a players array. If the players array is NULL or memory allocation fails,
 * NULL is returned. Otherwise, the added player instance is returned.
 *
 * @param players_array A pointer to the players array to add the player to.
 * @param player A pointer to the player instance to be added.
 * @return A pointer to the added player instance, or NULL on failure.
 */
player_t *add_to_players_array(players_array_t *players_array, player_t *player);

#endif
