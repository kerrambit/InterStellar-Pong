/**
 * @file errors.h
 * @author Marek Eibel
 * @brief Groups all errors codes.
 * @version 0.1
 * @date 2023-07-18
 */

#ifndef ERRORS_H
#define ERRORS_H

/**
 * @brief All supported errors in the application.
 */
typedef enum errors_t
{
    NO_ERROR,
    BROKEN_TERMINAL,
    INACTIVE_TERMINAL,
    UNOPENABLE_FILE,
    GENERAL_IO_ERROR,
    CORRUPTED_WRITE_TO_FILE,
    FAILURE_OF_REMOVING_FILE,
    MEM_ALOC_FAILURE,
    INVALID_DATA_IN_FILE,
    MISSING_DATA_FILE,
    FAILURE_OF_RENAMING_FILE
} errors_t;

/**
 * @brief Resolves error code and prints error message to stderr.
 * 
 * @param error error code
 */
void resolve_error(errors_t error);

#endif