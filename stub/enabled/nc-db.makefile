PROGRAM_NAME = bin/nc-db
PROGRAM_OBJ = nc-db/main nc-db/database nc-db/configuration nc-db/networking stack index_map hash_table hash_table_string print print_array tcp_event network options delimit sums_file

LDLIBS += -lev
