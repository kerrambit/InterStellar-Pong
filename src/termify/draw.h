/**
 * @file draw.h
 * @author Marek Eibel
 * @brief Header file containing functions and data structures for graphics rendering and manipulation.
 * 
 * This file defines functions and data structures related to graphics rendering and manipulation,
 * including functions for drawing shapes, text, buttons, and managing pixel buffers and scenes.
 * 
 * @version 0.1
 * @date 2023-07-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef DRAW_H
#define DRAW_H

#include <stdbool.h>

#define UNDEFINIED_ID 0

/**
 * @typedef unsigned int px_t
 * @brief Alias for unsigned integer type used to represent pixel values.
 */
typedef unsigned int px_t;

/**
 * @typedef unsigned char ID_t
 * @brief Alias for unsigned char type used to represent object IDs.
 */
typedef unsigned char ID_t;

/**
 * @enum position_t
 * @brief Enumeration representing possible horizontal text positions.
 *
 * The `position_t` enumeration defines possible horizontal text positions.
 */
typedef enum position_t {
    LEFT,    /** Left-aligned text position. */
    CENTER,  /** Center-aligned text position. */
    RIGHT    /** Right-aligned text position. */
} position_t;

/**
 * @enum colour_t
 * @brief Enumeration representing possible colors used in graphics.
 *
 * The `colour_t` enumeration defines possible colors used in graphics.
 */
typedef enum colour_t {
    BLACK,          /** Black color. */
    WHITE,          /** White color. */
    RED,            /** Red color. */
    GREEN,          /** Green color. */
    BLUE,           /** Blue color. */
    YELLOW,         /** Yellow color. */
    ORANGE,          /** Orange color. */
    MAGENTA,        /** Magenta color. */
    CYAN,           /** Cyan color. */
    LIGHT_GRAY,     /** Light gray color. */
    DARK_GRAY,      /** Dark gray color. */
    LIGHT_RED,      /** Light red color. */
    LIGHT_GREEN,    /** Light green color. */
    LIGHT_BLUE,     /** Light blue color. */
    LIGHT_YELLOW,   /** Light yellow color. */
    LIGHT_MAGENTA,  /** Light magenta color. */
    LIGHT_CYAN      /** Light cyan color. */
} colour_t;

/**
 * @struct pixel_buffer_t
 * @brief Data structure representing a pixel buffer for graphics rendering.
 *
 * The `pixel_buffer_t` structure defines attributes for a pixel buffer used in graphics rendering.
 * Each pixel in pixel buffer contains ID of the object in the area.
 */
typedef struct pixel_buffer_t {
    px_t height;            /** Height of the pixel buffer. */
    px_t width;             /** Width of the pixel buffer. */
    ID_t* buff;             /** Pointer to the pixel buffer data. */
} pixel_buffer_t;

/**
 * @struct rectangle_t
 * @brief Data structure representing a drawable rectangle object.
 *
 * The `rectangle_t` structure defines attributes for a drawable rectangle object.
 * @note In the current state, the library does not support any other objects!
 */
typedef struct rectangle_t {
    ID_t ID;                 /** ID of the rectangle object. */
    px_t position_x;         /** Horizontal coordinate of the top-left corner of the rectangle. */
    px_t position_y;         /** Vertical coordinate of the top-left corner of the rectangle. */
    px_t width;      /** Length of one side of the rectangle. */
    px_t height;      /** Length of the other side of the rectangle. */
    int x_speed;             /** X-axis speed of the rectangle. */
    int y_speed;             /** Y-axis speed of the rectangle. */
    colour_t colour;         /** Color of the rectangle. */
    const char *name;        /** Name identifier of the rectangle. */
} rectangle_t;

/**
 * @struct scene_t
 * @brief Data structure representing a scene containing multiple drawable objects.
 *
 * The `scene_t` structure defines attributes for a scene containing multiple drawable rectangle objects.
 */
typedef struct scene_t {
    int number_of_objects;   /** Number of objects in the scene. */
    int length_of_arr;       /** Length of the objects array. */
    rectangle_t **scene;     /** Array of rectangle objects representing the scene. */
} scene_t;

/**
 * @brief Clears the terminal screen by sending escape codes.
 */
void clear_canvas(void);

/**
 * @brief Sets the cursor at the beginning of the terminal window.
 */
void set_cursor_at_beginning_of_window(void);

/**
 * @brief Sets the cursor at the beginning of the terminal canvas.
 */
void set_cursor_at_beginning_of_canvas(void);

/**
 * @brief Shows the cursor in the terminal.
 */
void show_cursor(void);

/**
 * @brief Hides the cursor in the terminal.
 */
void hide_cursor(void);

/**
 * @brief Draws borders around the terminal canvas.
 * 
 * @param height The height of the canvas.
 * @param width The width of the canvas.
 */
