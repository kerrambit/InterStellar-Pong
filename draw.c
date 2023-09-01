#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "errors.h"
#include "draw.h"
#include "utils.h"

// ---------------------------------------- MACROS --------------------------------------------- //

#define CURSOR_TO_BEGINNING_OF_LINE() printf("\r")
#define CHAR_RIGHT() printf("\033[C")
#define CHAR_LEFT() printf("\033[D")
#define ROW_DOWN() printf("\033[B")
#define ROW_UP() printf("\033[A")

// ------------------------------------ GLOBAL VARIABLE----------------------------------------- //

/**
 * @brief Global ID allocator for scene objects.
 *
 * The `gl_ID_allocater` is a global variable used to assign unique IDs to objects
 * added to the scene. It ensures that each object has a distinct identifier within
 * the scene. The IDs are allocated sequentially as objects are added to the scene.
 * This variable is used for tracking and managing scene objects.
 *
 * Note that the range of available IDs is limited by the range of an unsigned char.
 * Once the maximum ID value is reached, the allocator will wrap around and start
 * reusing IDs.
 */
static ID_t gl_ID_allocater = 0;

// ---------------------------------- STATIC DECLARATIONS--------------------------------------- //

static void display_button(px_t width, px_t height, px_t padding, const char *text);
static void display_button_text(px_t width, const char *text);
static colour_t ID_to_colour(scene_t *scene, ID_t ID);
static ID_t generate_id();

// ----------------------------------------- PROGRAM-------------------------------------------- //

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
        break;
    case CENTER:
        CHAR_RIGHT();
        px_t center_padding = (line_width - text_length) / 2;
        for (int i = 0; i < center_padding; i++) {
            putchar(' ');
        }
        break;
    case RIGHT:
        CHAR_RIGHT();
        px_t right_padding = (line_width - text_length);
        for (int i = 0; i < right_padding - 1; i++) {
            putchar(' ');
        }
        break;
    default:
        break;
    }

    printf("%s", text);
    CURSOR_TO_BEGINNING_OF_LINE();
    ROW_DOWN();
}

void write_text(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    
    CHAR_RIGHT();
    vprintf(format, args);
    
    va_end(args);
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
        for (int i = 0; i < button_height - 1; ++i) {
            ROW_UP();
        }
    } else {
        CURSOR_TO_BEGINNING_OF_LINE();
        ROW_DOWN();
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
        resolve_error(MEM_ALOC_FAILURE);
        return NULL;
    }

    pixel_buffer->height = height; pixel_buffer->width = width;
    pixel_buffer->buff = calloc(height * width, sizeof(unsigned char));

    if (pixel_buffer->buff == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        free(pixel_buffer);
        return NULL;
    }

    return pixel_buffer;
}

scene_t *create_scene()
{
    const int BEGIN_ARRAY_SIZE = 4;
    scene_t *scene = malloc(sizeof(scene_t));
    if (scene == NULL) {
        return NULL;
    }

    scene->number_of_objects = 0;
    scene->length_of_arr = BEGIN_ARRAY_SIZE;
    scene->scene = malloc(sizeof(rectangle_t*) * scene->length_of_arr);

    if (scene->scene == NULL) {
        free(scene);
        return NULL;
    }

    return scene;
}

void release_scene(scene_t *scene)
{

    if (scene == NULL) {
        return;
    }

    for (int i = 0; i < scene->number_of_objects; ++i) {
        release_rectangle(scene->scene[i]);
    }

    free(scene->scene);
    free(scene);
}

rectangle_t *add_to_scene(scene_t *scene, rectangle_t *object)
{
    const int GROWTH_FACTOR = 2;

    if (scene == NULL || object == NULL) {
        return NULL;
    }

    if (scene->number_of_objects >= scene->length_of_arr) {
        scene->length_of_arr *= GROWTH_FACTOR;
        rectangle_t **new_scene_arr = realloc(scene->scene, sizeof(rectangle_t*) * scene->length_of_arr);
        if (new_scene_arr == NULL) {
            return NULL;
        }
        scene->scene = new_scene_arr;
    }

    scene->scene[scene->number_of_objects++] = object;
    return object;
}

