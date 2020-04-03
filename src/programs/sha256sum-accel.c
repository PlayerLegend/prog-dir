#include "precompiled.h"

#define FLAT_INCLUDES

//#include "range.h"
#include "index_map.h"
#include "hash_table.h"
#include "dictionary.h"
#include "hash_table_string.h"
#include "sha256.h"

typedef dictionary(char*) sums_dict;
typedef array(char*) lines_stack;

char * end_line_str(char * line, char * end)
{
    char * find = strstr(line,end);
    if(!find)
	return NULL;

    *find = '\0';
    return find + strlen(end);
}

/*int str_append(char ** string, char * fmt, ...)
{
    va_list va1,va2;

    va_start(va1,fmt);
    va_copy(va2,va1);
    
    int ret = 0;

    ret = vsnprintf(NULL,0,fmt,va1);
    va_end(va1);

    char * new;
    
    if(ret > 0)
    {
	size_t len = *string ? strlen(*string) + 1 : 0;
	printf("len: %zu -> %zu\n",len,len + ret + 1);
	new = realloc(*string,len + ret + 100);
	printf("new %p->%p: %s\n",*string,new,new);
	char * write = new + len;
	sprintf(write,fmt,va2);
	va_end(va2);
	*string = new;
    }
    
    return ret;
    }*/
int str_write(char ** string, const char * fmt, ...)
{
    va_list va;

    int ret = 0;

    va_start(va,fmt);
    ret = vsnprintf(NULL,0,fmt,va);
    va_end(va);

    if(!*string)
	*string = malloc(ret + 1);
    else
	*string = realloc(*string,ret + 1);
    
    **string = '\0';
    va_start(va,fmt);
    vsnprintf(*string,ret + 1,fmt,va);
    va_end(va);
    
    return ret;
}

void end_line_c(char * line, char end)
{
    char * c = strchr(line,end);
    if(c)
	*c = '\0';
}

char * dupe_str(const char * str)
{
    return strcpy(malloc(strlen(str) + 1),str);
}

void include_line(sums_dict * dict, const char * noise, const char * sum)
{
    char ** set_sum;
    
    set_sum = dictionary_access_key(dict,noise);
    if( *set_sum )
    {
	fprintf(stderr,"duplicate sum: %s %s\n",noise,sum);
	free(*set_sum);
    }
    *set_sum = dupe_str(sum);
}

void load_file(sums_dict * dict, const char * file_name)
{
    FILE * file = fopen(file_name,"r");

    if(!file)
    {
	perror(file_name);
	return;
    }
    
    struct { size_t len; char * text; } line = { 0 };

    char * noise;
    char * sum;
    
    while( -1 != getline(&line.text,&line.len,file) )
    {
	end_line_c(line.text,'\n');
	noise = line.text;
	sum = end_line_str(noise,"  ");
	if(!sum || strlen(sum) != 64 || strlen(noise) != 64)
	{
	    //fprintf(stderr,"Malformed line: %s\n",line.text);
	    continue;
	}
	include_line(dict,noise,sum);
    }

    fclose(file);
}

const char * basename(const char * name)
{
    static char * string;

    const char * write = strrchr(name,'/');

    if(!write)
	write = name;

    while(*write == '/')
	write++;

    str_write(&string,write);

    return string;
}

int gen_noise(sha256_armor noise, const char * file_name)
{
    static char * string;

    struct stat _stat;

    if( -1 == stat(file_name,&_stat) )
    {
	perror(file_name);
	return -1;
    }

    //printf("noise 1: %zu %zd %s\n",_stat.st_mtime,_stat.st_size,basename(file_name));
    str_write(&string,"%zu %zd %s\n",_stat.st_mtime,_stat.st_size,basename(file_name));

    static sha256 raw;

    sha256_buffer(raw,string,strlen(string));

    sha256_makearmor(noise,raw);
    
    return 0;
}

void gen_name(sums_dict * dict, FILE * dict_append, FILE * record_append, char * file_name)
{
    static sha256 raw;
    static sha256_armor armor_sum, armor_noise;

    if( -1 == gen_noise(armor_noise,file_name) )
    {
	return;
    }

    char ** lookup = dictionary_access_key(dict,armor_noise);

    char * sum;
    
    if( *lookup )
	sum = *lookup;
    else
    {
	sha256_path(raw,file_name,1 << 16);
	sha256_makearmor(armor_sum,raw);

	*lookup = dupe_str(armor_sum);

	sum = armor_sum;
        
	fprintf(dict_append,"%s  %s\n",armor_noise,armor_sum);
    }
    
    printf("%s  %s\n",sum,file_name);
    if(record_append)
	fprintf(record_append,"%s  %s\n",sum,file_name);
}

void test_sum(sums_dict * dict, char * noise)
{
    char * sum = *dictionary_access_key(dict,noise);

    printf("%s: %s\n",noise,sum);
}

size_t table_digest_sum(const void * key)
{
    return *(uint64_t*)key;
}

#define TABLE_CONFIG_SUMS			\
    (table_config){				\
	.gen_digest = table_digest_sum,		\
	    .copy = table_copy_string,		\
	    .equals = table_equals_string,	\
	    .free = table_free_string,		\
	    }

char ** find_args_max(int argc, char * argv[])
{
    static char ** max;

    if( max )
	return max;

    max = argv + argc;
    
    for( char **i = argv + 1; i < max; i++ )
    {
	if(strcmp(*i,"--") == 0)
	{
	    max = i;
	    return i;
	}
    }

    fprintf(stderr,"error: a -- is required in the arguments list\n");
    exit(1);
	
    return NULL;
}

const char * get_arg(int argc, char * argv[], char * flag, char * flag_default)
{
    char ** max = find_args_max(argc,argv);

    for( char **i = argv + 1; i < max; i++ )
    {
	if( strcmp(*i,flag) == 0 )
	{
	    if( i + 1 == max )
		fprintf(stderr,"%s: requires an option\n",flag);
	    else
		return i[1];
	}
    }

    return flag_default;
}

bool get_flag(int argc, char * argv[], char * flag)
{
    char ** max = find_args_max(argc,argv);

    for( char **i = argv + 1; i < max; i++ )
    {
	if(strcmp(*i,flag) == 0)
	    return true;
    }

    return false;
}

char * get_next_filename(int argc, char * argv[])
{
    static int next;

    if(next == 0)
    {
	next = find_args_max(argc,argv) - argv + 1;
    }

    if(next < argc)
	return argv[next++];
    else
	return NULL;
}

int main(int argc, char * argv[])
{
    const char * dict_file = get_arg(argc,argv,"--dict",".sha256sums");
    const char * record_file = get_arg(argc,argv,"--record",NULL);

    sums_dict dict = {};

    dictionary_table(&dict).config = TABLE_CONFIG_SUMS;
    
    load_file(&dict,dict_file);

    FILE * dict_append = fopen(dict_file,"a");
    
    if(!dict_append)
    {
	perror(dict_file);
	exit(1);
    }
    
    FILE * record_append;

    if(record_file)
    {
	record_append = fopen(record_file,"a");
	if(!record_append)
	    perror(record_file);
    }
    else
	record_append = NULL;
    
    
    char * file_name;
    while( (file_name = get_next_filename(argc,argv)) )
    {
	gen_name(&dict,dict_append,record_append,file_name);
    }
}
