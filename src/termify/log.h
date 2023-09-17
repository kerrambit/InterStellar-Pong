/**
 * @file errors.h
 * @author Marek Eibel
 * @brief Definition of error codes used in the application.
 * 
 * This header file centralizes the definition of error codes used throughout
 * the application. It provides a clear reference for the various
 * error conditions that might occur during the execution of the program. By grouping
 * the error codes in this file, it becomes easier to manage and maintain error handling.
 * 
 * @version 0.1
 * @date 2023-07-18
 */

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdbool.h>

/**
 * @brief Default path to the file with logs (relative to the .app file - executable).
 */
#define LOG_FILE_PATH "logs/logs.log"

/**
 * @brief Definied as a generic type for all inner types of each of the `log_levels_t`. This covers all the possible states inside ERROR, DEBUG, LOG or WARNING.
 */
typedef int log_level_type_t;

/**
 * @brief Undefinied log level type used as the parameter value for log levels without their own subtypes.
 */
#define UNDEFINIED_LOG_LEVEL_TYPE -1

/**
 * @brief All supported types of logging messages in the application.
 */
typedef enum log_levels_t
{
    LOG_LOG,
    LOG_WARNING,
    LOG_DEBUG,
    LOG_ERROR
} log_levels_t;

/**
 * @brief All supported errors in the application.
 */
typedef enum errors_t
{
    NO_ERROR,
    GENERAL_ERROR,
    BROKEN_TERMINAL,
    INACTIVE_TERMINAL,
    UNOPENABLE_FILE,
    GENERAL_IO_ERROR,
    CORRUPTED_WRITE_TO_FILE,
    FAILURE_OF_REMOVING_FILE,
    MEM_ALOC_FAILURE,
    INVALID_DATA_IN_FILE,
    MISSING_DATA_FILE,
    FAILURE_OF_RENAMING_FILE,
    TOO_LONG_INPUT
} errors_t;

/**
 * @brief Sets the log level. It works just like a filter to decide which kind of messages will be sent into the file.
 * 
 * @param log_levels_t Level to use (all levels below in the levels hierarchy will be logged also).
 */
void set_log_level_filter(log_levels_t log_level);

/**
 * @brief The most generic logging function. It prints the definied message determined by `type_of_message_in_the_given_level` according to the level
 * `level` into the file `file`.
 * 
 * @param level Log level to use.
 * @param file Path to the file.
 * @param type_of_message_in_the_given_level This should be always type converted to the real type, e.g. if `level` is ERROR, you should use for example (errors_t)MEM_ALOC_FAILURE. If there is nothing implemented inside the log level, just use UNDEFINIED_LOG_LEVEL_TYPE.
 * @param additional_string Additional string to print (if possible), use NULL if you do not want to use it.
 */
void termify_log(log_levels_t level, const char *file_path, log_level_type_t type_of_message_in_the_given_level, const char *additional_string);

/**
 * @brief Logs the message given by `message` into the file `file_path` as [LOG] log type.
 * 
 * @param file_path File path of the log file.
 * @param message Message to log or NULL.
 */
void log_message(const char *file_path, const char *message);

/**
 * @brief Logs the warning given by `message` into the file `file_path` as [WARNING] log type.
 * 
 * @param file_path File path of the log file.
 * @param message Message to log or NULL.
 */
void log_warning(const char *file_path, const char *message);

/**
 * @brief Logs the error given by `error` subtype into the file `file_path` as [ERROR] log type.
 *  The error can be completed by additional string `additional_string`.
 * 
 * @param file_path File path of the log file.
 * @param error Error subtype.
 * @param message Message to log or NULL.
 */
void log_error(const char *file_path, errors_t error, const char *additional_string);

/**
 * @brief Logs the drebug message given by `message` into the file `file_path` as [DEBUG] log type.
 * 
 * @param file_path File path of the log file.
 * @param message Message to log or NULL.
 */
void log_debug_message(const char *file_path, const char *message);

/**
 * @brief Outputs stack trace node (current node is given by `function` and `line`) of the stack trace tree into the `file`.
 * 
 * @param file_path File path of the log file.
 * @param source_file_name Name of the file with the function.
 * @param function Current function name.
 * @param line Current line in the source file.
 */
void log_stack_trace_node(const char *file_path, const char *source_file_name, const char *function, int line);

/**
 * @brief Resolves error code. Special macro created to automate error logging. TODO: finish the docs for this macro.
 */
#define resolve_error(error, additional_string) set_log_level_filter(LOG_ERROR); log_error(LOG_FILE_PATH, error, additional_string); log_stack_trace_node(LOG_FILE_PATH, __FILE__, __FUNCTION__, __LINE__)

/**
 * @brief Converts log level `log_level` to the string.
 * 
 * @param log_level Log level to convert.
 * @return const char* Converted string.
 */
const char *convert_log_levels_type_2_string(log_levels_t log_level);

#endif