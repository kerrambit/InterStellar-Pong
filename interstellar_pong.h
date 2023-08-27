/**
 * @file interstellar_pong.h
 * @author Marek Eibel
 * @brief Header file managing game mechanics, levels, and resource materials for InterStellar Pong.
 * @version 0.1
 * @date 2023-07-30
 * 
 * @copyright Copyright (c) 2023
 */

#ifndef INTERSTELLAR_PONG_H
#define INTERSTELLAR_PONG_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "draw.h"
#include "errors.h"
#include "levels.h"
#include "materials.h"
#include "page_loader.h"
#include "player.h"
#include "utils.h"

/**
 * @brief Represents the possible states of a game. It is used to indicate whether the game is currently running or stopped (or terminated). 
 */
typedef enum game_state_t {
    RUNNING, /** The game's main loop is active. */ 
    STOPPED,  /** The game has been stopped. In this state, the game could be halted for a certain time but must be rerun again. */
    TERMINATED /** The game has been terminated. In this state, the game has halted its main loop. */
} game_state_t;

/**
 * @struct game_t
 * @brief Represents the game of Interstellar Pong. Holds general data about the current instance of the game.
 */
typedef struct game_t {
    scene_t *scene;                       /** Pointer to the game scene. */
    player_t *player;                     /** Pointer to the player object. */
    player_t *enemy;                      /** Pointer to the enemy object. */
    px_t width;                           /** Width of the game screen. */
    px_t height;                          /** Height of the game screen. */
    game_state_t game_state;              /** Current state of the game. */
    int game_ticks;                       /** Represents the number of ball bounces between players. */
    materials_table_t *materials_table;   /** Pointer to the table containing data about materials. */
    levels_table_t *levels_table;         /** Pointer to the table containing data about game levels. */
} game_t;

/**
 * @brief Initializes a new game instance with the specified player, height and width.
 * 
 * @param player The player instance to use, or NULL to create a default player (game bound to no player account).
 * @param height The height of the game area.
 * @param width The width of the game area.
 * @return A pointer to the newly created game instance, or NULL on failure.
 */
game_t *init_game(player_t *player, px_t height, px_t width);

/**
 * @brief Starts the game.
 * 
 * @param game The game instance to start.
 */
void start_game(game_t *game);

/**
 * @brief Retrieves the current game state.
 * 
 * @param game The game instance.
 * @return The current game state.
 */
game_state_t get_game_state(game_t *game);

/**
 * @brief Ends the game.
 * 
 * @param game The game instance to end.
 */
void end_game(game_t *game);

/**
 * @brief Stops the game.
 * 
 * @param game The game instance to stop.
 */
void stop_game(game_t *game);

/**
 * @brief Releases memory occupied by the game instance and associated resources.
 * 
 * @param game The game instance to release.
 */
void release_game(game_t *game);

/**
 * @brief Initializes a new scene for the provided game instance. The created objects in the scene are determined - they are
 *        objects in the InterStellar-Pong game (player, enemy, ball and meteors). 
 * 
 * @param game The game instance for which to create the scene.
 * @return A pointer to the newly created scene, or NULL on failure.
 */
scene_t *init_scene(game_t *game);

/**
 * @brief Updates the game scene based on the current game state and updates pixels in <pixel_buffer>.
 * 
 * @param game The game instance to update.
 * @param pixel_buffer The pixel buffer to update with object pixels.
 * @return The updated game scene.
 */
scene_t *update_scene(game_t *game, pixel_buffer_t *pixel_buffer);

/**
 * @brief Handles the keyboard event and updates the game state accordingly.
 * 
 * @param game The game instance to update.
 * @param c The character representing the pressed key.
 */
void handle_event(game_t *game, char c);

#endif
