/**
 * @file draw.h
 * @author Marek Eibel
 * @brief Groups all drawing functions, macros and enums.
 * @version 0.1
 * @date 2023-07-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef DRAW_H
#define DRAW_H

typedef unsigned int px_t;

typedef enum position_t {
    LEFT,
    CENTER,
    RIGHT
} position_t;

typedef struct pixel_buffer_t {
    px_t height;
    px_t width;
    unsigned char* buff;
} pixel_buffer_t;

void clear_canvas(void);
void show_cursor(void);
void hide_cursor(void);
void put_text(const char* text, px_t line_width, position_t pos);
void write_text(const char* text);
void put_empty_row(unsigned int rows_count);
void put_horizontal_line(px_t line_width, char symbol);

pixel_buffer_t *create_pixel_buffer(px_t height, px_t width);
void release_pixel_buffer(pixel_buffer_t *pixel_buffer);
void render_graphics(pixel_buffer_t *pixel_buffer);
// void bind_pixel_buffer(pixel_buffer_t *pixel_buffer, );

#endif
