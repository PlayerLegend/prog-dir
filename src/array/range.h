#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#endif

#define range(type)				\
    { type *begin; type *end; }

#define range_count(range)			\
    ( (range).end - (range).begin )

#define range_is_empty(range)			\
    ( (range).end == (range).begin )

#define for_range(iter_name, object)					\
    for (iter_name = (object).begin; iter_name != (object).end; iter_name++)

#define for_range_redo(iter_name)		\
    { (iter_name)--; continue; }

#define range_index(element,container)		\
    ( (element) - (container).begin )

#define range_alloc(range, count)		\
    {									\
	(range).begin = malloc (sizeof (*(range).begin) * count);	\
	(range).end = (range).begin + count;				\
    }

#define range_calloc(range, count)		\
    {									\
	(range).begin = calloc (count, sizeof (*(range).begin));		\
	(range).end = (range).begin + count;				\
    }

#define range_typedef(rangetype, name, ...)			\
    typedef union { struct range(const rangetype); __VA_ARGS__; } range_const_##name; \
    typedef union { struct range(rangetype); range_const_##name const_cast; __VA_ARGS__; } range_##name;

//#define range_fread(result, tmp, file_pointer) { (result).begin = (tmp).begin; (result).end = (tmp).begin + fread( (tmp).begin, sizeof(*(tmp).begin), range_count (tmp), file_pointer ); }

range_typedef (char, char);
range_typedef (unsigned char, unsigned_char, range_char char_cast);
range_typedef (char*, string);
range_typedef (void, void);
