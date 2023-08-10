#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "terminal.h"
#include "errors.h"
#include "utils.h"

// ------------------------------------------ MACROS ------------------------------------------------- //

#define TERMINAL_FILE "user_input.data"
#define BACKSPACE 127
#define NEWLINE '\n'

// ---------------------------------------- STATIC FUNCS --------------------------------------------- //

static char* get_last_line(terminal_data_t *terminal_data, int buffer_size);
static int get_maximal_terminal_buffer_size(terminal_data_t *terminal_data);
static int parse_backspace(terminal_data_t *terminal_data, FILE *file);
static int parse_newline(terminal_data_t *terminal_data, char **command);

// ------------------------------------------ LIBRARY ------------------------------------------------ //

terminal_data_t *enable_terminal(char *default_mess, terminal_output_mode_t default_mess_mode, char *special_flag_default_mess, terminal_output_mode_t special_flag_default_mess_mode)
{
    if (access(TERMINAL_FILE, F_OK) == 0) {
        if (remove(TERMINAL_FILE) != 0) {
            resolve_error(FAILURE_OF_REMOVING_FILE);
            return NULL;
        }
    }

    FILE* file = fopen(TERMINAL_FILE, "w");
    if (file == NULL) {
        resolve_error(UNOPENABLE_FILE);
        return NULL;
    }

    fclose(file);

    terminal_data_t *data = malloc(sizeof(terminal_data_t));
    if (data == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        return NULL;
    }

    data->is_terminal_enabled = true;
    data->terminal_default_mess = strdup(default_mess);
    if (data->terminal_default_mess == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        free(data);
        return NULL;
    }
    data->terminal_default_mess_mode = default_mess_mode;
    data->terminal_special_flag_default_mess = strdup(special_flag_default_mess);
    if (data->terminal_special_flag_default_mess == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        free(data->terminal_default_mess);
        free(data);
        return NULL;
    }
    data->terminal_spacial_flag_default_mess_mode = special_flag_default_mess_mode;
    data->curr_file_cursor = 0; data->curr_file_line_size = 0;

    return data;
}

int close_terminal(terminal_data_t *terminal_data)
{
    if (remove(TERMINAL_FILE) != 0) {
        resolve_error(FAILURE_OF_REMOVING_FILE);
        return -1;
    }

    if (terminal_data != NULL) {
        free(terminal_data->terminal_default_mess);
        free(terminal_data->terminal_special_flag_default_mess);
        free(terminal_data);
    }

    return 0;
}

static int parse_backspace(terminal_data_t *terminal_data, FILE *file)
{
    if (terminal_data->curr_file_cursor > 0 && terminal_data->curr_file_line_size > 0) {

        if (fseek(file, -1, SEEK_END) != 0) {
            resolve_error(GENERAL_IO_ERROR);
            return -1;
        }

        if (ftruncate(fileno(file), ftell(file)) != 0) {
            resolve_error(GENERAL_IO_ERROR);
            return -1;
        }

        terminal_data->curr_file_cursor--; terminal_data->curr_file_line_size--;
    }

    return 0;
}

static int parse_newline(terminal_data_t *terminal_data, char **command)
{
    int buffer_size = get_maximal_terminal_buffer_size(terminal_data); // changed from 100 to 102

    char *line = get_last_line(terminal_data, buffer_size);

    *command = malloc(strlen(line) + 1);
    if (*command == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        free(line);
        return -1;
    }

    strcpy(*command, line);

    terminal_data->curr_file_line_size = 0;
    free(line);
    return 0;
}

