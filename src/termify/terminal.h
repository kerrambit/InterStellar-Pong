/**
 * @file terminal.h
 * @author Marek Eibel
 * @brief Defines functions and data structures for managing a virtual terminal in a program.
 * 
 * This module provides functionality to enable, process, and render messages and commands in a virtual terminal.
 * It allows for handling various output modes, such as log, approval, warning, and error messages.
 * 
 * @version 0.1
 * @date 2023-07-21
 * 
 * @copyright Copyright (c) 2023
 */

#ifndef TERMINAL_H
#define TERMINAL_H

#include "draw.h"
#include <stdbool.h>

/**
 * @enum terminal_output_mode_t
 * @brief Enumeration representing different output modes for terminal messages.
 *
 * The `terminal_output_mode_t` enumeration defines the various output modes for terminal messages.
 */
typedef enum terminal_output_mode_t {
    TERMINAL_NORMAL_TEXT,       /** Normal text output mode. */
    TERMINAL_LOG,               /** Log output mode. Definied as italica text in grey color. */
    TERMINAL_APPROVAL,          /** Approval output mode. Definied as light green text. */
    TERMINAL_WARNING,           /** Warning output mode. Definied as light yellow text. */
    TERMINAL_ERROR,             /** Error output mode. Definied as light red text. */
    TERMINAL_N_A                /** Not applicable output mode. Use to indicate you do not want to use any mode. */
} terminal_output_mode_t;

/**
 * @brief A structure to store cursor information for a terminal.
 * With every saved command, this structure holds data about its position in the file (that is <cursor_offset>) and its length.
 */
typedef struct terminal_cursor_duo_t {
    int cursor_offset;  /** The cursor offset. */
    int line_length;    /** The length of the line. */
} terminal_cursor_duo_t;

/**
 * @brief A structure to store terminal file cursor storage. All commands positions in the file are stored here.
 */
typedef struct terminal_file_cursor_storage_t {
    terminal_cursor_duo_t *storage; /** An array of cursor duos. */
    int count;                      /** The current number of cursor duos in the array. */
    int length;                     /** The allocated length of the cursor duo array. */
} terminal_file_cursor_storage_t;

/**
 * @struct terminal_data_t
 * @brief Data structure to hold terminal-related information.
 *
 * The `terminal_data_t` structure stores data related to terminal functionality, including whether
 * the terminal is enabled, the current cursor position in a file, default terminal messages, and more.
 */
typedef struct terminal_data_t {
    bool is_terminal_enabled;                                       /** Indicates whether the terminal is enabled. */
    unsigned long curr_file_cursor;                                 /** Current cursor position in a file. */
    unsigned long curr_file_line_size;                              /** Size of the current file's line. */
    char *terminal_default_mess;                                    /** Default terminal message. Shown when the current line in the file is empty. */
    terminal_output_mode_t terminal_default_mess_mode;              /** Output mode for the default terminal message. */
    char *terminal_special_flag_default_mess;                       /** Default terminal message with special flag. Shown if the variable <special_flag> in render_terminal() function is set to true. */
    terminal_output_mode_t terminal_spacial_flag_default_mess_mode; /** Output mode for the default terminal message with special flag. */
    terminal_file_cursor_storage_t *cursors_storage;                /** A structure to store terminal file cursor storage. */
    int curr_line;                                                  /** Current line in the file. */
    int lines_count_in_file;                                        /** Number of lines in the terminal file. */
} terminal_data_t;

/**
 * @brief Enables the terminal and initialize its data structure.
 *
 * The `enable_terminal` function initializes the terminal by creating an empty terminal file and
 * setting up the terminal data structure with default messages and modes.
 *
 * @param default_mess Default terminal message.
 * @param default_mess_mode Output mode for the default terminal message.
 * @param special_flag_default_mess Default terminal message with special flag.
 * @param special_flag_default_mess_mode Output mode for the default terminal message with special flag.
 * @return A pointer to the initialized terminal data structure, or NULL on failure.
 */
terminal_data_t *enable_terminal(char *default_mess, terminal_output_mode_t default_mess_mode, char *special_flag_default_mess, terminal_output_mode_t special_flag_default_mess_mode);

/**
 * @brief Closes the terminal and free its resources.
 *
 * The `close_terminal` function cleans up the resources used by the terminal data structure and
 * removes the terminal file.
 *
 * @param terminal_data A pointer to the terminal data structure.
 * @return 0 on success, -1 on failure.
 */
int close_terminal(terminal_data_t *terminal_data);

/**
 * @brief Renders the terminal output to the console.
 *
 * The `render_terminal` function renders the terminal output to the console, considering special flags and
 * custom messages, and applying color codes based on the specified output mode.
 *
 * @param terminal_data A pointer to the terminal data structure.
 * @param line_width The width of the window.
 * @param special_flag Indicates whether a special flag is enabled.
 * @param volunatary_mess A custom message to be printed, or NULL to use default messages.
 * @param mode The output mode for rendering, or TERMINAL_N_A to use default mode.
 * @return 0 on success, -1 on failure.
 */
int render_terminal(terminal_data_t *terminal_data, px_t line_width, bool special_flag, char *volunatary_mess, terminal_output_mode_t mode);

/**
 * @brief Process a keyboard input character for the terminal.
 *
 * The `process_command` function processes a keyboard input character for the terminal, including handling
 * backspace and newline characters. It writes the character to the terminal file and manages cursor position.
 *
 * @param terminal_data A pointer to the terminal data structure.
 * @param c The input character to be processed.
 * @param command A pointer to the command string that will be set if a newline character is detected.
 * @return 0 on success, -1 on failure.
 */
int process_command(terminal_data_t *terminal_data, char c, char **command); 

#endif