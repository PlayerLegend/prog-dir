typedef struct {
    char * flag;
    char * name;
    char * value;
}
    option_desc;

void load_options(option_desc * desc, const char * config_name);
