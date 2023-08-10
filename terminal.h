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
    TERMINAL_NORMAL_TEXT,
    TERMINAL_LOG,
    TERMINAL_APPROVAL,
    TERMINAL_WARNING,
    TERMINAL_ERROR,
    TERMINAL_N_A
} terminal_output_mode_t;

typedef struct terminal_data_t {
    bool is_terminal_enabled;
    unsigned long curr_file_cursor;
    unsigned long curr_file_line_size;
    char *terminal_default_mess;
    terminal_output_mode_t terminal_default_mess_mode;
    char *terminal_special_flag_default_mess;
    terminal_output_mode_t terminal_spacial_flag_default_mess_mode;
} terminal_data_t;

terminal_data_t *enable_terminal(char *default_mess, terminal_output_mode_t default_mess_mode, char *special_flag_default_mess, terminal_output_mode_t special_flag_default_mess_mode);
int render_terminal(terminal_data_t *terminal_data, px_t line_width, bool special_flag, char *volunatary_mess, terminal_output_mode_t mode);
int process_command(terminal_data_t *terminal_data, char c, char **command); 
int close_terminal(terminal_data_t *terminal_data);

#endif