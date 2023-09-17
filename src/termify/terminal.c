#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "terminal.h"
#include "utils.h"

// ---------------------------------------- MACROS --------------------------------------------- //

#define TERMINAL_FILE_PATH "src/termify/temp/user_input.data"
#define TERMINAL_LINE_LENGTH_HARD_LIMIT 512
#define BACKSPACE 127
#define ESCPAPE 27
#define NEWLINE '\n'

// ---------------------------------- STATIC DECLARATIONS--------------------------------------- //

static bool add_to_terminal_file_cursor_storage(terminal_file_cursor_storage_t *terminal_file_cursor_storage, terminal_cursor_duo_t duo);
static terminal_cursor_duo_t create_terminal_cursor_duo(int cursor_offset, int line_length);
static char* get_last_line(char *filename, terminal_data_t *terminal_data, int buffer_size);
static void release_terminal_file_cursor_storage(terminal_file_cursor_storage_t *storage);
static int write_character_into_file(char c, FILE *file, unsigned long line_length, const char *file_path);
static terminal_file_cursor_storage_t *create_terminal_file_cursor_storage(void);
static char* get_line(const char *filename, int buffer_size, int file_offset);
static int clear_terminal_window(terminal_data_t *terminal_data, FILE *file);
static int get_maximal_terminal_buffer_size(unsigned long current_line_size);
static void restore_terminal_attributes(struct termios *original_termios);
static int parse_newline(terminal_data_t *terminal_data, char **command);
static int parse_backspace(terminal_data_t *terminal_data, FILE *file);
static int handle_arrow_up_and_down(terminal_data_t *terminal_data);
static struct termios init_termios();
static bool check_character(char c);

// ----------------------------------------- PROGRAM-------------------------------------------- //

terminal_data_t *enable_terminal(char *default_mess, terminal_output_mode_t default_mess_mode, char *special_flag_default_mess, terminal_output_mode_t special_flag_default_mess_mode)
{
    if (access(TERMINAL_FILE_PATH, F_OK) == 0) {
        if (remove(TERMINAL_FILE_PATH) != 0) {
            resolve_error(FAILURE_OF_REMOVING_FILE, TERMINAL_FILE_PATH);
            return NULL;
        }
    }

    FILE* file = fopen(TERMINAL_FILE_PATH, "w");
    if (file == NULL) {
        resolve_error(UNOPENABLE_FILE, TERMINAL_FILE_PATH);
        return NULL;
    }

    fclose(file);

    terminal_data_t *data = malloc(sizeof(terminal_data_t));
    if (data == NULL) {
        resolve_error(MEM_ALOC_FAILURE, NULL);
        return NULL;
    }

    data->is_terminal_enabled = true;
    data->terminal_default_mess = strdup(default_mess);
    if (data->terminal_default_mess == NULL) {
        resolve_error(MEM_ALOC_FAILURE, NULL);
        free(data);
        return NULL;
    }
    data->terminal_default_mess_mode = default_mess_mode;
    data->terminal_special_flag_default_mess = strdup(special_flag_default_mess);
    if (data->terminal_special_flag_default_mess == NULL) {
        resolve_error(MEM_ALOC_FAILURE, NULL);
        free(data->terminal_default_mess);
        free(data);
        return NULL;
    }
    data->terminal_spacial_flag_default_mess_mode = special_flag_default_mess_mode;
    data->curr_file_cursor = 0; data->curr_file_line_size = 0;

    data->cursors_storage = create_terminal_file_cursor_storage();
    if (data->cursors_storage == NULL) {
        free(data->terminal_default_mess);
        free(data);
        return NULL;
    }

    data->curr_line = 0;
    data->lines_count_in_file = 0;

    data->old_term = init_termios();
    return data;
}

int close_terminal(terminal_data_t *terminal_data)
{
    if (remove(TERMINAL_FILE_PATH) != 0) {
        resolve_error(FAILURE_OF_REMOVING_FILE, TERMINAL_FILE_PATH);
        return -1;
    }

    restore_terminal_attributes(&terminal_data->old_term);

    if (terminal_data != NULL) {
        free(terminal_data->terminal_default_mess);
        free(terminal_data->terminal_special_flag_default_mess);
        release_terminal_file_cursor_storage(terminal_data->cursors_storage);
        free(terminal_data);
    }

    return 0;
}

