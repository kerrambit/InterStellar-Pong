#include <stdlib.h>

#include "errors.h"
#include "levels.h"
#include "utils.h"

level_row_t create_level_row(int stone_request, int copper_request, int iron_request, int gold_request, int prob_stone, int prob_copper, int prob_iron, int prob_gold)
{
    level_row_t row = { stone_request, copper_request, iron_request, gold_request, prob_stone, prob_copper, prob_iron, prob_gold };
    return row;
}

levels_table_t *create_levels_table()
{
    const int BEGIN_ARRAY_SIZE = 4;

    levels_table_t *table = malloc(sizeof(levels_table_t));
    if (table == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        return NULL;
    }

    table->count = 0;
    table->length = BEGIN_ARRAY_SIZE;
    table->levels = malloc(sizeof(level_row_t) * table->length);

    if (table->levels == NULL) {
        free(table);
        resolve_error(MEM_ALOC_FAILURE);
        return NULL;
    }

    return table;
}

levels_table_t *add_level(levels_table_t *table, level_row_t level)
{
    const int GROWTH_FACTOR = 2;

    if (table == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        return NULL;
    }

    if (table->count >= table->length) {
        table->length *= GROWTH_FACTOR;
        level_row_t *new_levels_table = realloc(table->levels, sizeof(level_row_t) * table->length);
        if (new_levels_table == NULL) {
            resolve_error(MEM_ALOC_FAILURE);
            return NULL;
        }
        table->levels = new_levels_table;
    }

    table->levels[table->count++] = level;
    return table;
}

void release_levels_table(levels_table_t *table)
{
    if (table != NULL) {
        free(table->levels);
        free(table);
    }
}

const char *convert_level_row_2_string(level_row_t level_row)
{
    const char *string = create_string("stone request: %d, copper request: %d, iron request: %d, gold request: %d, stone spawn probability: %d, copper spawn probability: %d, iron spawn probability: %d, gold spawn probability: %d", level_row.stone_request, level_row.copper_request, level_row.iron_request, level_row.gold_request, level_row.prob_stone, level_row.prob_copper, level_row.prob_iron, level_row.prob_gold);
    if (string == NULL) {
        resolve_error(MEM_ALOC_FAILURE);
        return NULL;
    }
    return string;
}
