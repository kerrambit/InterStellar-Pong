#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

#include "draw.h"
#include "errors.h"

// --------------------------------------------------------------------------------------------- //

void clear_canvas(void)
{
    printf("\033[2J\033[H");
}

void hide_cursor()
{
    printf("\033[?25l");
}

void show_cursor()
{
    printf("\033[?25h");
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

void put_horizontal_line(px_t line_width, char symbol)
{
    for (unsigned int i = 0; i < line_width; ++i) {
        putchar(symbol);
    }
    putchar('\n');
}

pixel_buffer_t *create_pixel_buffer(px_t height, px_t width)
{
    pixel_buffer_t *pixel_buffer = malloc(sizeof(pixel_buffer_t));
    if (pixel_buffer == NULL) {
        return NULL;
    }

    pixel_buffer->height = height; pixel_buffer->width = width;
    pixel_buffer->buff = calloc(height * width, sizeof(unsigned char));
    if (pixel_buffer->buff == NULL) {
        free(pixel_buffer);
        return NULL;
    }

    return pixel_buffer;
}

void render_graphics(pixel_buffer_t *pixel_buffer) {
    
    for (unsigned int i = 0; i < pixel_buffer->height; ++i) {
        for (unsigned int j = 0; j < pixel_buffer->width; ++j) {

            int pixel = pixel_buffer->buff[i * pixel_buffer->width + j];
            
            if (pixel == 0) {
                printf(" ");
            } else if (pixel == 1) {
                printf("\033[0;97m█\033[0m");
            }
        }
        putchar('\n');
    }
}

void release_pixel_buffer(pixel_buffer_t *pixel_buffer)
{
    free(pixel_buffer->buff);
    free(pixel_buffer);
}