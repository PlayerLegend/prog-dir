PROGRAM_NAME = bin/nc-db
PROGRAM_OBJ = nc-db/main nc-db/database nc-db/configuration nc-db/networking nc-db/index_string stack index_map hash_table hash_table_string print print_array tcp_event network options delimit

LDLIBS += -lev
