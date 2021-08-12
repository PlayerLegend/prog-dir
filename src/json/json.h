#ifndef FLAT_INCLUDES
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#endif

typedef struct json_value json_value;
typedef struct json_object json_object;
typedef struct range(json_value) json_array;
typedef enum json_type json_type;

enum json_type {
    JSON_NULL,
    JSON_NUMBER,
    JSON_TRUE,
    JSON_FALSE,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT,
    JSON_BADTYPE,
};

struct json_value {
    json_type type;
    union {
	double number;
	char * string;
	json_array array;
	json_object * object;
    };
};

typedef struct json_get_number_arg json_get_number_arg;
struct json_get_number_arg {
    const json_object * parent;
    const char * key;
    bool * success;
    bool optional;
    double default_value;
};

typedef struct json_get_string_arg json_get_string_arg;
struct json_get_string_arg {
    const json_object * parent;
    const char * key;
    bool * success;
    bool optional;
    const char * default_value;
};

typedef struct json_get_array_arg json_get_array_arg;
struct json_get_array_arg {
    const json_object * parent;
    const char * key;
    bool * success;
};

typedef struct json_get_object_arg json_get_object_arg;
struct json_get_object_arg {
    const json_object * parent;
    const char * key;
    bool * success;
    bool optional;
};

json_value * json_parse (const char * begin, const char * end);
json_value * json_lookup (const json_object * object, const char * key);
void json_free (json_value * value);
const char * json_type_name(json_type type);

double _json_get_number(json_get_number_arg arg);
#define json_get_number(...) _json_get_number((json_get_number_arg){__VA_ARGS__})

const char * _json_get_string(json_get_string_arg arg);
#define json_get_string(...) _json_get_string((json_get_string_arg){__VA_ARGS__})

const json_array * _json_get_array(json_get_array_arg arg);
#define json_get_array(...) _json_get_array((json_get_array_arg){__VA_ARGS__})

const json_object * _json_get_object(json_get_object_arg arg);
#define json_get_object(...) _json_get_object((json_get_object_arg){__VA_ARGS__})
