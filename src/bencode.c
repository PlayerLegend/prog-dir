#include <stdint.h>
#include "dictionary.h"
#include "hash_table_string.h"
#include "stream.h"
#include <string.h>
#include <assert.h>

enum bencode_type {
    BEN_STRING,
    BEN_INT,
    BEN_LIST,
    BEN_DICT,
};

typedef struct bencode_node {

    enum bencode_type type;

    union {
	char * bstr;
	long long bint;
	array(struct bencode_node) list;
	dictionary(struct bencode_node) dict;
    };	
}
    bencode_node;

typedef struct {
    stream stream;
    
}
    bdecode;

#define init_node(node_type) (bencode_node){ .type = node_type }

void setup_bencode_dict(bencode_node * node)
{
    *node = init_node(BEN_DICT);
    dictionary_config(&node->dict) = TABLE_CONFIG_STRING;
}

bencode_node * bencode_dict_add(bencode_node * dict, char * key)
{
    assert(dict->type == BEN_DICT);

    return dictionary_access_key(&dict->dict,key);
}

void setup_bencode_list(bencode_node * node)
{
    *node = init_node(BEN_LIST);
}

bencode_node * bencode_list_add(bencode_node * list)
{
    assert(list->type == BEN_LIST);

    return array_push(&list->list);
}
