#include <stdlib.h>
#include <string.h>

#include "utils.h"

bool convert_string_2_int(const char* str, int* placeholder)
{
    char* endptr;
    long int value = strtol(str, &endptr, 10);

    if (*str != '\0' && *endptr == '\0') {
        *placeholder = (int)value;
        return true;
    } else {
        return false;
    }
}

void strip_newline(char* string)
{
    if (strlen(string) > 0 && string[strlen(string) - 1] == '\n') {
        string[strlen(string) - 1] = '\0';
    }
}