int process_command(terminal_data_t *terminal_data, char c, char **command)
{
    if (!check_character(c)) {
        return 0;
    }

    if (c == ESCPAPE) {
        return handle_arrow_up_and_down(terminal_data);
    }

    if (!terminal_data->is_terminal_enabled) {
        resolve_error(INACTIVE_TERMINAL, NULL);
        return -1;
    }

    FILE* file = fopen(TERMINAL_FILE_PATH, "a");
    if (file == NULL) {
        resolve_error(UNOPENABLE_FILE, TERMINAL_FILE_PATH);
        return -1;
    }

    if (KEYBOARD_PRESSED(c, BACKSPACE)) {
        int parse_backspace_return_code = parse_backspace(terminal_data, file);
        fclose(file);
        return parse_backspace_return_code;
    }

    if (write_character_into_file(c, file, terminal_data->curr_file_line_size, TERMINAL_FILE_PATH) == -1) {
        fclose(file);
        return -1;
    }

    terminal_data->curr_file_cursor++; terminal_data->curr_file_line_size++;

    if (KEYBOARD_PRESSED(c, NEWLINE)) {
        int parse_newline_return_code = parse_newline(terminal_data, command);
        fclose(file);
        return parse_newline_return_code;
    }

    fclose(file);
    return 0;
}

/**
 * @brief Initializes the terminal settings for interactive input.
 *
 * This function sets up the terminal settings for interactive input, disabling
 * canonical mode and echoing. It returns the original terminal settings which
 * can be restored later.
 *
 * @return The original struct termios settings before modification.
 */
static struct termios init_termios()
{
    struct termios old_term, new_term;
    tcgetattr(STDIN_FILENO, &old_term);

    new_term = old_term;
    new_term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);

    return old_term;
}

/**
 * @brief Restores terminal attributes for a file descriptor.
 *
 * This function is used to restore terminal attributes for a specified
 * file descriptor. It takes a pointer to the termios structure containing
 * the original attributes and applies them to the terminal using the
 * tcsetattr() command with the TCSANOW flag.
 *
 * @param original_termios A pointer to the termios structure containing the original attributes.
 */
static void restore_terminal_attributes(struct termios *original_termios)
{
    tcsetattr(STDIN_FILENO, TCSANOW, original_termios);
}

/**
 * @brief Writes a character into a file.
 *
 * This function writes a character into a specified file and performs error checks during the write operation.
 *
 * @param c The character to be written to the file.
 * @param file A pointer to the file where the character will be written.
 * @param line_length The current line length in the file (excluding the character to be written).
 *
 * @return 0 if the character is successfully written to the file.
 * @return -1 if an error occurs during the write operation.
 */
static int write_character_into_file(char c, FILE *file, unsigned long line_length, const char *file_path)
{
    if (line_length + 1 >= TERMINAL_LINE_LENGTH_HARD_LIMIT) {
        resolve_error(TOO_LONG_INPUT, NULL);
        return -1;
    }

    if (fputc(c, file) == EOF) {
        resolve_error(CORRUPTED_WRITE_TO_FILE, file_path);
        return -1;
    }

    return 0;
}

