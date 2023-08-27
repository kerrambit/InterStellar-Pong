#ifndef UTILS_H
#define UTILS_H

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "errors.h"

#define STR_EQ(_str1, _str2) (strcmp(_str1, _str2) == 0) ? true : false
#define SQUARE(_num) (_num * _num)
#define KEYBOARD_PRESSED(_user_keyboard, _target_keyboard) (_user_keyboard == _target_keyboard)

/**
 * @brief Converts a string to an integer.
 *
 * This function attempts to convert a given string to an integer value. If the conversion is successful
 * and the entire string is converted, the integer value is stored in the provided placeholder,
 * and the function returns true. Otherwise, it returns false.
 *
 * @param str The input string to be converted.
 * @param placeholder A pointer to an integer to store the converted value.
 * @return Returns true if the conversion is successful, or false otherwise.
 */
bool convert_string_2_int(const char* str, int* placeholder);

/**
 * @brief Removes the newline character from the end of a string.
 *
 * This function checks if the last character of the provided string is a newline character.
 * If it is, the newline character is replaced with a null terminator, effectively removing
 * it from the string.
 *
 * @param string The input string to be modified.
 */
void strip_newline(char* string);

/**
 * @brief Removes all leading and trailing whitespace characters from a string.
 * 
 * This function removes whitespace characters (such as space, tab, newline, etc.)
 * from the beginning and end of the input string. The modified string will be
 * stored in the same memory location as the input string.
 * 
 * @param string The input string to be modified.
 */
void complete_strip(char* string);

/**
 * Creates a formatted string using a variable argument list.
 * 
 * @param format The format string.
 * @param ... Variable arguments to format.
 * @return The formatted string, or NULL in case of memory allocation failure.
 */
char *create_string(const char *format, ...);

#endif