void draw_borders(px_t height, px_t width);

/**
 * @brief Puts text on the terminal with alignment options.
 * 
 * @param text The text to be displayed.
 * @param line_width The width of the line where the text will be aligned.
 * @param pos The alignment position (LEFT, CENTER, or RIGHT).
 */
void put_text(const char* text, px_t line_width, position_t pos);

/**
 * @brief Writes formatted text to the terminal.
 * 
 * @param format The format string for the text.
 * @param ... Additional arguments for formatting.
 */
void write_text(const char* format, ...);

/**
 * @brief Puts a button on the terminal with alignment options.
 * 
 * @param width The width of the terminal.
 * @param button_width The width of the button.
 * @param button_height The height of the button.
 * @param text The text displayed on the button.
 * @param pos The alignment position (LEFT, CENTER, or RIGHT).
 * @param row_mode Whether to move the cursor to the next row.
 * @param row_margin Additional margin in row_mode.
 */
void put_button(px_t width, px_t button_width, px_t button_height, const char *text, position_t pos, bool row_mode, px_t row_margin);

/**
 * @brief Renders an empty row on the terminal.
 * 
 * @param rows_count The number of rows to render.
 */
void put_empty_row(unsigned int rows_count);

/**
 * @brief Puts a horizontal line on the terminal.
 * 
 * @param line_width The width of the line.
 * @param symbol The character to use for the line.
 */
void put_horizontal_line(px_t line_width, char symbol);

/**
 * @brief Converts a colour enumeration value to a corresponding string.
 * 
 * @param colour The colour enumeration value.
 * @return The string representation of the colour.
 */
const char* colour_2_string(colour_t colour);

/**
 * @brief Creates a pixel buffer with a given height and width.
 * 
 * @param height The height of the pixel buffer.
 * @param width The width of the pixel buffer.
 * @return A pointer to the created pixel buffer.
 */
pixel_buffer_t *create_pixel_buffer(px_t height, px_t width);

/**
 * @brief Releases memory allocated for a scene and its objects.
 * 
 * @param scene The scene to release.
 */
void release_pixel_buffer(pixel_buffer_t *pixel_buffer);

/**
 * @brief Renders graphics on a pixel buffer using a scene.
 *        Each pixel contains ID of the object and each object has its color.
 * 
 * @param pixel_buffer The pixel buffer to render on.
 * @param scene The scene containing objects to be rendered.
 */
void render_graphics(pixel_buffer_t *pixel_buffer, scene_t *scene);

/**
 * @brief Computes and assigns pixel IDs in the pixel buffer for a given object.
 * 
 * @param pixel_buffer The pixel buffer to compute IDs in.
 * @param object The object for which to compute pixel IDs.
 * @return The assigned ID for the object.
 */
ID_t compute_object_pixels_in_buffer(pixel_buffer_t *pixel_buffer, rectangle_t *object);

/**
 * @brief Resets the pixel buffer by clearing assigned pixel IDs.
 * 
 * @param pixel_buffer The pixel buffer to reset.
 */
void reset_pixel_buffer(pixel_buffer_t *pixel_buffer);

/**
 * @brief Creates a rectangle object with specified attributes.
 * 
 * @param position_x The X position of the rectangle.
 * @param position_y The Y position of the rectangle.
 * @param width The length of one side of the rectangle.
 * @param height The length of the other side of the rectangle.
 * @param x_speed The speed of the rectangle along the X-axis.
 * @param y_speed The speed of the rectangle along the Y-axis.
 * @param colour The color of the rectangle.
 * @param name The name of the rectangle.
 * @return A pointer to the created rectangle object.
 */
rectangle_t *create_rectangle(px_t position_x, px_t position_y, px_t width, px_t height, int x_speed, int y_speed, colour_t colour, const char *name);

/**
 * @brief Releases memory allocated for a rectangle object.
 * 
 * @param rectangle The rectangle object to release.
 */
void release_rectangle(rectangle_t *rectangle);

/**
 * @brief Creates a scene for managing objects.
 * 
 * @return A pointer to the created scene.
 */
scene_t *create_scene();

/**
 * @brief Adds an object to a scene.
 * 
 * @param scene The scene to which the object will be added.
 * @param object The object to be added.
 * @return A pointer to the added object.
 */
rectangle_t *add_to_scene(scene_t *scene, rectangle_t *object);

/**
 * @brief Removes an object from a scene.
 * 
 * @param scene The scene from which the object will be removed.
 * @param object The object to be removed.
 * @return A pointer to the removed object.
 */
rectangle_t *remove_object_from_scene(scene_t *scene, rectangle_t *object);

/**
 * @brief Releases memory allocated for a scene and its objects.
 * 
 * @param scene The scene to release.
 */
void release_scene(scene_t *scene);

#endif
