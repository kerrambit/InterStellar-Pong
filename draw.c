#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "draw.h"

bool is_terminal_enabled = false;

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

void render_terminal(px_t line_width)
{
    is_terminal_enabled = true;

    put_horizontal_line(line_width, '=');
    put_text("|| > ", line_width, LEFT);
    put_text("||\n", line_width - 4, RIGHT);
    put_horizontal_line(line_width, '=');
}

void put_horizontal_line(px_t line_width, char symbol)
{
    for (unsigned int i = 0; i < line_width; ++i) {
        putchar(symbol);
    }
    putchar('\n');
}