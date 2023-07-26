#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "terminal.h"
#include "errors.h"

// ------------------------------------------ MACROS ------------------------------------------------- //

#define TERMINAL_FILE "user_input.data"
#define BACKSPACE 127
#define NEWLINE '\n'

// ---------------------------------------- GLOBAL VARS ---------------------------------------------- //

static bool IS_TERMINAL_TURNED_ON = false;
static bool IS_TERMINAL_ENABLED = false;
static unsigned long curr_cursor = 0;
static unsigned long curr_line_size = 0;

// ---------------------------------------- STATIC FUNCS --------------------------------------------- //

static char* get_last_line(int buffer_size);

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
        return -1;
    }

    fclose(file);
    IS_TERMINAL_ENABLED = true;
    return 0;
}

int remove_terminal_data()
{
    if (remove(TERMINAL_FILE) != 0) {
        return -1;
    }

    IS_TERMINAL_TURNED_ON = false;
    IS_TERMINAL_ENABLED = false;
}

int process_command(char c, char **command)
{
    if (!IS_TERMINAL_TURNED_ON) {
        resolve_error(INACTIVE_TERMINAL);
        return -1;
    }

    FILE* file = fopen(TERMINAL_FILE, "a");
    if (file == NULL) {
        resolve_error(UNOPENABLE_FILE);
        return -1;
    }

    if (c == BACKSPACE) {

        if (curr_cursor > 0 && curr_line_size > 0) {

            if (fseek(file, -1, SEEK_END) != 0) {
                resolve_error(GENERAL_IO_ERROR);
                fclose(file);
                return -1;
            }

            if (ftruncate(fileno(file), ftell(file)) != 0) {
                resolve_error(GENERAL_IO_ERROR);
                fclose(file);
                return -1;
            }
            curr_cursor--; curr_line_size--;
        }

        fclose(file);
        return 0;
    }

    if (fputc(c, file) == EOF) {
        resolve_error(CORRUPTED_WRITE_TO_FILE);
        fclose(file);
        return -1;
    }

    curr_cursor++; curr_line_size++;

    if (c == NEWLINE) {
        
        int buffer_size = curr_line_size;
        if (buffer_size > 100) {
            buffer_size = 100;
        }

        char *line = get_last_line(buffer_size);

        *command = malloc(strlen(line) + 1);
        if (*command == NULL) {
            resolve_error(MEM_ALOC_FAILURE);
            free(line);
            fclose(file);
            return -1;
        }

        strcpy(*command, line);

        curr_line_size = 0;
        free(line);
    }

    fclose(file);
    return 0;
}

int render_terminal(px_t line_width)
{
    if (IS_TERMINAL_ENABLED) {

        IS_TERMINAL_TURNED_ON = true;

        FILE* file = fopen(TERMINAL_FILE, "rb");
        if (file == NULL) {
            resolve_error(UNOPENABLE_FILE);
            return -1;
        }

        int buffer_size = curr_line_size;
        if (buffer_size > 102) {
            buffer_size = 102;
        }

        char *line = get_last_line(buffer_size);

        put_horizontal_line(line_width - 1, '=');
        write_text("|| > ");
        write_text(line);
        put_text("||", line_width - 7 - buffer_size, RIGHT);
        put_horizontal_line(line_width - 1, '=');

        fclose(file);
        free(line);
        return 0;
    }
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

    if (fseek(file, curr_cursor - curr_line_size, SEEK_SET) != 0) {
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