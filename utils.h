#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <stdbool.h>

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

#endif