rectangle_t *remove_object_from_scene(scene_t *scene, rectangle_t *object)
{
    if (scene == NULL || object == NULL) {
        return NULL;
    }

    int index = -1;
    for (int i = 0; i < scene->number_of_objects; i++) {
        if (scene->scene[i] == object) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        return NULL;
    }

    for (int i = index; i < scene->number_of_objects - 1; i++) {
        scene->scene[i] = scene->scene[i + 1];
    }

    scene->number_of_objects--;

    if (scene->number_of_objects < scene->length_of_arr / 2) {
        scene->length_of_arr /= 2;
        rectangle_t **new_scene_arr = realloc(scene->scene, sizeof(rectangle_t*) * scene->length_of_arr);
        if (new_scene_arr == NULL) {
            return NULL;
        }
        scene->scene = new_scene_arr;
    }

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
                case ORANGE:
                    printf("\033[38;5;208m█\033[0m"); break; 
                case MAGENTA:
                    printf("\033[0;95m█\033[0m"); break;
                case CYAN:
                    printf("\033[0;96m█\033[0m"); break;
                case LIGHT_GRAY:
                    printf("\033[0;37m█\033[0m"); break;
                case DARK_GRAY:
                    printf("\033[0;90m█\033[0m"); break;
                case LIGHT_RED:
                    printf("\033[0;31m█\033[0m"); break;
                case LIGHT_GREEN:
                    printf("\033[0;32m█\033[0m"); break;
                case LIGHT_BLUE:
                    printf("\033[0;34m█\033[0m"); break;
                case LIGHT_YELLOW:
                    printf("\033[0;33m█\033[0m"); break;
                case LIGHT_MAGENTA:
                    printf("\033[0;35m█\033[0m"); break;
                case LIGHT_CYAN:
                    printf("\033[0;36m█\033[0m"); break;
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

ID_t compute_object_pixels_in_buffer(pixel_buffer_t *pixel_buffer, rectangle_t *object)
{
    if (object == NULL) {
        return UNDEFINIED_ID;
    }

    for (px_t i = object->position_y; i < object->position_y + object->height; ++i) {
        for (px_t j = object->position_x; j < object->position_x + object->width; ++j) {

            if ((i * pixel_buffer->width + j) >= 0 && (i * pixel_buffer->width + j) < pixel_buffer->height * pixel_buffer->width) {
                if (pixel_buffer->buff[i * pixel_buffer->width + j] != UNDEFINIED_ID) {
                    return pixel_buffer->buff[i * pixel_buffer->width + j];
                } else {
                    pixel_buffer->buff[i * pixel_buffer->width + j] = object->ID;
                }
            }   
        }
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

rectangle_t *create_rectangle(px_t position_x, px_t position_y, px_t width, px_t height, int x_speed, int y_speed, colour_t colour, const char *name)
{
    rectangle_t *rectangle = malloc(sizeof(rectangle_t));
    if (rectangle == NULL) {
        return NULL;
    }

    rectangle->ID = generate_id();
    rectangle->position_x = position_x; rectangle->position_y = position_y;
    rectangle->x_speed = x_speed; rectangle->y_speed = y_speed;
    rectangle->width = width * 2; rectangle->height = height;
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
        case BLACK:         return "black";
        case WHITE:         return "white";
        case RED:           return "red";
        case GREEN:         return "green";
        case BLUE:          return "blue";
        case YELLOW:        return "yellow";
        case ORANGE:        return "orange";
        case MAGENTA:       return "magenta";
        case CYAN:          return "cyan";
        case LIGHT_GRAY:    return "light_gray";
        case DARK_GRAY:     return "dark_gray";
        case LIGHT_RED:     return "light_red";
        case LIGHT_GREEN:   return "light_green";
        case LIGHT_BLUE:    return "light_blue";
        case LIGHT_YELLOW:  return "light_yellow";
        case LIGHT_MAGENTA: return "light_magenta";
        case LIGHT_CYAN:    return "light_cyan";
        default:            return "unknown";
    }
}

/**
 * @brief Displays a button with specified dimensions and text.
 * 
 * @param width The width of the button.
 * @param height The height of the button.
 * @param padding The padding around the button.
 * @param text The text displayed on the button.
 */
static void display_button(px_t width, px_t height, px_t padding, const char *text)
{
    CURSOR_TO_BEGINNING_OF_LINE();
    CHAR_RIGHT();

    for (int i = 0; i < padding; ++i) {
        CHAR_RIGHT();
    }

    printf("┌");
    for (int i = 0; i < width - 2; ++i) {
        printf("─");
    }
    printf("┐\n");

    for (int i = 0; i < height - 2; ++i) {
        CHAR_RIGHT();
        for (int i = 0; i < padding; ++i) {
            CHAR_RIGHT();
        }
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
    for (int i = 0; i < padding; ++i) {
        CHAR_RIGHT();
    }

    printf("└");
    for (int i = 0; i < width - 2; ++i) {
        printf("─");
    }
    printf("┘");
}

/**
 * @brief Displays the text content inside a button.
 * 
 * @param width The width of the button.
 * @param text The text to be displayed inside the button.
 */
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

/**
 * @brief Generates a unique ID for objects in the scene.
 * 
 * @return The generated unique ID.
 */
static ID_t generate_id()
{
    return ++gl_ID_allocater;
}

/**
 * @brief Maps an ID to a colour for rendering.
 * 
 * @param scene The scene containing objects.
 * @param ID The ID to map to a colour.
 * @return The colour corresponding to the given ID.
 */
static colour_t ID_to_colour(scene_t *scene, ID_t ID)
{
    for (int i = 0; i < scene->number_of_objects; ++i) {
        if (scene->scene[i]->ID == ID) {
            return scene->scene[i]->colour;
        }
    }
    return BLACK;
}
