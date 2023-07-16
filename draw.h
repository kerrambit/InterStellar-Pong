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

typedef enum {
    LEFT,
    CENTER,
    RIGHT
} position_t;

void clear_canvas(void);
void put_text(const char* text, px_t line_width, position_t pos);
void put_space(unsigned int rows_count);
void render_terminal(px_t line_width);
void put_horizontal_line(px_t line_width, char symbol);

#endif
