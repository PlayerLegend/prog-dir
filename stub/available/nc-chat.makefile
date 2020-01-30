PROGRAM_NAME = bin/nc-chat
PROGRAM_OBJ = programs/nc-chat network tcp_event hash_table hash_table_string index_map stack print print_array

LDLIBS += -lpthread -lev
