#ifndef IGLOO_TYPES_H
#define IGLOO_TYPES_H

enum flag {
    OFF,
    ON
};

typedef struct igloo_str {
    unsigned char *str;
    size_t len;
} igloo_str;

enum igloo_cmd_type {
    CAT,
    SALT,
    SUB,
    CUT,
    REV,
    XOR,
    REPLACE,
    SHUFFLE,
    CCIPHER,
    RCIPHER,
    COMPLEMENT,
    GENXOR,
    PASSWORDIFY,
    FORK,
    SUBSTR,
    JOIN,
    ITERATE,
    ITERATE_END,
    IG_SHA2_256,
    IG_SHA2_512,
    IG_SHA3_256,
    IG_SHA3_512,
    IG_BLAKE2B,
    IG_BLAKE2S,
    IG_SHAKE128,
    IG_SHAKE256,
};

enum igloo_arg_type {
    INT,
    SIZE_T,
    STRING,
    MIXED,
    NONE,
};

union igloo_arg {
    igloo_str *istr;
    int i;
    size_t sz;
};

#define CAT_FRONT 0
#define CAT_BACK 1
#define NO_LIMIT 0
#define DYNAMIC_SEED 0
typedef struct igloo_cmd {
    enum igloo_cmd_type cmd_t;
    enum igloo_arg_type arg1_t;
    enum igloo_arg_type arg2_t;
    enum igloo_arg_type arg3_t;
    union igloo_arg arg1;
    union igloo_arg arg2;
    union igloo_arg arg3;
} igloo_cmd;

#define IGLOO_CMDS_INIT_SIZE 16
typedef struct igloo_cmds {
    igloo_cmd *cmds;
    size_t size;
    size_t used;
} igloo_cmds;

#define IGLOO_STR_STACK_INIT_SIZE 8
typedef struct igloo_str_stack {
    igloo_str **stack;
    size_t size;
    size_t used;
} igloo_str_stack;

struct igloo_cmd_state_node {
    igloo_cmd *icmd;
    size_t iteration_count;
    size_t cmd_ind;
};

#define IGLOO_CMD_STATE_INIT_SIZE 4
typedef struct igloo_cmd_state {
    struct igloo_cmd_state_node *state;
    size_t size;
    size_t used;
} igloo_cmd_state;

typedef struct igloo {
    /* algorithms to use for obfuscation, basically a queue of operations */
    igloo_cmds cmds;
    /* location to save commands to */
    const char *profile;
    /* password to obscure */
    igloo_str pwd;
    /* number of passwords to convert, 0 is unbounded */
    size_t pwd_amt;
    /* length to truncate obscured password to, 0 is unbounded */
    size_t truncate;
    /* file name to output password to. The file name will be suffixed with the
     * current password number, unless pwd_amt == 1 */
    char *output;
    /* suppress printing to stdout, except the obscured password */
    enum flag quiet;
} igloo;

#endif
