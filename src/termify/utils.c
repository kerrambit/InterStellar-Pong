#include <ctype.h>
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

void complete_strip(char* string)
{
    size_t length = strlen(string);

    while (length > 0 && isspace((unsigned char)string[length - 1])) {
        string[length - 1] = '\0';
        length--;
    }

    size_t start = 0;
    while (isspace((unsigned char)string[start])) {
        start++;
    }

    if (start > 0) {
        memmove(string, string + start, length - start + 1);
    }
}

char *create_string(const char *format, ...)
{
    va_list args, args_copy;
    va_start(args, format);
    va_copy(args_copy, args);
    
    int size = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);

    char *string = (char *)malloc(size + 1);
    if (string == NULL) {
        resolve_error(MEM_ALOC_FAILURE, NULL);
        va_end(args);
        return NULL;
    }

    vsnprintf(string, size + 1, format, args);
    va_end(args);

    return string;
}
