#include <stdlib.h>

#include "../termify/log.h"
#include "materials.h"
#include "../termify/utils.h"

material_row_t create_material_row(material_type_t material, int probability_of_size_1_px_t, int probability_of_size_2_px_t, int probability_of_rectangle_shape, int probability_of_square_shape)
{
    material_row_t row = { material, probability_of_size_1_px_t, probability_of_size_2_px_t, probability_of_rectangle_shape, probability_of_square_shape };
    return row;
}

materials_table_t *create_materials_table()
{
    const int BEGIN_ARRAY_SIZE = 4;

    materials_table_t *table = malloc(sizeof(materials_table_t));
    if (table == NULL) {
        resolve_error(MEM_ALOC_FAILURE, NULL);
        return NULL;
    }

    table->count = 0;
    table->length = BEGIN_ARRAY_SIZE;
    table->materials = malloc(sizeof(material_row_t) * table->length);

    if (table->materials == NULL) {
        free(table);
        resolve_error(MEM_ALOC_FAILURE, NULL);
        return NULL;
    }

    return table;
}

materials_table_t *add_material(materials_table_t *table, material_row_t material)
{
    const int GROWTH_FACTOR = 2;

    if (table == NULL) {
        resolve_error(MEM_ALOC_FAILURE, NULL);
        return NULL;
    }

    if (table->count >= table->length) {
        table->length *= GROWTH_FACTOR;
        material_row_t *new_materials_table = realloc(table->materials, sizeof(material_row_t) * table->length);
        if (new_materials_table == NULL) {
            resolve_error(MEM_ALOC_FAILURE, NULL);
            return NULL;
        }
        table->materials = new_materials_table;
    }

    table->materials[table->count++] = material;
    return table;
}

void release_materials_table(materials_table_t *table)
{
    if (table != NULL) {
        free(table->materials);
        free(table);
    }
}

const char *convert_material_row_2_string(material_row_t material_row)
{
    const char *string = create_string("Material type: %s, size 1_px_t probability: %d, size 2_px_t probability: %d, rectangle shape probability: %d, square shape probability: %d", convert_material_type_2_string(material_row.material_type), material_row.prob_size_1_px_t, material_row.prob_size_2_px_t, material_row.prob_rectangle_shape, material_row.prob_square_shape);
    if (string == NULL) {
        resolve_error(MEM_ALOC_FAILURE, NULL);
        return NULL;
    }
    return string;
}

const char *convert_material_type_2_string(material_type_t material_type)
{
    switch (material_type)
    {
    case STONE: return "stone";
    case COPPER: return "copper";
    case IRON: return "iron";
    case GOLD: return "gold";
    }
}