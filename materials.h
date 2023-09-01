/**
 * @file materials.h
 * @author Marek Eibel
 * @brief Defines material types and functions for managing materials and their probabilities.
 * @version 0.1
 * @date 2023-08-27
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef MATERIALS_H
#define MATERIALS_H

#include "draw.h"

#define MATERIALS_COUNT 4

/**
 * @enum material_type_h
 * @brief Enumerates different types of materials. Each material is mapped to its colour.
 */
typedef enum material_type_t {
    STONE=DARK_GRAY,    /** Represents a stone material type. */
    COPPER=ORANGE,      /** Represents a copper material type. */
    IRON=LIGHT_RED,     /** Represents a iron material type. */
    GOLD=YELLOW         /** Represents a gold material type. */
} material_type_t;

/**
 * @brief This enumeration defines two possible shapes for a material object:
 */
typedef enum material_shape_t {
    RECTANGLE,     /** Represents a square shape. */
    SQUARE            /** Represents a rectangular shape. */
} material_shape_t;

/**
 * @struct material_row_t
 * @brief Represents a row of material data with associated probabilities.
 */
typedef struct material_row_t {
    material_type_t material_type; ///< Type of the material.
    int prob_size_1_px_t;       /** Probability of size 1 in pixels. */
    int prob_size_2_px_t;       /** Probability of size 2 in pixels. */
    int prob_rectangle_shape;   /** Probability of rectangle shape. */
    int prob_square_shape;      /** Probability of square shape. */
} material_row_t;

/**
 * @struct materials_table_t
 * @brief Represents a table of materials with associated data.
 */
typedef struct materials_table_t {
    material_row_t *materials; /** Pointer to an array of material rows. */
    int count;                 /** Number of materials in the table. */
    int length;                /** Current allocated length of the materials array. */
} materials_table_t;

/**
 * @brief Creates a material row with given probabilities.
 * 
 * @param probability_of_size_1_px_t Probability of size equal to 1 pixel.
 * @param probability_of_size_2_px_t Probability of size equal to 2 pixels.
 * @param probability_of_rectangle_shape Probability of rectangle shape.
 * @param probability_of_square_shape Probability of square shape.
 * @return material_row_t The created material row.
 */
material_row_t create_material_row(material_type_t material, int probability_of_size_1_px_t, int probability_of_size_2_px_t, int probability_of_rectangle_shape, int probability_of_square_shape);

/**
 * @brief Creates an empty materials table.
 * 
 * @return materials_table_t* A pointer to the created materials table.
 */
materials_table_t *create_materials_table();

/**
 * @brief Adds a material row to the materials table.
 * 
 * @param table The materials table to add to.
 * @param material The material row to add.
 * @return materials_table_t* A pointer to the modified materials table or NULL if error occurs.
 */
materials_table_t *add_material(materials_table_t *table, material_row_t material);

/**
 * @brief Releases the memory allocated for a materials table.
 * 
 * @param table The materials table to release.
 */
void release_materials_table(materials_table_t *table);

/**
 * @brief Converts a material row data structure to a string representation.
 * 
 * @param material_row The material row to be converted.
 * @return A dynamically allocated string containing the material row information,
 *         or NULL if memory allocation fails.
 * @warning Returned string must be freed!
 */
const char *convert_material_row_2_string(material_row_t material_row);

/**
 * @brief Converts a material type enumeration value to a string representation.
 * 
 * @param material_type The material type to be converted.
 * @return The string representation of the given material type.
 */
const char *convert_material_type_2_string(material_type_t material_type);

#endif
