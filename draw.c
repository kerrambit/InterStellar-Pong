#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

#include "draw.h"
#include "errors.h"
#include "utils.h"

// --------------------------------------------------------------------------------------------- //

#define CURSOR_TO_BEGINNING_OF_LINE() printf("\r")
#define CHAR_RIGHT() printf("\033[C")
#define ROW_DOWN() printf("\033[B")
#define ROW_UP() printf("\033[A")

// --------------------------------------------------------------------------------------------- //

static unsigned char ID = 6;

// --------------------------------------------------------------------------------------------- //

static void display_button(px_t width, px_t height, px_t padding, const char *text);
static void display_button_text(px_t width, const char *text);
static ID_t generate_id();
static colour_t ID_to_colour(scene_t *scene, ID_t ID);

// --------------------------------------------------------------------------------------------- //

void clear_canvas(void)
{
    printf("\033[2J\033[H");
}

void set_cursor_at_beginning_of_window(void)
{
    printf("\033[H");
}

void set_cursor_at_beginning_of_canvas(void)
{
    set_cursor_at_beginning_of_window();
    put_empty_row(1);
    CHAR_RIGHT();
}

void hide_cursor()
{
    printf("\033[?25l");
}

void show_cursor()
{
    printf("\033[?25h");
}

void draw_borders(px_t height, px_t width)
{
    set_cursor_at_beginning_of_window();
    width += 1; height += 1;

    printf("┌");
    for (int i = 0; i < width - 2; ++i) {
        printf("─");
    }
    printf("┐\n");

    for (int i = 0; i < height - 2; ++i) {
        printf("│");
        for (int j = 0; j < width - 2; ++j) {
            printf(" ");
        }
        printf("│\n");
    }

    printf("└");
    for (int i = 0; i < width - 2; ++i) {
        printf("─");
    }
    printf("┘\n");
}

void put_text(const char* text, px_t line_width, position_t pos)
{
    px_t text_length = strlen(text);

    switch (pos)
    {
    case LEFT:
        CHAR_RIGHT();
        printf("%s", text);
        break;

    case CENTER:
        CHAR_RIGHT();
        px_t center_padding = (line_width - text_length) / 2;
        for (int i = 0; i < center_padding; i++) {
            putchar(' ');
        }
        printf("%s", text);
        break;

    case RIGHT:
        CHAR_RIGHT();
        px_t right_padding = (line_width - text_length);
        for (int i = 0; i < right_padding - 1; i++) {
            putchar(' ');
        }
        printf("%s", text);
        break;
    default:
        break;
    }
    CURSOR_TO_BEGINNING_OF_LINE();
    ROW_DOWN();
}

void write_text(const char* text)
{
    CHAR_RIGHT();
    printf("%s", text);
}

void put_button(px_t width, px_t button_width, px_t button_height, const char *text, position_t pos, bool row_mode, px_t row_margin)
{
    switch (pos)
    {
    case LEFT:
        display_button(button_width, button_height, 1 + row_margin, text);
        break;
    case CENTER:
        px_t center_padding = (width - button_width) / 2;
        display_button(button_width, button_height, center_padding + row_margin, text);
        break;
    case RIGHT:
        px_t right_padding = (width - button_width) - 1;
        display_button(button_width, button_height, right_padding + row_margin, text);
        break;
    default:
        break;
    }

    if (row_mode) {
        for (int i = 0; i < button_height - 1; ++i) { ROW_UP(); }
    } else {
        CURSOR_TO_BEGINNING_OF_LINE();
        ROW_DOWN();
    }
}

static void display_button(px_t width, px_t height, px_t padding, const char *text)
{
    CURSOR_TO_BEGINNING_OF_LINE();
    CHAR_RIGHT();

    for (int i = 0; i < padding; ++i) {CHAR_RIGHT();}

    printf("┌");
    for (int i = 0; i < width - 2; ++i) {
        printf("─");
    }
    printf("┐\n");

    for (int i = 0; i < height - 2; ++i) {
        CHAR_RIGHT();
        for (int i = 0; i < padding; ++i) {CHAR_RIGHT();}
        printf("│");

        if (i == (height - 2) / 2) {
            display_button_text(width, text);
        } else {
            for (int j = 0; j < width - 2; ++j) {
                printf(" ");
            }
        }
        printf("│\n");
    }

    CHAR_RIGHT();
    for (int i = 0; i < padding; ++i) {CHAR_RIGHT();}

    printf("└");
    for (int i = 0; i < width - 2; ++i) {
        printf("─");
    }
    printf("┘");
}

static void display_button_text(px_t width, const char *text)
{
    int text_length_equalizer = 0;

    if (strlen(text) % 2 != 0) {
        text_length_equalizer--;
    }
    if (width % 2 != 0) {
        text_length_equalizer--;
    }

    for (int j = 0; j < (width - 1) / 2 - (strlen(text) / 2); ++j) {
        printf(" ");
    }

    printf("%s", text);

    for (int j = 0; j < (width - 1) / 2 - (strlen(text) / 2) + text_length_equalizer; ++j) {
        printf(" ");
    }
}

