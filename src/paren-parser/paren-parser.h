#ifndef FLAT_INCLUDES
#include <stdbool.h>
#include <stdio.h>
#define FLAT_INCLUDES
#include "../immutable/immutable.h"
#include "../keyargs/keyargs.h"
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

keyargs_declare(paren_atom*,
		paren_parse,
		const char * text;
		const char * filename;
		immutable_namespace * namespace;);

#define paren_parse(...) keyargs_call(paren_parse, __VA_ARGS__);

void paren_atom_free (paren_atom * atom);

#define paren_warning(atom, fmt, ...) fprintf(stderr, "%s:%d:%d: warning: " fmt "\n", (atom)->filename, (atom)->line_number, (atom)->column_number, ##__VA_ARGS__);
#define paren_fatal(atom, fmt, ...) { fprintf(stderr, "%s:%d:%d: fatal: " fmt "\n", (atom)->filename, (atom)->line_number, (atom)->column_number, ##__VA_ARGS__); goto fail; }

