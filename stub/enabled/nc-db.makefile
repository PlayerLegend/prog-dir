PROGRAM_NAME = bin/nc-db
PROGRAM_OBJ = programs/nc-db/main programs/nc-db/database programs/nc-db/configuration programs/nc-db/networking programs/nc-db/index_string stack index_map hash_table hash_table_string print print_array tcp_event network options delimit

LDLIBS += -lev
