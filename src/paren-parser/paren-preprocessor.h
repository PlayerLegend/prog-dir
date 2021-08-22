#ifndef FLAT_INCLUDES
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../immutable/immutable.h"
#include "../keyargs/keyargs.h"
#include "paren-parser.h"
#endif

keyargs_declare_clone(paren_preprocessor, paren_parse);
#define paren_preprocessor(...) keyargs_call(paren_preprocessor, __VA_ARGS__)
