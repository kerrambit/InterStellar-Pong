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

#include <stdbool.h>

typedef unsigned int px_t;

typedef enum position_t {
    LEFT,
    CENTER,
    RIGHT
} position_t;

typedef enum colour_t {
    BLACK,
    WHITE,
    RED,
    GREEN,
    BLUE,
    YELLOW
} colour_t;

typedef enum object_type_t {
    SQUARE,
    CIRCLE,
    RECTANGLE
} object_type_t;

typedef struct pixel_buffer_t {
    px_t height;
    px_t width;
    unsigned char* buff;
} pixel_buffer_t;

typedef struct square_t {
    px_t position_x;
    px_t position_y;
    px_t side_length;
    colour_t colour;
} square_t;

typedef struct rectangle_t {
    px_t position_x;
    px_t position_y;
    px_t side_length_1;
    px_t side_length_2;
    colour_t colour;
} rectangle_t;

typedef struct circle_t {
    px_t position_x;
    px_t position_y;
    px_t radius;
    colour_t colour;
    colour_t fill_colour;
} circle_t;

void clear_canvas(void);
void set_cursor_at_beginning_of_canvas(void);
void show_cursor(void);
void hide_cursor(void);
void put_text(const char* text, px_t line_width, position_t pos);
void write_text(const char* text);
void put_empty_row(unsigned int rows_count);
void put_horizontal_line(px_t line_width, char symbol);

pixel_buffer_t *create_pixel_buffer(px_t height, px_t width);
void release_pixel_buffer(pixel_buffer_t *pixel_buffer);
void render_graphics(pixel_buffer_t *pixel_buffer);
void bind_obj_to_pixel_buffer(pixel_buffer_t *pixel_buffer, void *obj, object_type_t obj_type);
void reset_pixel_buffer(pixel_buffer_t *pixel_buffer);

square_t *create_square(px_t position_x, px_t position_y, px_t side_length, colour_t colour);
circle_t *create_circle(px_t position_x, px_t position_y, px_t radius, colour_t colour, colour_t fill_colour);
rectangle_t *create_rectangle(px_t position_x, px_t position_y, px_t side_length_1, px_t side_length_2, colour_t colour);
void release_square(square_t *square);
void release_circle(circle_t *square);
void release_rectangle(rectangle_t *rectangle);

#endif
