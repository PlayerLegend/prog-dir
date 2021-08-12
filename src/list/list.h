#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#endif

#define list1_typedef(type,name)				\
    typedef struct _list1_element_const_##name			\
    {							\
	const struct _list1_element_const_##name * peer;		\
	const type child;				\
    } list1_element_const_##name;				\
    typedef union _list1_element_##name				\
    {							\
	struct						\
	{						\
	    union _list1_element_##name * peer;			\
	    type child;					\
	};						\
	list1_element_const_##name const_cast;			\
    } list1_element_##name;					\
    typedef list1_element_##name* list1_handle_##name;		\
    typedef list1_element_const_##name* list1_handle_const_##name;	\

#define list_element(type) { void * peer; type child; }
#define list_handle(element_type) element_type*
#define list_bounds(element_type) { element_type* begin; element_type* end; }

inline static void * _list_pop(void ** list_handle)
{
    void * retval = *list_handle;
    if (retval)
    {
	*list_handle = *(void**)retval;
    }
    return retval;
}

#define list_pop(handle)			\
    (_list_pop ((void**)&(handle)))

#define list_push(handle,element)		\
    {						\
	(element).peer = handle;		\
	handle = &(element);			\
    }

#define list_bounds_push_end(bounds,element)		\
    {							\
	if (!(bounds).begin)				\
	    (bounds).begin = (bounds).end = &(element);	\
	else						\
	{						\
	    (bounds).end->peer = &(element);		\
	    (bounds).end = &(element);			\
	    (element).peer = NULL;			\
	}						\
    }

#define list_bounds_push_begin(bounds,element)		\
    {							\
	if (!(bounds).begin)				\
	    (bounds).begin = (bounds).end = &(element);	\
	else						\
	    list_push((bounds).begin,element);		\
    }

#define for_list(element,handle)					\
    for (element = handle; element; element = (element)->peer)
