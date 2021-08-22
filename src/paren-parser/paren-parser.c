#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#define FLAT_INCLUDES
#include "../immutable/immutable.h"
#include "../keyargs/keyargs.h"
#include "paren-parser.h"
#include "../log/log.h"
#include "../array/range.h"
#include "../array/buffer.h"

#define PAREN_OPEN '('
#define PAREN_CLOSE ')'

inline static void skip_space (const char ** text, size_t * newline_count, const char ** last_newline)
{
    while (**text && isspace(**text))
    {
	if (**text == '\n')
	{
	    (*newline_count)++;
	    *last_newline = *text;
	}
	(*text)++;
    }

    assert (**text != ' ');
    assert (**text != '\n');
}

inline static void skip_word (const char ** text)
{
    while (**text && !isspace (**text) && **text != PAREN_OPEN && **text != PAREN_CLOSE)
    {
	(*text)++;
    }
}

void paren_atom_free (paren_atom * atom)
{
    if (!atom)
    {
	return;
    }
    
    paren_atom * here;

    while ( (here = atom) )
    {
	atom = atom->peer;

	if (!here->child.is_text)
	{
	    paren_atom_free (here->child.atom);
	}

	free (here);
    }
}

#define fatal(STR, ...) { log_error("%s:%d:%d: " STR, arg.filename, line_number, iter - last_newline, ##__VA_ARGS__); goto fail; }

inline static void copy_range (buffer_char * output, range_const_char * input)
{
    size_t alloc = range_count (*input) + 1;
    buffer_resize(*output, alloc);
    memcpy(output->begin, input->begin, alloc - 1);
    output->end--;
    *output->end = '\0';
}

inline static paren_atom * new_node (unsigned int line_number, unsigned int column_number, const char * filename)
{
    paren_atom * retval = calloc (1, sizeof (*retval));

    retval->line_number = line_number;
    retval->column_number = column_number;
    retval->filename = filename;

    return retval;
}

inline static char * _load_file (const char * filename)
{
    struct stat s;

    if (0 != stat (filename, &s))
    {
	perror (filename);
	return NULL;
    }

    char * retval = malloc (s.st_size + 1);

    if (!retval)
    {
	perror ("malloc");
	return NULL;
    }

    ssize_t read_size;

    FILE * file = fopen (filename, "r");

    read_size = fread (retval, 1, s.st_size, file);

    fclose (file);
    
    if (s.st_size != read_size)
    {
	if (read_size < 0)
	{
	    perror (filename);
	}
	else
	{
	    log_error ("%s: read file size was less than reported by stat(), expected %zd but got %zd", filename, s.st_size, read_size);
	}

	free (retval);
	return NULL;
    }
    else
    {
	retval[s.st_size] = '\0';
	return retval;
    }
}

keyargs_define(paren_parse)
{
    if (!args.filename)
    {
	args.filename = "input";
    }
    else
    {
	args.filename = immutable_path (args.namespace, args.filename);
    }

    char * self_load_text;

    if (!args.text)
    {
	self_load_text = _load_file (args.filename);
	args.text = self_load_text;
	if (!self_load_text)
	{
	    return NULL;
	}
    }
    else
    {
	self_load_text = NULL;
    }
    
    const char * iter = args.text;
    char c;
    struct buffer(paren_atom**) paths = {0};
    buffer_char word_buffer = {0};
    range_const_char word_range;
    paren_atom * retval = NULL;
    paren_atom ** set = &retval;

    const paren_atom error_dummy = { .filename = args.filename };
    const paren_atom * error_atom = &error_dummy;

    size_t line_number = 1;
    const char * last_newline = iter;
    
    while (*iter)
    {
	skip_space (&iter, &line_number, &last_newline);

	c = *iter;

	if (c == PAREN_OPEN)
	{
	    iter++;
	    assert (set);
	    error_atom = *set = new_node (line_number, iter - last_newline, immutable_string (NULL, args.filename));
	    *buffer_push (paths) = &(*set)->peer;
	    set = &(*set)->child.atom;
	}
	else if (c == PAREN_CLOSE)
	{
	    iter++;
	    if (range_count (paths))
	    {
		set = *(--paths.end);
	    }
	    else
	    {
		paren_fatal (error_atom, "Unexpected '%c'", PAREN_CLOSE);
	    }
	}
	else if (!c)
	{
	    break;
	}
	else
	{
	    word_range.begin = iter;
	    skip_word (&iter);
	    word_range.end = iter;

	    copy_range (&word_buffer, &word_range);

	    error_atom = *set = new_node (line_number, word_range.begin - last_newline, immutable_string (NULL, args.filename));

	    (*set)->child.is_text = true;
	    (*set)->child.text = immutable_string (NULL, word_buffer.begin);

	    set = &(*set)->peer;
	}

    }

    if (range_count (paths) != 0)
    {
	paren_fatal (error_atom, "Expected '%c'", PAREN_CLOSE);
    }
    else
    {
	goto success;
    }

fail:

    if (self_load_text)
    {
	free (self_load_text);
    }
    
    free (paths.begin);
    free (word_buffer.begin);

    paren_atom_free(retval);
    
    return NULL;

success:
    
    if (self_load_text)
    {
	free (self_load_text);
    }
    
    free (paths.begin);
    free (word_buffer.begin);

    return retval;
}
