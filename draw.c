#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

#include "draw.h"
#include "errors.h"

// --------------------------------------------------------------------------------------------- //

#define SQUARE(num) (num * num)

// --------------------------------------------------------------------------------------------- //

void clear_canvas(void)
{
    printf("\033[2J\033[H");
}

void set_cursor_at_beginning_of_canvas(void)
{
    printf("\033[H");
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
            
            switch (pixel) {
                case BLACK:
                    printf(" "); break;
                case WHITE:
                    printf("\033[0;97m█\033[0m"); break;
                case RED:
                    printf("\033[0;91m█\033[0m"); break;
                case GREEN:
                    printf("\033[0;92m█\033[0m"); break;
                case BLUE:
                    printf("\033[0;94m█\033[0m"); break;
                case YELLOW:
                    printf("\033[0;93m█\033[0m"); break;
                default:
                    printf(" "); break;
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

colour_t compute_object_pixels_in_buffer(pixel_buffer_t *pixel_buffer, void *obj, object_type_t obj_type)
{
    switch (obj_type)
    {

    case CIRCLE:
        circle_t *circle_obj = (circle_t*) obj;
        const int HEIGHT = 2 * circle_obj->radius;
        const int WIDTH = 4 * circle_obj->radius;

        for (px_t i = 0; i < HEIGHT + 1; ++i) {
            for (px_t j = 0 ; j < WIDTH - 1; ++j) {

                int x = j - WIDTH / 2;
                int y = i - HEIGHT / 2;

                if (SQUARE(x) + SQUARE(y) < SQUARE(circle_obj->radius)) {
                    int pixel_x = circle_obj->position_x + x;
                    int pixel_y = circle_obj->position_y + y;
                    if (pixel_x >= 0 && pixel_x < pixel_buffer->width && pixel_y >= 0 && pixel_y < pixel_buffer->height) {
                        pixel_buffer->buff[pixel_y * pixel_buffer->width + pixel_x] = circle_obj->colour;
                    }
                }
            }
        }
        break;

    case RECTANGLE:
        rectangle_t *rectangle_obj = (rectangle_t*)obj;
        for (px_t i = rectangle_obj->position_y; i < rectangle_obj->position_y + rectangle_obj->side_length_2; ++i) {
            for (px_t j = rectangle_obj->position_x; j < rectangle_obj->position_x + rectangle_obj->side_length_1; ++j) {
                if((i * pixel_buffer->width + j) >= 0 && (i * pixel_buffer->width + j) < pixel_buffer->height * pixel_buffer->width) {
                    if (pixel_buffer->buff[i * pixel_buffer->width + j] != BLACK) {
                        return pixel_buffer->buff[i * pixel_buffer->width + j];
                    } else {
                        pixel_buffer->buff[i * pixel_buffer->width + j] = rectangle_obj->colour;
                    }
                }   
            }
        }
        break;

    default:
        break;
    }
    return BLACK;
}

void reset_pixel_buffer(pixel_buffer_t *pixel_buffer)
{
    for (int i = 0; i < pixel_buffer->height; i++) {
        for (int j = 0; j < pixel_buffer->width; j++) {
            pixel_buffer->buff[i * pixel_buffer->width + j] = BLACK;
        }
    }
}

circle_t *create_circle(px_t position_x, px_t position_y, px_t radius, colour_t colour, colour_t fill_colour)
{
    circle_t *circle = malloc(sizeof(circle_t));
    if (circle == NULL) {
        return NULL;
    }

    circle->position_x = position_x; circle->position_y = position_y; circle->radius = radius; circle->colour = colour; circle->fill_colour = fill_colour;
    return circle;
}

void release_circle(circle_t *circle)
{
    free(circle);
}

rectangle_t *create_rectangle(px_t position_x, px_t position_y, px_t side_length_1, px_t side_length_2, int x_speed, int y_speed, colour_t colour)
{
    rectangle_t *rectangle = malloc(sizeof(rectangle_t));
    if (rectangle == NULL) {
        return NULL;
    }

    rectangle->position_x = position_x; rectangle->position_y = position_y, rectangle->colour = colour;
    rectangle->x_speed = x_speed; rectangle->y_speed = y_speed;
    rectangle->side_length_1 = side_length_1 * 2; rectangle->side_length_2 = side_length_2;

    return rectangle;
}

void release_rectangle(rectangle_t *rectangle)
{
    free(rectangle);
}

const char* colour_2_string(colour_t colour)
{
    switch (colour)
    {
    case BLACK: return "black"; break;
    case WHITE: return "white"; break;
    case RED:   return "red"; break;
    case GREEN:   return "green"; break;
    case BLUE:   return "blue"; break;
    case YELLOW:   return "yellow"; break;
    default: return "unknown"; break;
    }
}