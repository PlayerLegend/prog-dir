#include "delimit.h"
#include "print.h"
#include <stdlib.h>
#include <string.h>
#include "range.h"

void test_clauses(const char * file_name,  clause_config config)
{
    FILE * file;

    int count = 0;

    printf("\nStarting clause test for %s, separators '%s'(%d), separator count %d\n",file_name,config.separator_list,strlen(config.separator_list),config.separator_count);

    if( NULL == (file = fopen(file_name,"r")) )
    {
	perror(file_name);
	exit(1);
    }

    struct { char * text; size_t len; } line = { 0 };

    clause clause;
    
    while( -1 != getline(&line.text,&line.len,file) )
    {
	delimit_terminate(line.text,'\n');
	if( -1 == delimit_clause(&clause,&config,line.text) )
	    printf("[%03d] failed clause for '%s'\n",count,line.text);
	else
	    printf("[%03d] clause: [%s] [%s]\n",count,clause.subject,clause.predicate);
	clear_clause(&clause);
	count++;
    }

    free(line.text);
}

void test_lists(const char * file_name,  delimit_config config)
{
    FILE * file;

    int count = 0;

    printf("\nStarting list test for %s, whitespace '%s'(%d)\n",file_name,config.whitespace,strlen(config.whitespace));

    if( NULL == (file = fopen(file_name,"r")) )
    {
	perror(file_name);
	exit(1);
    }

    struct { char * text; size_t len; } line = { 0 };

    delimited_list list = { 0 };
    
    while( -1 != getline(&line.text,&line.len,file) )
    {
	delimit_terminate(line.text,'\n');
	if( -1 == delimit_list(&list,&config,line.text) )
	    printf("[%03d] failed list for '%s'\n",count,line.text);
	else
	{
	    printf("[%03d] list:",count);
	    for_range(word,list)
		printf(" [%s]",*word);
	    printf("\n");
	}
		
	clear_delimited_list(&list);
	count++;
    }

    free(line.text);
}

int main(int argc, char * argv[])
{
    if( argc != 3 )
    {
	print_error("usage: %s [predicates file] [lists file]",argv[0]);
	return 1;
    }
    
    test_clauses(argv[1], (clause_config){ .separator_list = " " });
    test_clauses(argv[1], (clause_config){ .separator_list = " ", .separator_count = 1});
    test_clauses(argv[1], (clause_config){ .separator_list = " ", .separator_count = 2});
    test_clauses(argv[1], (clause_config){ .separator_list = " ", .separator_count = 3});
    test_clauses(argv[1], (clause_config){ .separator_list = " \t" });
    test_clauses(argv[1], (clause_config){ .separator_list = " \t", .separator_count = 1});
    test_clauses(argv[1], (clause_config){ .separator_list = " \t", .separator_count = 2});
    test_clauses(argv[1], (clause_config){ .separator_list = " \t", .separator_count = 3});

    test_lists(argv[2], (delimit_config){ .whitespace = " ", .quote = "\"" });
    test_lists(argv[2], (delimit_config){ .whitespace = " ", .quote = "\"", .escape = '\\' });
    test_lists(argv[2], (delimit_config){ .whitespace = " \t", .quote = "\"", .escape = '\\' });
}
