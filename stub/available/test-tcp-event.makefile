PROGRAM_NAME = tests/test-tcp-event
PROGRAM_OBJ = tests/test-tcp-event network tcp_event print stack

LDLIBS += -lpthread -lev