int render_terminal(terminal_data_t *terminal_data, px_t line_width, bool special_flag, char *volunatary_mess, terminal_output_mode_t mode)
{
    if (!terminal_data->is_terminal_enabled) {
        resolve_error(INACTIVE_TERMINAL, NULL);
        return -1;
    }

    FILE* file = fopen(TERMINAL_FILE_PATH, "rb");
    if (file == NULL) {
        resolve_error(UNOPENABLE_FILE, TERMINAL_FILE_PATH);
        return -1;
    }

    char *line = NULL;
    char *string_to_print = NULL;
    terminal_output_mode_t mode_to_print_with;
    if (special_flag) {
        if (volunatary_mess == NULL) {
            string_to_print = terminal_data->terminal_special_flag_default_mess;
            mode_to_print_with = terminal_data->terminal_spacial_flag_default_mess_mode;
        } else {
            string_to_print = volunatary_mess;
            mode_to_print_with = mode;
        }
    } else {
        int buffer_size = get_maximal_terminal_buffer_size(terminal_data->curr_file_line_size);
        line = get_last_line(TERMINAL_FILE_PATH, terminal_data, buffer_size);
        if (strlen(line) != 0) {
            string_to_print = line;
            mode_to_print_with = TERMINAL_NORMAL_TEXT;
        } else {
            string_to_print = terminal_data->terminal_default_mess;
            mode_to_print_with = terminal_data->terminal_default_mess_mode;
        }
    } 
    
    put_horizontal_line(line_width - 1, '=');
    write_text("|| > ");

    switch (mode_to_print_with)
    {
    case TERMINAL_NORMAL_TEXT:
        printf("%s", string_to_print); break;
    case TERMINAL_LOG:
        printf("\033[3m\033[90m%s\033[0m", string_to_print); break;
    case TERMINAL_APPROVAL:
        printf("\033[0;32m%s\033[0m", string_to_print); break;
    case TERMINAL_WARNING:
        printf("\033[0;33m%s\033[0m", string_to_print); break;
    case TERMINAL_ERROR:
        printf("\033[0;31m%s\033[0m", string_to_print); break;
    case TERMINAL_N_A:
        printf("%s", string_to_print); break;
    default:
        break;
    }

    put_text("||", line_width - strlen(string_to_print) - 6, RIGHT);
    put_horizontal_line(line_width - 1, '=');

    free(line);
    fclose(file);
    return 0;
}

/**
 * @brief Clears the virtual terminal window by removing text from the terminal window.
 *
 * @param terminal_data A pointer to the terminal_data_t structure.
 * @param file A pointer to the FILE stream used for terminal operations.
 *
 * @return 0 on success, -1 on failure
 */
static int clear_terminal_window(terminal_data_t *terminal_data, FILE *file)
{
    unsigned long counter = terminal_data->curr_file_line_size;
    for (int i = 0; i < counter; ++i) {
        if (parse_backspace(terminal_data, file) == -1) {
            fclose(file);
            return -1;
        }
    }
    return 0;
}

/**
 * @brief Handles Arrow Up and Arrow Down keypresses.
 *
 * This function handles Arrow Up and Arrow Down keypresses in the terminal.
 * The effect of pressing these buttons is definied as browsing the commands history.
 *
 * @param terminal_data A pointer to the terminal_data_t structure.
 *
 * @return 0 on success, -1 on failure
 */
static int handle_arrow_up_and_down(terminal_data_t *terminal_data)
{
    int c1 = getchar();
    int c2 = getchar();

    if (c1 == '[' && c2 == 'A') {
        terminal_data->curr_line--;
    } else if (c1 == '[' && c2 == 'B') {
        terminal_data->curr_line++;
    } else {
        return 0;
    }

    FILE* file = fopen(TERMINAL_FILE_PATH, "a");
    if (file == NULL) {
        resolve_error(UNOPENABLE_FILE, TERMINAL_FILE_PATH);
        return -1;
    }

    if (terminal_data->curr_line < 0) {
        terminal_data->curr_line++;
        fclose(file);
        return 0;
    }

    if (terminal_data->curr_line > terminal_data->lines_count_in_file) {
        terminal_data->curr_line--;
        fclose(file);
        return 0;
    }

    if (clear_terminal_window(terminal_data, file) == -1) {
        fclose(file);
        return -1;
    }
    
    if (terminal_data->curr_line == terminal_data->lines_count_in_file) {
        fclose(file);
        return 0;
    }

    char *string = get_line(TERMINAL_FILE_PATH, terminal_data->cursors_storage->storage[terminal_data->curr_line].line_length, terminal_data->cursors_storage->storage[terminal_data->curr_line].cursor_offset);
    strip_newline(string);
    terminal_data->curr_file_line_size += strlen(string);
    terminal_data->curr_file_cursor += strlen(string);

    fprintf(file, "%s", string);
    free(string);
    fclose(file);

    return 0;
}

/**
 * @brief Creates a new `terminal_file_cursor_storage_t` instance.
 *
 * This function initializes a dynamic array for storing terminal cursor information,
 * such as cursor offsets and line lengths.
 *
 * @return A pointer to the newly created `terminal_file_cursor_storage_t` instance,
 *         or NULL if memory allocation fails.
 */
