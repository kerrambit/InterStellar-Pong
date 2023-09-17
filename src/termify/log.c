#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#include "log.h"
#include "utils.h"

// ------------------------------------ GLOBAL VARIABLE----------------------------------------- //

/**
 * @brief Variable holds the current log level in use. This variable can be set only by set_log_level().
 */
static log_levels_t gl_curr_log_level = LOG_LOG;

/**
 * @brief Boolean variable holds the state if the logs.log file was opened at least once in the course of program.
 */
static bool gl_has_been_log_file_already_opened = false;

// ---------------------------------- STATIC DECLARATIONS--------------------------------------- //

static bool check_log_level(log_levels_t level);
static const char *create_data_and_time_stamp_string();
static const char *create_user_name_string();
static const char *get_log_levels_string(log_levels_t level);
static const char *get_log_level_type_string(log_levels_t level, log_level_type_t log_level_type);
static const char *get_error_string(errors_t error);
static const char* create_log_line(log_levels_t level, log_level_type_t inner_log_level_type);

// ----------------------------------------- PROGRAM-------------------------------------------- //

void set_log_level_filter(log_levels_t log_level)
{
    gl_curr_log_level = log_level;
}

const char *convert_log_levels_type_2_string(log_levels_t log_level)
{
    switch (log_level)
    {
    case LOG_LOG: return "LOG";
    case LOG_WARNING: return "WARNING";
    case LOG_DEBUG: return "DEBUG";
    case LOG_ERROR: return "ERROR";
    default: "UNKOWN";
    }
}

/**
 * @brief Checks if the filter level `gl_curr_log_level` is set sufficiently high enough
 * to print with the level `level`;
 * 
 * @param level level
 * @return true if the `gl_curr_log_level` is set sufficiently high enough else false
 */
static bool check_log_level(log_levels_t level)
{
    if (gl_curr_log_level > level) {
        return false;
    }

    return true;
}

/**
 * @brief Creates a date and time stamp string in the format "YYYY-MM-DD HH:MM:SS".
 *
 * @return A dynamically allocated string containing the date and time stamp or NULL if memory allocation fails.
 *
 * @warning Make sure to free the memory when done using the returned string to avoid memory leaks.
 */
static const char *create_data_and_time_stamp_string()
{
    const int BUFFER_SIZE = 20;
    time_t current_time;
    struct tm *local_time;

    time(&current_time);
    local_time = localtime(&current_time);

    char *date_and_time_stamp = malloc(BUFFER_SIZE * sizeof(char));

    if (date_and_time_stamp == NULL) {
        resolve_error(MEM_ALOC_FAILURE, NULL);
        return NULL;
    }

    strftime(date_and_time_stamp, BUFFER_SIZE, "%Y-%m-%d %H:%M:%S", local_time);

    return (const char *)date_and_time_stamp;
}

/**
 * @brief Retrieves the username of the currently logged-in user.
 *
 * @return A string containing the username of the currently logged-in user or NULL if the "USER" environment variable is not found or if an error occurs.
 *
 * @warning Ensure that the "USER" environment variable is set correctly on the system.
 */
static const char *create_user_name_string()
{
    char *user_name = getenv("USER");
    if (user_name == NULL) {
        resolve_error(GENERAL_ERROR, "could not get USER variable from the environment variables.");
        return NULL;
    }

    return (const char*)user_name;
}

/**
 * @brief Converts a log level enum to a corresponding string representation.
 *
 * @param level The log level enumeration value.
 *
 * @return A string representing the log level.
 */
static const char *get_log_levels_string(log_levels_t level)
{
    switch (level)
    {
    case LOG_LOG: return "LOG";
    case LOG_WARNING: return "WARNING";
    case LOG_ERROR: return "ERROR";
    case LOG_DEBUG: return "DEBUG";
    default: return "LOG";
    }
}

/**
 * @brief Retrieves the string representation of a log level type.
 *
 * @param level The log level enumeration value.
 * @param log_level_type The log level type enumeration value.
 *
 * @return A string representing the log level type.
 */
static const char *get_log_level_type_string(log_levels_t level, log_level_type_t log_level_type)
{
    const char *NO_LOG_LEVEL_TYPE = ": ";

    if (log_level_type == UNDEFINIED_LOG_LEVEL_TYPE) {
        return NO_LOG_LEVEL_TYPE;
    }

    switch (level)
    {
    case LOG_LOG:
        return NO_LOG_LEVEL_TYPE;
    case LOG_WARNING:
        return NO_LOG_LEVEL_TYPE;
    case LOG_ERROR:
        return get_error_string((errors_t)log_level_type);
    case LOG_DEBUG:
        return NO_LOG_LEVEL_TYPE;
    }
}

