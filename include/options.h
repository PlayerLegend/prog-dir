typedef struct {
    const char * config_name;
    const char * getopt_flags;
}
    option_config;

typedef struct {
    int flag;
    const char * name;
    const char * handle;
    char * value;
}
    option_desc;

void load_options(option_desc * desc, option_config * config);
