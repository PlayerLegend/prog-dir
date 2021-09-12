#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#endif

/**
   @file keyargs.h
   This file provides macros which allow for the creation of functions that accept .key=value based argument lists. These argument lists have the same syntax as structure initializers, so they will additionally accept regular ordered argument lists and allow for optional arguments.
*/

#define keyargs_struct_name(name) _keyargs_args_##name
#define keyargs_func_name(name) _keyargs_func_##name
#define keyargs_type_name(name) _keyargs_type_##name

#define keyargs_declare(nametype, name, ...) \
    typedef struct { __VA_ARGS__ } keyargs_struct_name(name);	\
    typedef nametype keyargs_type_name(name); \
    nametype keyargs_func_name(name) ( keyargs_struct_name(name) );
/**<
   Declare a keyargs function that will be referred to as 'name' in subsequent keyargs_macros and whose whose return type is 'nametype'. The variable arguments portion of the call may specify arguments that the function will take. The syntax for these argument definitions is the same as the syntax for defining a struct, each item will be delimited by a semicolon, and a semicolon is required after the last item and before the closing ) of the macro's argument list.
*/

#define keyargs_declare_clone(new,old)		\
    typedef keyargs_struct_name(old) keyargs_struct_name(new);	\
    typedef keyargs_type_name(old) keyargs_type_name(new);	\
    keyargs_type_name(new) keyargs_func_name(new) ( keyargs_struct_name(new) );
/**<
   Creates an exact copy of a keyargs declaration created by keyargs_declare
*/

#define keyargs_define(name) \
    keyargs_type_name(name) keyargs_func_name(name) ( keyargs_struct_name(name) args )
/**<
   Begins the definition of a previous keyargs declaration created by keyargs_declare. This macro should be followed by a function body. Within the function body, the arguments given to the function are referenced as part of an 'args' structure. So, if the keyargs function in question has an argument called 'ARG', one would get the value of 'ARG' passed to the keyargs function by referencing args.ARG within the function's body.
*/

#define keyargs_declare_static(nametype, name, ...) \
    typedef struct { __VA_ARGS__ } keyargs_struct_name(name);	\
    typedef nametype keyargs_type_name(name); \
    static nametype keyargs_func_name(name) ( keyargs_struct_name(name) );
/**<
   This macro does the same job as keyargs_declare, but the function it declares is statically scoped.
*/

#define keyargs_define_static(name) \
    static keyargs_type_name(name) keyargs_func_name(name) ( keyargs_struct_name(name) args )
/**<
   This macro does the same job as keyargs_define, but the function it defines must be a statically scoped declaration created by keyargs_declare_static, rather than keyargs_declare.
*/

#define keyargs_call(name, ...) keyargs_func_name(name) ( (keyargs_struct_name(name)) { __VA_ARGS__ } )
/**<
   This macro calls the keyargs function named 'name' with arguments given in the varargs portion of the macro call. To turn this into a function with a similar appearance to common C functions, you can wrap this macro in another function-like macro named after the keyargs function in question, i.e.:

   #define some_function(...) keyargs_call(some_function, __VA_ARGS__)
*/
