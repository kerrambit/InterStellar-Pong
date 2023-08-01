/**
 * @file terminal.h
 * @author Marek Eibel
 * @brief 
 * @version 0.1
 * @date 2023-07-21
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef TERMINAL_H
#define TERMINAL_H

#include "draw.h"
#include <stdbool.h>

int enable_terminal();
int render_terminal(px_t line_width, bool special_regime, const char *volunatary_mess, int mess_length);
int process_command(char c, char **command);
int remove_terminal_data();

#endif