/**
 * @file terminal.h
 * @author Marek Eibel
 * @brief (...)
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

typedef enum terminal_output_mode_t {
    NORMAL_TEXT
    TERMINAL_LOG,
    APPROVAL,
    WARNING,
    ERROR,
    N/A
} terminal_output_mode_t;

int enable_terminal(); // int enable_terminal(const char *default_mess, terminal_output_mode_t default_mess_mode, const char *special_regime_default_mess, terminal_output_mode_t special_regime_default_mess_mode);
int render_terminal(px_t line_width, bool special_regime, const char *volunatary_mess, int mess_length); // int render_terminal(px_t line_width, bool special_flag, const char *volunatary_mess, terminal_output_mode_t mode);
int process_command(char c, char **command);
int remove_terminal_data();

#endif