/**
 * @brief Retrieves the error message associated with an error code.
 * 
 * @param error The error code enumeration value.
 *
 * @return A string representing the error message.
 */
const char *get_error_string(errors_t error)
{
    switch (error)
    {
    case GENERAL_ERROR: return " - [Error]: ";
    case BROKEN_TERMINAL: return " - [I/O Error]: unable to render terminal.";
    case INACTIVE_TERMINAL: return " - [Application Error]: terminal is turned off.";
    case UNOPENABLE_FILE: return " - [I/O Error]: opening of the file failed. The file name: ";
    case GENERAL_IO_ERROR: return " - [I/O Error]: ";
    case CORRUPTED_WRITE_TO_FILE: return " - [I/O Error]: writing to the file failed.";
    case FAILURE_OF_REMOVING_FILE: return " - [I/O Error]: removing of the file failed. The file name: ";
    case MEM_ALOC_FAILURE: return " - [Memory error]: memory allocation failed.";
    case INVALID_DATA_IN_FILE: return " - [Data Error]: data in file was invalid or corrupted. The file name: ";
    case MISSING_DATA_FILE: return " - [Application Error]: the file with data was not found. The file name: ";
    case FAILURE_OF_RENAMING_FILE: return " - [I/O Error]: renaming of the file failed. The file name: ";
    default: return " - [Unknown Error]: ";
    }
}

void termify_log(log_levels_t level, const char *file_path, log_level_type_t type_of_message_in_the_given_level, const char *additional_string)
{
    if (!check_log_level(level)) {
        return;
    }

    if (type_of_message_in_the_given_level == UNDEFINIED_LOG_LEVEL_TYPE && additional_string == NULL) {
        return;
    }

    if (!gl_has_been_log_file_already_opened && access(file_path, F_OK) == 0) {
        if (remove(file_path) != 0) {
            fprintf(stderr, "Critical error occured: unable to remove file \"%s\" before logging of the error message!\n", file_path);
            return;
        }
    }

    gl_has_been_log_file_already_opened = true;

    FILE *file = fopen(file_path, "a");
    if (file == NULL) {
        fprintf(stderr, "Critical error occured: unable to write into the \"%s\" file while logging the error message!\n", file_path);
        return;
    }

    const char *UNKNOWN_DATE_AND_TIME = "???\?-?\?-?? ??:??:??";

    const char *date_and_time_stamp = create_data_and_time_stamp_string();
    if (date_and_time_stamp == NULL) {
        date_and_time_stamp = UNKNOWN_DATE_AND_TIME;
    }

    const char *user_name = create_user_name_string();
    if (user_name == NULL) {
        user_name = "unknown_user";
    }

    const char *log_levels_string = get_log_levels_string(level);
    const char *log_level_type_string = get_log_level_type_string(level, type_of_message_in_the_given_level);
    const char *message = (additional_string == NULL) ? "" : additional_string;

    fprintf(file, "%s (%s)\t\t[%s]%s%s\n", date_and_time_stamp, user_name, log_levels_string, log_level_type_string, message);
    fclose(file);
    
    if (!STR_EQ(date_and_time_stamp, UNKNOWN_DATE_AND_TIME)) {
        free((char*)date_and_time_stamp);
    }
}

void log_message(const char *file_path, const char *message)
{
    termify_log(LOG_LOG, file_path, UNDEFINIED_LOG_LEVEL_TYPE, message);
}

void log_warning(const char *file_path, const char *message)
{
    termify_log(LOG_WARNING, file_path, UNDEFINIED_LOG_LEVEL_TYPE, message);
}

void log_debug_message(const char *file_path, const char *message)
{
    termify_log(LOG_DEBUG, file_path, UNDEFINIED_LOG_LEVEL_TYPE, message);
}

void log_error(const char *file_path, errors_t error, const char *additional_string)
{
    termify_log(LOG_ERROR, file_path, error, additional_string);
}

void log_stack_trace_node(const char *file_path, const char *source_file_name, const char *function, int line)
{
    if (!gl_has_been_log_file_already_opened && access(file_path, F_OK) == 0) {
        if (remove(file_path) != 0) {
            fprintf(stderr, "Critical error occured: unable to remove file \"%s\" before logging of the error message!\n", file_path);
            return;
        }
    }

    gl_has_been_log_file_already_opened = true;

    FILE *file = fopen(file_path, "a");
    if (file == NULL) {
        fprintf(stderr, "Critical error occured: unable to write into the \"%s\" file while logging the error message!\n", file_path);
        return;
    }

    fprintf(file, "\t├── %s:%d in function \'%s\'\n", source_file_name, line, function);   
    fclose(file);
}