int process_command(terminal_data_t *terminal_data, char c, char **command)
{
    if (!terminal_data->is_terminal_enabled) {
        resolve_error(INACTIVE_TERMINAL);
        return -1;
    }

    FILE* file = fopen(TERMINAL_FILE, "a");
    if (file == NULL) {
        resolve_error(UNOPENABLE_FILE);
        return -1;
    }

    if (KEYBOARD_PRESSED(c, BACKSPACE)) {
        int parse_backspace_return_code =  parse_backspace(terminal_data, file);
        fclose(file);
        return parse_backspace_return_code;
    }

    if (fputc(c, file) == EOF) {
        resolve_error(CORRUPTED_WRITE_TO_FILE);
        fclose(file);
        return -1;
    }

    terminal_data->curr_file_cursor++; terminal_data->curr_file_line_size++;

    if (KEYBOARD_PRESSED(c, NEWLINE)) {
        int parse_newline_return_code = parse_newline(terminal_data, command);
        fclose(file);
        return parse_newline_return_code;
    }

    fclose(file);
    return 0;
}

int render_terminal(terminal_data_t *terminal_data, px_t line_width, bool special_flag, char *volunatary_mess, terminal_output_mode_t mode)
{
    if (terminal_data->is_terminal_enabled) { // TODO check terminal for negative result and omit the if branch then

        FILE* file = fopen(TERMINAL_FILE, "rb");
        if (file == NULL) {
            resolve_error(UNOPENABLE_FILE);
            return -1;
        }

        char *string_to_print = NULL;
        char *line = NULL;
        terminal_output_mode_t mode_to_print_with;
        if (special_flag) {
            if (volunatary_mess == NULL) {
                string_to_print = terminal_data->terminal_special_flag_default_mess;
                mode_to_print_with = terminal_data->terminal_spacial_flag_default_mess_mode;
            } else {
                string_to_print = volunatary_mess;
                mode_to_print_with = mode;
            }
        } else {
            int buffer_size = get_maximal_terminal_buffer_size(terminal_data);
            line = get_last_line(terminal_data, buffer_size);
            if (strlen(line) != 0) {
                string_to_print = line;
                mode_to_print_with = TERMINAL_NORMAL_TEXT;
            } else {
                string_to_print = terminal_data->terminal_default_mess;
                mode_to_print_with = terminal_data->terminal_default_mess_mode;
            }
        } 
        
        put_horizontal_line(line_width - 1, '=');
        write_text("|| > ");

        switch (mode_to_print_with)
        {
        case TERMINAL_NORMAL_TEXT:
            printf("%s", string_to_print); break;
        case TERMINAL_LOG:
            printf("\033[3m\033[90m%s\033[0m", string_to_print); break;
        case TERMINAL_APPROVAL:
            printf("\033[0;32m%s\033[0m", string_to_print); break;
        case TERMINAL_WARNING:
            printf("\033[0;33m%s\033[0m", string_to_print); break;
        case TERMINAL_ERROR:
            printf("\033[0;31m%s\033[0m", string_to_print); break;
        case TERMINAL_N_A:
            printf("%s", string_to_print); break;
        default:
            break;
        }

        put_text("||", line_width - strlen(string_to_print) - 6, RIGHT);
        put_horizontal_line(line_width - 1, '=');

        free(line);
        fclose(file);
        return 0;
    }
}

static int get_maximal_terminal_buffer_size(terminal_data_t *terminal_data)
{
    int buffer_size = terminal_data->curr_file_line_size;
    if (buffer_size > 102) {
        buffer_size = 102;
    }

    return buffer_size;
}

static char* get_last_line(terminal_data_t *terminal_data, int buffer_size)
{
    char *line = calloc(buffer_size + 1, sizeof(char));
    if (line == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        return NULL;
    }

    FILE* file = fopen(TERMINAL_FILE, "rb");
    if (file == NULL) {
        resolve_error(UNOPENABLE_FILE);
        free(line);
        return NULL;
    }

    if (fseek(file, terminal_data->curr_file_cursor - terminal_data->curr_file_line_size, SEEK_SET) != 0) {
        resolve_error(GENERAL_IO_ERROR);
        free(line);
        fclose(file);
        return NULL;
    }

    fread(line, sizeof(char), buffer_size, file);
    line[buffer_size] = '\0';

    fclose(file);
    return line;
}
