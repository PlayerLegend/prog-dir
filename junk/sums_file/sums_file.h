#ifndef FLAT_INCLUDES
#include "dictionary.h"
#endif

typedef struct
{
    table sums, names;
}
    sums_relation; // dict value for any table key, only one table for everything

typedef dictionary(sums_relation) sums_db;

void sums_db_init(sums_db * init);
void sums_db_add(sums_db * addto, const char * sum, const char * name);
void sums_db_del_line(sums_db * delfrom, const char * sum, const char * name);
void sums_db_del_sum(sums_db * delfrom, const char * sum);
void sums_db_del_name(sums_db * delfrom, const char * name);
int sums_file_load(sums_db * load, FILE * file);
