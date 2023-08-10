#include <stdio.h>

#include "errors.h"

// ---------------------------------------- MACROS --------------------------------------------- //

#define PRINT_ERROR_MESS(err_mess) fprintf(stderr, "\033[31m[ERROR] - %s.\033[0m\n", err_mess); break
#define PRINT_WARNING_MESS(err_mess) fprintf(stderr, "\033[33m[WARNING]  %s.\033[0m\n", err_mess); break

// ----------------------------------------- PROGRAM-------------------------------------------- //

void resolve_error(errors_t error)
{
    switch (error)
    {
    case BROKEN_TERMINAL: PRINT_ERROR_MESS("[I/O Error]: unable to render terminal");
    case INACTIVE_TERMINAL: PRINT_ERROR_MESS("[Application Error]: terminal is turned off");
    case UNOPENABLE_FILE: PRINT_ERROR_MESS("[I/O Error]: opening of the file failed");
    case GENERAL_IO_ERROR: PRINT_ERROR_MESS("[I/O Error]");
    case CORRUPTED_WRITE_TO_FILE: PRINT_ERROR_MESS("[I/O Error]: writing to the file failed");
    case FAILURE_OF_REMOVING_FILE: PRINT_ERROR_MESS("[I/O Error]: removing of the file failed");
    case MEM_ALOC_FAILURE: PRINT_ERROR_MESS("[Memory error]: memory allocation failed");
    case INVALID_DATA_IN_FILE: PRINT_ERROR_MESS("[Data Error]: data in file was invalid or corrupted");
    case MISSING_DATA_FILE: PRINT_ERROR_MESS("[Application Error]: file with data was not found");
    case FAILURE_OF_RENAMING_FILE: PRINT_ERROR_MESS("[I/O Error]: renaming of the file failed");
    default:
        break;
    }
}
