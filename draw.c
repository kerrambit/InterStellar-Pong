#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "draw.h"

// --------------------------------------------------------------------------------------------- //

#define TERMINAL_FILE "user_input.data"

// --------------------------------------------------------------------------------------------- //


static bool IS_TERMINAL_TURNED_ON = false;
static bool IS_TERMINAL_ENABLED = false;
static unsigned long curr_cursor = 0;
static unsigned long curr_line_size = 0;

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

int save_char(char c)
{
    FILE* file = fopen(TERMINAL_FILE, "a");
    if (file == NULL) {
        printf("[I/O Error]: fail to open the file for appending.\n");
        return -1;
    }

    if (c == 127) {

        if (curr_cursor > 0 && curr_line_size > 0) {

            fseek(file, -1, SEEK_END);
            int truncate_result = ftruncate(fileno(file), ftell(file));

            if (truncate_result != 0) {
                printf("[I/O Error]: truncating of the file failed.\n");
                fclose(file);
                return -1;
            }

            curr_cursor--; curr_line_size--;
        }

        return 0;
    }

    if (fputc(c, file) == EOF) {
        printf("[I/O Error]: fail to append the character to the file.\n");
        fclose(file);
        return -1;
    }

    fclose(file);
    curr_cursor++; curr_line_size++;
    if (c == '\n') {
        curr_line_size = 0;
    }
    return 0;
}

int render_terminal(px_t line_width)
{
    if (IS_TERMINAL_ENABLED) {

        IS_TERMINAL_TURNED_ON = true;

        put_horizontal_line(line_width, '=');
        put_text("|| > ", line_width, LEFT);

        int buffer = curr_line_size;
        if (buffer > 100) {
            buffer = 100;
        }

        char line[buffer];
        FILE* file = fopen(TERMINAL_FILE, "rb");
        if (file == NULL) {
            printf("[I/O Error]: opening of the file failed.\n");
            return -1;
        }

        fseek(file, curr_cursor - curr_line_size, SEEK_SET);
        size_t bytes_read = fread(&line, sizeof(char), buffer, file);
        line[buffer] = '\0';
        printf("%s", line);
        
        fclose(file);
        put_text("||\n", line_width - 4 - buffer, RIGHT);
        put_horizontal_line(line_width, '=');

        return 0;
    }
}

void put_horizontal_line(px_t line_width, char symbol)
{
    for (unsigned int i = 0; i < line_width; ++i) {
        putchar(symbol);
    }
    putchar('\n');
}