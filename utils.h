#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <stdbool.h>

#define STR_EQ(_str1, _str2) (strcmp(_str1, _str2) == 0) ? true : false
#define SQUARE(num) (num * num)
#define KEYBOARD_PRESSED(user_keyboard, target_keyboard) (user_keyboard == target_keyboard)

bool convert_string_2_int(const char* str, int* placeholder);
void strip_newline(char* string);

#endif
