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
typedef unsigned char ID_t;
#define UNDEFINIED_ID 0

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
    YELLOW,
    MAGENTA,
    CYAN,
    LIGHT_GRAY,
    DARK_GRAY,
    LIGHT_RED,
    LIGHT_GREEN,
    LIGHT_BLUE,
    LIGHT_YELLOW,
    LIGHT_MAGENTA,
    LIGHT_CYAN
} colour_t;

typedef enum object_type_t {
    CIRCLE,
    RECTANGLE
} object_type_t;

typedef struct pixel_buffer_t {
    px_t height;
    px_t width;
    unsigned char* buff;
} pixel_buffer_t;

typedef struct rectangle_t {
    ID_t ID;
    px_t position_x;
    px_t position_y;
    px_t side_length_1;
    px_t side_length_2;
    int x_speed;
    int y_speed;
    colour_t colour;
    const char *name;
} rectangle_t;

typedef struct scene_t {
    int number_of_objects;
    int length_of_arr;
    rectangle_t **scene;
} scene_t;

typedef struct circle_t {
    px_t position_x;
    px_t position_y;
    px_t radius;
    colour_t colour;
    colour_t fill_colour;
} circle_t;

void clear_canvas(void);
void set_cursor_at_beginning_of_window(void);
void set_cursor_at_beginning_of_canvas(void);
void show_cursor(void);
void hide_cursor(void);

void draw_borders(px_t height, px_t width);
void put_text(const char* text, px_t line_width, position_t pos);
void write_text(const char* format, ...);
void put_button(px_t width, px_t button_width, px_t button_height, const char *text, position_t pos, bool row_mode, px_t row_margin);
void put_empty_row(unsigned int rows_count);
void put_horizontal_line(px_t line_width, char symbol);
const char* colour_2_string(colour_t colour);

pixel_buffer_t *create_pixel_buffer(px_t height, px_t width);
void release_pixel_buffer(pixel_buffer_t *pixel_buffer);
void render_graphics(pixel_buffer_t *pixel_buffer, scene_t *scene);
ID_t compute_object_pixels_in_buffer(pixel_buffer_t *pixel_buffer, void *obj, object_type_t obj_type);
void reset_pixel_buffer(pixel_buffer_t *pixel_buffer);

circle_t *create_circle(px_t position_x, px_t position_y, px_t radius, colour_t colour, colour_t fill_colour);
rectangle_t *create_rectangle(px_t position_x, px_t position_y, px_t side_length_1, px_t side_length_2, int x_speed, int y_speed, colour_t colour, const char *name);
void release_circle(circle_t *square);
void release_rectangle(rectangle_t *rectangle);

scene_t *create_scene();
rectangle_t *add_to_scene(scene_t *scene, rectangle_t *object);
rectangle_t *remove_object_from_scene(scene_t *scene, rectangle_t *object);
void release_scene(scene_t *scene);

#endif
