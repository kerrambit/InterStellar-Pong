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
 * @brief All possible and supported errors in the program.
 */
typedef enum errors_t
{
    NO_ERROR,
    BROKEN_TERMINAL,
    UNOPENABLE_FILE,
    GENERAL_IO_ERROR,
    CORRUPTED_WRITE_TO_FILE,
    FAILURE_OF_REMOVING_FILE
} errors_t;

/**
 * @brief Resolves error code and prints error message to the stderr.
 * 
 * @param error error code
 */
void resolve_error(errors_t error);

#endif