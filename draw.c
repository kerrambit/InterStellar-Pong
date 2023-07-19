#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

#include "draw.h"
#include "errors.h"

// --------------------------------------------------------------------------------------------- //

#define TERMINAL_FILE "user_input.data"
#define BACKSPACE 127

// --------------------------------------------------------------------------------------------- //


static bool IS_TERMINAL_TURNED_ON = false;
static bool IS_TERMINAL_ENABLED = false;
static unsigned long curr_cursor = 0;
static unsigned long curr_line_size = 0;

// --------------------------------------------------------------------------------------------- //

static char* get_last_line(int buffer_size);

// --------------------------------------------------------------------------------------------- //

void clear_canvas(void)
{
    printf("\033[2J\033[H");
}

void put_text(const char* text, px_t line_width, position_t pos)
{
    px_t text_length = strlen(text);

    switch (pos)
    {
    case LEFT:
        printf("%s", text);
        break;
    case CENTER:
        px_t center_padding = (line_width - text_length) / 2;
        for (int i = 0; i < center_padding; i++) {
            putchar(' ');
        }
        printf("%s", text);
        break;
    case RIGHT:
        px_t right_padding = (line_width - text_length);
        for (int i = 0; i < right_padding; i++) {
            putchar(' ');
        }
        printf("%s", text);
        break;
    default:
        break;
    }
}

void write_text(const char* text)
{
    printf("%s", text);
}

void put_empty_row(unsigned int rows_count)
{
    for (unsigned int i = 0; i < rows_count; ++i) {
        putchar('\n');
    }
}

int enable_terminal()
{
    // TO-DO: if the file exists, remove it
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

    if (c == '\n') {
        
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
        if (buffer_size > 100) {
            buffer_size = 100;
        }

        char *line = get_last_line(buffer_size);

        put_horizontal_line(line_width, '=');
        put_text("|| > ", line_width, LEFT);
        write_text(line);
        put_text("||\n", line_width - 4 - buffer_size, RIGHT);
        put_horizontal_line(line_width, '=');

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

void put_horizontal_line(px_t line_width, char symbol)
{
    for (unsigned int i = 0; i < line_width; ++i) {
        putchar(symbol);
    }
    putchar('\n');
}