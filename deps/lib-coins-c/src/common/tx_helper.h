
typedef enum {
    STEP_BEGIN = 0,
    STEP_CONTINUOUS = 1,
    STEP_END = 2,
    STEP_VERSION = 100
} hash_step_t;

#define HASHER(hasher, param, output, offset, step) do {\
    ret = hasher(param, output, offset, step);\
    if (ret != 0) {\
        return ret;\
    }\
    offset = 0;\
} while(0)