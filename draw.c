#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "draw.h"

bool IS_TERMINAL_TURNED_ON = false;
bool IS_TERMINAL_ENABLED = false;

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

void put_space(unsigned int rows_count)
{
    for (unsigned int i = 0; i < rows_count; ++i) {
        putchar('\n');
    }
}

int enable_terminal()
{
    FILE* file = fopen("user_input.data", "w");
    if (file == NULL) {
        return -1;
    }

    fclose(file);
    IS_TERMINAL_ENABLED = true;
    return 0;
}

void remove_terminal_data()
{
    if (remove("user_input.data") == 0) {
    } else {
        printf("[I/O Error]: while removing the file 'user_input.data', error occured.\n");
    }

    IS_TERMINAL_TURNED_ON = false;
    IS_TERMINAL_ENABLED = false;
}

int save_char(char c)
{
    FILE* file = fopen("user_input.data", "a");
    if (file == NULL) {
        printf("[I/O Error]: fail to open the file for appending.\n");
        return -1;
    }

    if (fputc(c, file) == EOF) {
        printf("[I/O Error]: fail to append the character to the file.\n");
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}

void render_terminal(px_t line_width)
{
    if (IS_TERMINAL_ENABLED) {

        IS_TERMINAL_TURNED_ON = true;

        put_horizontal_line(line_width, '=');
        put_text("|| > ", line_width, LEFT);
        put_text("||\n", line_width - 4, RIGHT);
        put_horizontal_line(line_width, '=');
    }
}

void put_horizontal_line(px_t line_width, char symbol)
{
    for (unsigned int i = 0; i < line_width; ++i) {
        putchar(symbol);
    }
    putchar('\n');
}