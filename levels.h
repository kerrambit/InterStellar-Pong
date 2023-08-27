/**
 * @file levels.h
 * @author Marek Eibel
 * @brief Defines structures and functions for managing game levels.
 * @version 0.1
 * @date 2023-08-27
 * 
 * This file contains the declarations of structures and functions related to game levels.
 * It provides the necessary structures to store and manage level data, including resource requests and probabilities.
 * The functions enable creating, adding, and releasing levels within a levels table.
 * 
 * @copyright Copyright (c) 2023
 */

#ifndef LEVELS_H
#define LEVELS_H

/**
 * @brief Represents a row of level data containing resource requests and probabilities.
 */
typedef struct level_row_t {
    int stone_request;      /** The stone resource request for the level. */
    int copper_request;     /** The copper resource request for the level. */
    int iron_request;       /** The iron resource request for the level. */
    int gold_request;       /** The gold resource request for the level. */
    int prob_stone;         /** The probability of encountering stone resources in the level. */
    int prob_copper;        /** The probability of encountering copper resources in the level. */
    int prob_iron;          /** The probability of encountering iron resources in the level. */
    int prob_gold;          /** The probability of encountering gold resources in the level. */
} level_row_t;

/**
 * @brief Represents a table containing an array of level data.
 */
typedef struct levels_table_t {
    level_row_t *levels;    /** Pointer to an array of levels. */
    int count;              /** Number of levels cells in the table. */
    int length;             /** Current allocated length of the levels array. */
} levels_table_t;

/**
 * @brief Creates a new level row with the specified parameters.
 * 
 * @param stone_request The stone resource request for the level.
 * @param copper_request The copper resource request for the level.
 * @param iron_request The iron resource request for the level.
 * @param gold_request The gold resource request for the level.
 * @param prob_stone The probability of encountering stone resources in the level.
 * @param prob_copper The probability of encountering copper resources in the level.
 * @param prob_iron The probability of encountering iron resources in the level.
 * @param prob_gold The probability of encountering gold resources in the level.
 * @return level_row_t The created level row.
 */
level_row_t create_level_row(int stone_request, int copper_request, int iron_request, int gold_request, int prob_stone, int prob_copper, int prob_iron, int prob_gold);

/**
 * @brief Creates a new empty levels table.
 * 
 * @return levels_table_t* A pointer to the created levels table.
 */
levels_table_t *create_levels_table();

/**
 * @brief Adds a new level to the levels table.
 * 
 * @param table A pointer to the levels table.
 * @param level The level row to be added.
 * @return levels_table_t* A pointer to the updated levels table.
 */
levels_table_t *add_level(levels_table_t *table, level_row_t level);

/**
 * @brief Releases the memory allocated for the levels table and its contents.
 * 
 * @param table A pointer to the levels table to be released.
 */
void release_levels_table(levels_table_t *table);

/**
 * @brief Converts a level row data structure to a string representation.
 * 
 * @param material_row The level row to be converted.
 * @return A dynamically allocated string containing the material row information,
 *         or NULL if memory allocation fails.
 * @warning Returned string must be freed!
 */
const char *convert_level_row_2_string(level_row_t level_row);

#endif
