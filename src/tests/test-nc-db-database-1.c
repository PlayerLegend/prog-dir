#define FLAT_INCLUDES

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "stack.h"
#include "array.h"
#include "index_map.h"
#include "hash_table.h"
#include "dictionary.h"
#include "nc-db/database.h"

char * write_string(const char * string)
{
    static char * ret;

    if(string == NULL)
    {
	free(ret);
	ret = NULL;
	return NULL;
    }

    ret = realloc(ret,strlen(string) + 1);
    strcpy(ret,string);

    return ret;    
}

int main(int argc, char * argv[])
{
    if( argc != 3 )
    {
	printf("%s [single db] [multiple db]\n", argv[0]);
	exit(1);
    }

    struct {
	const char *single, *multiple;
    }
    db_name = { argv[1], argv[2] };

    struct {
	db_single * single;
	db_multiple * multiple;
    }
    db;

    assert( -1 != init_db() );

    key_value kv;

    assert( -1 == db_make_kv(&kv,write_string("notasumsline")) );
    assert( -1 == db_make_kv(&kv,write_string("not asumsline")) );
    assert( -1 != db_make_kv(&kv,write_string("is  asumsline")) );
    assert( kv.key != kv.value );
    assert( kv.key != 0 );
    assert( kv.value != 0 );

    printf("loading single database %s\n",db_name.single);
    
    assert( -1 != db_load_single(&db.single,db_name.single) );

    db_add_single(db.single,(key_value){ 5, 7 });
    db_add_single(db.single,(key_value){ 1, 2 });
    db_add_single(db.single,(key_value){ 1, 3 });
    db_add_single(db.single,(key_value){ 1, 4 });
    db_add_single(db.single,(key_value){ 2, 4 });
    db_add_single(db.single,(key_value){ 3, 4 });
    db_add_single(db.single,(key_value){ 4, 4 });

    assert( 7 == db_lookup_forward(db.single,5) );
    assert( 5 == db_lookup_reverse(db.single,7) );

    assert( 0 == db_lookup_forward(db.single,1) );
    assert( 0 == db_lookup_forward(db.single,2) );
    assert( 0 == db_lookup_forward(db.single,3) );

    assert( 0 == db_lookup_reverse(db.single,1) );
    assert( 0 == db_lookup_reverse(db.single,2) );
    assert( 0 == db_lookup_reverse(db.single,3) );

    assert( 4 == db_lookup_forward(db.single,4) );
    assert( 4 == db_lookup_reverse(db.single,4) );
    
    assert( -1 != db_load_multiple(&db.multiple,db_name.multiple) );

    write_string(NULL);
}
