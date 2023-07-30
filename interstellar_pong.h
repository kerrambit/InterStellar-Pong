/**
 * @file interstellar_pong.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-07-30
 * 
 * @copyright Copyright (c) 2023
 * 
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

typedef enum game_state_t {
    RUNNING,
    STOPPED
} game_state_t;

typedef struct game_t {
    scene_t *scene;
    player_t *player;
    px_t width;
    px_t height;
    game_state_t game_state;
} game_t;

game_t *init_game(player_t *player, px_t height, px_t width);
scene_t *init_scene(game_t *game);
void start_game(game_t *game);
scene_t *update_scene(game_t *game, pixel_buffer_t *pixel_buffer);
void handle_event(game_t *game, char c);
game_state_t get_game_state(game_t *game);
void end_game(game_t *game);
void release_game(game_t *game);

#endif