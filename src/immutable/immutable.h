#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#endif

typedef struct immutable_namespace immutable_namespace;

const char * immutable_string (immutable_namespace * namespace, const char * input);
const char * immutable_path (immutable_namespace * namespace, const char * path);
