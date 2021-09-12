#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#endif

/**
   @file range.h
   Describes a generic range data structure and associated operations.
   This range structure provides a begin and end pointer for demarcating a continuous portion of memory.
*/

#define range(type)				\
    { type *begin; type *end; }
/**<
   @brief The body of a range-type structure, it contains a begin and end pointer for the given type
   @param type The type of the resulting range structure
*/

#define range_count(range)			\
    ( (range).end - (range).begin )
/**<
   @brief Evaluates to the number of elements in a range-like structure
   @param range The range to be counted
*/

#define range_is_empty(range)			\
    ( (range).end == (range).begin )
/**<
   @brief Evaluates to true if the range-like structure contains no items, false otherwise
   @param range The range to be checked
*/

#define for_range(iter_name, object)					\
    for (iter_name = (object).begin; iter_name != (object).end; iter_name++)
/**<
   @brief A range-based for loop
   @param iter_name Specifies the iterator to use. This iterator must be declared beforehand
   @param object Specifies the range-like structure to iterate over
*/

#define for_range_redo(iter_name)		\
    { (iter_name)--; continue; }
/**<
   @brief When used within a for_range loop, this causes the current loop to end and the subsequent loop to iterate over the current item again
*/

#define range_index(element,container)		\
    ( (element) - (container).begin )
/**<
   @brief Returns the index of the given element within the range
*/

#define range_alloc(range, count)		\
    {									\
	(range).begin = malloc (sizeof (*(range).begin) * count);	\
	(range).end = (range).begin + count;				\
    }
/**<
   @brief Allocates the number of elements given by count within the given range
   @param range The range in which to allocate elements
   @param count The number of elements to allocate
*/

#define range_calloc(range, count)		\
    {									\
	(range).begin = calloc (count, sizeof (*(range).begin));		\
	(range).end = (range).begin + count;				\
    }
/**<
   @brief Allocates the number of elements given by count within the given range, with the newly allocated elements all set to 0
   @param range The range in which to allocate elements
   @param count The number of elements to allocate
*/

#define range_typedef(rangetype, name, ...)			\
    typedef union { struct range(const rangetype); __VA_ARGS__; } range_const_##name; \
    typedef union { struct range(rangetype); range_const_##name const_cast; __VA_ARGS__; } range_##name;
/**<
   @brief Creates non-const and const typedefs for the given range type that are of the form range_NAME and range_const_NAME, where NAME is given by the name argument. Both structures will contain the usual begin and end members of a range object, but the non-const type will contain a const_cast union member that acts as a const qualified alias to the parent.
   @param rangetype The type of the items contained within the range
   @param name The name to attach to the range
   @param ... Additional member declarations to be placed in the resulting range structures may optionally go here. These will be union members, and so will overlap with the other members of the range, as the const_cast member does.
*/

//#define range_fread(result, tmp, file_pointer) { (result).begin = (tmp).begin; (result).end = (tmp).begin + fread( (tmp).begin, sizeof(*(tmp).begin), range_count (tmp), file_pointer ); }

range_typedef (char, char);
range_typedef (unsigned char, unsigned_char, range_char char_cast);
range_typedef (char*, string);
range_typedef (void, void);

/**
   @union range_char 
   A range containing char items created by the range_typedef macro
*/

/**
   @union range_const_char
   A range containing const char items created by the range_typedef macro
*/

/**
   @union range_unsigned_char
   A range containing unsigned char items created by the range_typedef macro
*/

/**
   @var range_unsigned_char::char_cast
   Aliases the parent's unsigned char contents to signed char
*/

/**
   @union range_const_unsigned_char
   A range containing const unsigned char items created by the range_typedef macro
*/

/**
   @var range_const_unsigned_char::char_cast
   Aliases the parent's const unsigned char contents to NON CONST signed char. It should, however, be addressed as const, and will (hopefully) be const in the future.
   @todo Make this cast to const
*/

/**
   @union range_string
   A range of char pointer items created by the range_typedef macro
*/

/**
   @union range_void
   A range of void items
*/