static terminal_file_cursor_storage_t *create_terminal_file_cursor_storage()
{
    const int BEGIN_ARRAY_SIZE = 4;

    terminal_file_cursor_storage_t *storage = malloc(sizeof(terminal_file_cursor_storage_t));
    if (storage == NULL) {
        resolve_error(MEM_ALOC_FAILURE, NULL);
        return NULL;
    }

    storage->count = 0;
    storage->length = BEGIN_ARRAY_SIZE;
    storage->storage = malloc(sizeof(terminal_cursor_duo_t) * storage->length);

    if (storage->storage == NULL) {
        free(storage);
        return NULL;
    }

    return storage;
}

/**
 * @brief Creates a new terminal_cursor_duo_t instance with the specified cursor offset and line length.
 *
 * @param cursor_offset The offset of the cursor.
 * @param line_length   The length of the line.
 * @return A terminal_cursor_duo_t instance with the given values.
 */
static terminal_cursor_duo_t create_terminal_cursor_duo(int cursor_offset, int line_length)
{
    terminal_cursor_duo_t duo;
    duo.cursor_offset = cursor_offset; duo.line_length = line_length;
    return duo;
}

/**
 * @brief Adds a terminal_cursor_duo_t instance to the terminal_file_cursor_storage_t.
 *
 * @param terminal_file_cursor_storage The terminal_file_cursor_storage_t to which the duo should be added.
 * @param duo The terminal_cursor_duo_t instance to add.
 * @return true if the addition was successful, false otherwise.
 */
static bool add_to_terminal_file_cursor_storage(terminal_file_cursor_storage_t *terminal_file_cursor_storage, terminal_cursor_duo_t duo)
{
    const int GROWTH_FACTOR = 2;

    if (terminal_file_cursor_storage == NULL) {
        return false;
    }

    if (terminal_file_cursor_storage->count >= terminal_file_cursor_storage->length) {
        terminal_file_cursor_storage->length *= GROWTH_FACTOR;
        terminal_cursor_duo_t *new_terminal_file_cursor_storage = realloc(terminal_file_cursor_storage->storage, sizeof(terminal_cursor_duo_t) * terminal_file_cursor_storage->length);
        if (new_terminal_file_cursor_storage == NULL) {
            return false;
        }
        terminal_file_cursor_storage->storage = new_terminal_file_cursor_storage;
    }

    terminal_file_cursor_storage->storage[terminal_file_cursor_storage->count++] = duo;
    return true;
}

/**
 * @brief Releases the memory allocated for a `terminal_file_cursor_storage_t` instance.
 *
 * This function deallocates the memory used by the provided terminal cursor storage,
 * including the array of cursor duos and the storage structure itself.
 *
 * @param storage A pointer to the `terminal_file_cursor_storage_t` instance to release.
 */
static void release_terminal_file_cursor_storage(terminal_file_cursor_storage_t *storage)
{
    if (storage != NULL) {
        free(storage->storage);
        free(storage);
    }
}

/**
 * @brief Checks if a character falls within a valid range for input handling.
 *
 * Valid characters include normal printable characters, newline, and
 * backspace. It also handles the case of arrow keys by skipping their sequence if detected.
 * 
 * @param c The character to be checked.
 * @return Returns true if the character is within the valid range, otherwise false.
 */
static bool check_character(char c)
{
    if (c == BACKSPACE || c == NEWLINE || (c >= 32 && c <= 126) || c == ESCPAPE) {
        return true;
    }

    return false;
}

/**
 * @brief Parses a newline character and retrieves the current command.
 *
 * The `parse_newline` function processes a newline character in the terminal file and retrieves the
 * current command from the file. It allocates memory for the command and sets the `command` parameter.
 *
 * @param terminal_data A pointer to the terminal data structure.
 * @param command A pointer to the variable that will hold the retrieved command.
 * @return 0 on success, -1 on failure.
 */
