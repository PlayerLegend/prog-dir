_buffer_resize ( (buffer_void*)&(buffer), sizeof (*(buffer).begin), 10 + ((buffer).max - (buffer).begin) * 3 )

//(( (size_t)((buffer).max - (buffer).begin) <= (size_t)(count) )	\
//   ? _buffer_realloc ((buffer_void*)&(buffer), sizeof (*(buffer).begin), (count) * 2) \
// : 0)

//    ((buffer).end == (buffer).max ? _buffer_resize ( (buffer_void*)&(buffer), sizeof (*(buffer).begin), 10 + ((buffer).max - (buffer).begin) * 3 ), (buffer).end++ : (buffer).end++)
