#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#endif

#define keyargs_struct_name(name) _keyargs_args_##name
#define keyargs_func_name(name) _keyargs_func_##name
#define keyargs_type_name(name) _keyargs_type_##name

#define keyargs_declare(nametype, name, ...) \
    typedef struct { __VA_ARGS__ } keyargs_struct_name(name);	\
    typedef nametype keyargs_type_name(name); \
    nametype keyargs_func_name(name) ( keyargs_struct_name(name) );

#define keyargs_declare_clone(new,old)		\
    typedef keyargs_struct_name(old) keyargs_struct_name(new);	\
    typedef keyargs_type_name(old) keyargs_type_name(new);	\
    keyargs_type_name(new) keyargs_func_name(new) ( keyargs_struct_name(new) );

#define keyargs_define(name) \
    keyargs_type_name(name) keyargs_func_name(name) ( keyargs_struct_name(name) args )

#define keyargs_declare_static(nametype, name, ...) \
    typedef struct { __VA_ARGS__ } keyargs_struct_name(name);	\
    typedef nametype keyargs_type_name(name); \
    static nametype keyargs_func_name(name) ( keyargs_struct_name(name) );

#define keyargs_define_static(name) \
    static keyargs_type_name(name) keyargs_func_name(name) ( keyargs_struct_name(name) args )

#define keyargs_call(name, ...) keyargs_func_name(name) ( (keyargs_struct_name(name)) { __VA_ARGS__ } )