void put_empty_row(unsigned int rows_count)
{
    for (unsigned int i = 0; i < rows_count; ++i) {
        ROW_DOWN();
    }
}

void put_horizontal_line(px_t line_width, char symbol)
{
    CHAR_RIGHT();
    for (unsigned int i = 0; i < line_width; ++i) {
        putchar(symbol);
    }
    CURSOR_TO_BEGINNING_OF_LINE();
    ROW_DOWN();
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

scene_t *create_scene()
{
    scene_t *scene = malloc(sizeof(scene_t));
    if (scene == NULL) {
        return NULL;
    }

    scene->number_of_objects = 0;
    scene->length_of_arr = 4;
    scene->scene = malloc(sizeof(rectangle_t*) * scene->length_of_arr);

    if (scene->scene == NULL) {
        free(scene);
        return NULL;
    }

    return scene;
}

void release_scene(scene_t *scene) {

    if (scene == NULL) {
        return;
    }

    for (int i = 0; i < scene->number_of_objects; ++i) {
        release_rectangle(scene->scene[i]);
    }

    free(scene->scene);
    free(scene);
}

rectangle_t *add_to_scene(scene_t *scene, rectangle_t *object) {

    if (scene == NULL || object == NULL) {
        return NULL;
    }

    if (scene->number_of_objects >= scene->length_of_arr) {
        scene->length_of_arr *= 2;
        rectangle_t **new_scene_arr = realloc(scene->scene, sizeof(rectangle_t*) * scene->length_of_arr);
        if (new_scene_arr == NULL) {
            return NULL;
        }
        scene->scene = new_scene_arr;
    }

    scene->scene[scene->number_of_objects++] = object;
    return object;
}

void render_graphics(pixel_buffer_t *pixel_buffer, scene_t *scene)
{
    for (unsigned int i = 0; i < pixel_buffer->height; ++i) {
        for (unsigned int j = 0; j < pixel_buffer->width; ++j) {

            int ID = pixel_buffer->buff[i * pixel_buffer->width + j];
            colour_t pixel = ID_to_colour(scene, ID);
            
            switch (pixel) {
                case BLACK:
                    CHAR_RIGHT(); break;
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
                    CHAR_RIGHT(); break;
            }
        }
        putchar('\n');
    }
}

void release_pixel_buffer(pixel_buffer_t *pixel_buffer)
{
    if (pixel_buffer != NULL) {
        free(pixel_buffer->buff);
        free(pixel_buffer);   
    }
}

ID_t compute_object_pixels_in_buffer(pixel_buffer_t *pixel_buffer, void *obj, object_type_t obj_type)
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
                    if (pixel_buffer->buff[i * pixel_buffer->width + j] != UNDEFINIED_ID) {
                        return pixel_buffer->buff[i * pixel_buffer->width + j];
                    } else {
                        pixel_buffer->buff[i * pixel_buffer->width + j] = rectangle_obj->ID;
                    }
                }   
            }
        }
        break;

    default:
        break;
    }
    return UNDEFINIED_ID;
}

void reset_pixel_buffer(pixel_buffer_t *pixel_buffer)
{
    for (int i = 0; i < pixel_buffer->height; i++) {
        for (int j = 0; j < pixel_buffer->width; j++) {
            pixel_buffer->buff[i * pixel_buffer->width + j] = UNDEFINIED_ID;
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

rectangle_t *create_rectangle(px_t position_x, px_t position_y, px_t side_length_1, px_t side_length_2, int x_speed, int y_speed, colour_t colour, const char *name)
{
    rectangle_t *rectangle = malloc(sizeof(rectangle_t));
    if (rectangle == NULL) {
        return NULL;
    }

    rectangle->ID = generate_id();
    rectangle->position_x = position_x; rectangle->position_y = position_y;
    rectangle->x_speed = x_speed; rectangle->y_speed = y_speed;
    rectangle->side_length_1 = side_length_1 * 2; rectangle->side_length_2 = side_length_2;
    rectangle->colour = colour;
    rectangle->name = malloc(strlen(name) + 1);

    if (rectangle->name == NULL) {
        free(rectangle);
        return NULL;
    }

    strcpy((char*)rectangle->name, name);

    return rectangle;
}

void release_rectangle(rectangle_t *rectangle)
{
    if (rectangle != NULL) {
        free((char*)rectangle->name);
        free(rectangle);
    }
}

const char* colour_2_string(colour_t colour)
{
    switch (colour)
    {
    case BLACK:    return "black"; break;
    case WHITE:    return "white"; break;
    case RED:      return "red"; break;
    case GREEN:    return "green"; break;
    case BLUE:     return "blue"; break;
    case YELLOW:   return "yellow"; break;
    default:       return "unknown"; break;
    }
}

unsigned char generate_id()
{
    return ++ID;
}

static colour_t ID_to_colour(scene_t *scene, ID_t ID)
{
    for (int i = 0; i < scene->number_of_objects; ++i) {
        if (scene->scene[i]->ID == ID) {
            return scene->scene[i]->colour;
        }
    }
    return BLACK;
}