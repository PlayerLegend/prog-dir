#define for_range(iter_name,object)			\
    for( typeof((object).begin) iter_name = (object).begin, range_max = (object).end; iter_name != range_max; iter_name++ )

#define for_range_adjust(object)	\
    { range_max = (object).end; }

#define for_range_redo(iter_name)			\
    { (iter_name)--; continue; }

#define is_range_empty(object)			\
    ((object).begin == (object).end)

#define count_range(object)			\
    ((object).end - (object).begin)
