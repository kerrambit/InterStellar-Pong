/**
 * @file interstellar_pong.h
 * @author Marek Eibel
 * @brief 
 * @version 0.1
 * @date 2023-07-30
 * 
 * @copyright Copyright (c) 2023
 */

#ifndef INTERSTELLAR_PONG_H
#define INTERSTELLAR_PONG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "draw.h"
#include "errors.h"
#include "utils.h"
#include "page_loader.h"
#include "player.h"

/**
 * @brief Represents the possible states of a game. It is used to indicate whether the game is currently running or stopped (or terminated). 
 */
typedef enum game_state_t {
    RUNNING, /** The game's main loop is active. */ 
    STOPPED  /** The game has been stopped or terminated. In this state, the game has halted its main loop. */
} game_state_t;

/**
 * @struct game_t
 * @brief Represents the game of Interstellar Pong. Holds general data about the current instance of the game.
 */
typedef struct game_t {
    scene_t *scene;          /** Pointer to the game scene. */
    player_t *player;        /** Pointer to the player object. */
    px_t width;              /** Width of the game screen. */
    px_t height;             /** Height of the game screen. */
    game_state_t game_state; /** Current state of the game. */
} game_t;

game_t *init_game(player_t *player, px_t height, px_t width);
void start_game(game_t *game);
game_state_t get_game_state(game_t *game);
void end_game(game_t *game);
void release_game(game_t *game);

scene_t *init_scene(game_t *game);
scene_t *update_scene(game_t *game, pixel_buffer_t *pixel_buffer);

void handle_event(game_t *game, char c);

#endif