static int parse_newline(terminal_data_t *terminal_data, char **command)
{
    int buffer_size = get_maximal_terminal_buffer_size(terminal_data->curr_file_line_size);
    char *line = get_last_line(TERMINAL_FILE_PATH, terminal_data, buffer_size);

    terminal_cursor_duo_t current_line_duo = create_terminal_cursor_duo(terminal_data->curr_file_cursor - strlen(line) - 1, strlen(line));
    if(!add_to_terminal_file_cursor_storage(terminal_data->cursors_storage, current_line_duo)) {
        free(line);
        return -1;
    }

    *command = malloc(strlen(line) + 1);
    if (*command == NULL) {
        resolve_error(MEM_ALOC_FAILURE, NULL);
        free(line);
        return -1;
    }

    strcpy(*command, line);
    terminal_data->curr_file_line_size = 0;
    terminal_data->lines_count_in_file++;
    terminal_data->curr_line = terminal_data->lines_count_in_file;
    free(line);

    return 0;
}

/**
 * @brief Parses a backspace character and updates the terminal file.
 *
 * The `parse_backspace` function processes a backspace character in the terminal file and updates the
 * file contents to remove the last character. It also adjusts the cursor position and line size.
 *
 * @param terminal_data A pointer to the terminal data structure.
 * @param file A pointer to the terminal file.
 * @return 0 on success, -1 on failure.
 */
static int parse_backspace(terminal_data_t *terminal_data, FILE *file)
{
    if (terminal_data->curr_file_cursor > 0 && terminal_data->curr_file_line_size > 0) {

        if (fseek(file, -1, SEEK_END) != 0) {
            resolve_error(GENERAL_IO_ERROR, "invalid operation with function \'fseek()\'.");
            return -1;
        }

        if (ftruncate(fileno(file), ftell(file)) != 0) {
            resolve_error(GENERAL_IO_ERROR, "invalid operation with function \'ftruncate()\'.");
            return -1;
        }

        terminal_data->curr_file_cursor--; terminal_data->curr_file_line_size--;
    }

    return 0;
}

/**
 * @brief Gets the maximal buffer size for terminal output.
 *
 * The `get_maximal_terminal_buffer_size` function calculates and returns the maximal buffer size
 * for terminal output, considering the current line size.
 *
 * @param terminal_data A pointer to the terminal data structure.
 * @return The maximal buffer size.
 */
static int get_maximal_terminal_buffer_size(unsigned long current_line_size)
{
    const int MAX_BUFFER_SIZE = 102;

    int buffer_size = current_line_size;
    if (buffer_size > MAX_BUFFER_SIZE || buffer_size == 0) {
        buffer_size = MAX_BUFFER_SIZE;
    }

    return buffer_size;
}

/**
 * @brief Gets the line of the terminal file.
 *
 * The `get_line` function retrieves and returns the line of the terminal file given by its offset.
 *
 * @param terminal_data A pointer to the terminal data structure.
 * @param buffer_size The buffer size for reading the line.
 * @param file_offset The offset in the file.
 * @return A dynamically allocated string containing the line.
 */
static char* get_line(const char *filename, int buffer_size, int file_offset)
{
    char *line = calloc(buffer_size + 1, sizeof(char));
    if (line == NULL) {
        resolve_error(MEM_ALOC_FAILURE, NULL);
        return NULL;
    }

    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        resolve_error(UNOPENABLE_FILE, filename);
        free(line);
        return NULL;
    }

    if (fseek(file, file_offset, SEEK_SET) != 0) {
        resolve_error(GENERAL_IO_ERROR, "invalid operation with function \'fseek()\'.");
        free(line);
        fclose(file);
        return NULL;
    }

    fread(line, sizeof(char), buffer_size, file);
    line[buffer_size] = '\0'; 

    fclose(file);
    return line;
}

/**
 * @brief Gets the last line of the terminal file.
 *
 * The `get_last_line` function retrieves and returns the last line of the terminal file, considering the
 * current cursor position and buffer size.
 *
 * @param terminal_data A pointer to the terminal data structure.
 * @param buffer_size The buffer size for reading the last line.
 * @return A dynamically allocated string containing the last line.
 */
static char* get_last_line(char *filename, terminal_data_t *terminal_data, int buffer_size)
{
    int offset = terminal_data->curr_file_cursor - terminal_data->curr_file_line_size;
    return get_line(filename, buffer_size, offset);
}
