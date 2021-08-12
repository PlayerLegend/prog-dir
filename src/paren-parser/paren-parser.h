#ifndef FLAT_INCLUDES
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../immutable/immutable.h"
#endif

typedef struct paren_atom paren_atom;
typedef struct paren_atom_child paren_atom_child;
struct paren_atom_child
{
    bool is_text;
    union {
	paren_atom * atom;
	const char * text;
    };
};

struct paren_atom
{
    paren_atom * peer;
    paren_atom_child child;

    unsigned int line_number, column_number;
    const char * filename;
};

struct paren_parse_arg { const char *text, *filename; immutable_namespace * namespace; };
paren_atom * _paren_parse(struct paren_parse_arg arg);
#define paren_parse(...) _paren_parse((struct paren_parse_arg){__VA_ARGS__})

void paren_atom_free (paren_atom * atom);
