#ifndef FLAT_INCLUDES
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../immutable/immutable.h"
#include "paren-parser.h"
#endif

paren_atom * _paren_preprocessor (struct paren_parse_arg arg);
#define paren_preprocessor(...) _paren_preprocessor((struct paren_parse_arg) {__VA_ARGS__})

