#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#endif

typedef enum log_level log_level;
enum log_level {
    LOG_NORMAL,
    LOG_ERROR,
    LOG_DEBUG,
};

void _log (log_level level, const char * fmt, ...);

#define plural(singular,plural,count) ( (count) > 1 ? (plural) : (singular) )

#define log_error(fmt, ...) _log (LOG_ERROR, "ERROR: " fmt "\n", ##__VA_ARGS__)
#define log_stderr(fmt, ...) _log (LOG_ERROR, fmt "\n", ##__VA_ARGS__)
#define log_normal(fmt, ...) _log (LOG_NORMAL, fmt "\n", ##__VA_ARGS__)
#define log_debug(fmt, ...) _log (LOG_DEBUG, "DEBUG: " fmt "\n", ##__VA_ARGS__)
#define log_fatal(fmt, ...) { _log (LOG_ERROR, "FATAL: " fmt "\n", ##__VA_ARGS__); goto fail; }
