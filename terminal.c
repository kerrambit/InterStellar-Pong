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

// ---------------------------------------- GLOBAL VARS ---------------------------------------------- //

static bool gl_is_terminal_enabled = false;
static unsigned long gl_curr_file_cursor = 0;
static unsigned long gl_curr_file_line_size = 0;

// ---------------------------------------- STATIC FUNCS --------------------------------------------- //

static char* get_last_line(int buffer_size);
static int get_maximal_terminal_buffer_size();

// ------------------------------------------ LIBRARY ------------------------------------------------ //

int enable_terminal()
{
    if (access(TERMINAL_FILE, F_OK) == 0) {
        if (remove(TERMINAL_FILE) != 0) {
            resolve_error(FAILURE_OF_REMOVING_FILE);
            return -1;
        }
    }

    FILE* file = fopen(TERMINAL_FILE, "w");
    if (file == NULL) {
        resolve_error(UNOPENABLE_FILE);
        return -1;
    }

    fclose(file);
    gl_is_terminal_enabled = true;
    return 0;
}

int remove_terminal_data()
{
    if (remove(TERMINAL_FILE) != 0) {
        resolve_error(FAILURE_OF_REMOVING_FILE);
        return -1;
    }

    gl_is_terminal_enabled = false;
}

static int parse_backspace(FILE *file)
{
    if (gl_curr_file_cursor > 0 && gl_curr_file_line_size > 0) {

        if (fseek(file, -1, SEEK_END) != 0) {
            resolve_error(GENERAL_IO_ERROR);
            return -1;
        }

        if (ftruncate(fileno(file), ftell(file)) != 0) {
            resolve_error(GENERAL_IO_ERROR);
            return -1;
        }

        gl_curr_file_cursor--; gl_curr_file_line_size--;
    }

    return 0;
}

static int parse_newline(char **command)
{
    int buffer_size = get_maximal_terminal_buffer_size(); // changed from 100 to 102

    char *line = get_last_line(buffer_size);

    *command = malloc(strlen(line) + 1);
    if (*command == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        free(line);
        return -1;
    }

    strcpy(*command, line);

    gl_curr_file_line_size = 0;
    free(line);
    return 0;
}

int process_command(char c, char **command)
{
    if (!gl_is_terminal_enabled) {
        resolve_error(INACTIVE_TERMINAL);
        return -1;
    }

    FILE* file = fopen(TERMINAL_FILE, "a");
    if (file == NULL) {
        resolve_error(UNOPENABLE_FILE);
        return -1;
    }

    if (KEYBOARD_PRESSED(c, BACKSPACE)) {
        int parse_backspace_return_code =  parse_backspace(file);
        fclose(file);
        return parse_backspace_return_code;
    }

    if (fputc(c, file) == EOF) {
        resolve_error(CORRUPTED_WRITE_TO_FILE);
        fclose(file);
        return -1;
    }

    gl_curr_file_cursor++; gl_curr_file_line_size++;

    if (KEYBOARD_PRESSED(c, NEWLINE)) {
        int parse_newline_return_code = parse_newline(command);
        fclose(file);
        return parse_newline_return_code;
    }

    fclose(file);
    return 0;
}

int render_terminal(px_t line_width, bool special_regime, const char *volunatary_mess, int mess_length)
{
    if (gl_is_terminal_enabled) {

        FILE* file = fopen(TERMINAL_FILE, "rb");
        if (file == NULL) {
            resolve_error(UNOPENABLE_FILE);
            return -1;
        }

        int buffer_size = get_maximal_terminal_buffer_size();
        char *line = get_last_line(buffer_size);

        put_horizontal_line(line_width - 1, '=');
        write_text("|| > ");

        if (special_regime) {
            if (volunatary_mess == NULL) {
                printf("\033[3m\033[93mUnknown command.\033[0m"); // default message
                put_text("||", 90, RIGHT);
            } else {
                write_text(volunatary_mess);
                put_text("||", line_width - mess_length - buffer_size - 7, RIGHT);
            }
            
        } else if (strlen(line) != 0) {
            write_text(line);
            put_text("||", line_width - 7 - buffer_size, RIGHT);
        } else {
            printf("\033[3m\033[90mEnter your commands.\033[0m");
            put_text("||", 86, RIGHT);
        }
        
        put_horizontal_line(line_width - 1, '=');

        fclose(file);
        free(line);
        return 0;
    }
}

static int get_maximal_terminal_buffer_size()
{
    int buffer_size = gl_curr_file_line_size;
    if (buffer_size > 102) {
        buffer_size = 102;
    }

    return buffer_size;
}

static char* get_last_line(int buffer_size)
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

    if (fseek(file, gl_curr_file_cursor - gl_curr_file_line_size, SEEK_SET) != 0) {
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
