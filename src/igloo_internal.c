#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "igloo_internal.h"

int igloo_dynamic_read(FILE *st, char **strp, size_t *len)
{
    int err = 0;
    char *str;
    char *stri;
    char *tmp;
    size_t prev_len;

    str = *strp;
    if (str == NULL){
        str = malloc(sizeof(char) * BUFFER_INIT_SIZE);
        *len = BUFFER_INIT_SIZE;
        err = str == NULL;
    }

    if (err == 0 && fgets(str, *len, st) == NULL){
        err = -1;
    }
    else if (err == 0){
        stri = str;
        while (strchr(stri, '\n') == NULL){
            prev_len = *len;
            *len *= 2;
            if ((tmp = reallocarray(str, *len, sizeof(char))) == NULL){
                err = 1;
                break;
            }
            else{
                str = tmp;
            }

            stri = str + prev_len - 1;
            if (fgets(stri, prev_len + 1, st) == NULL){
                err = 1;
                break;
            }
        }
    }

    if (err != 0){
        explicit_bzero(str, sizeof(char) * *len);
        free(str);
        *strp = NULL;
        *len = 0;
    }
    else{
        *strp = str;
    }

    return err;
}

igloo_str * igloo_str_create(const unsigned char *str, size_t len)
{
    igloo_str *istr;

    if (len == 0){
        return NULL;
    }
    else if ((istr = malloc(sizeof(igloo_str))) == NULL){
        return NULL;
    }
    else if ((istr->str = malloc(sizeof(unsigned char) * len)) == NULL){
        free(istr);
        return NULL;
    }
    else{
        memcpy(istr->str, str, len);
        istr->len = len;
        return istr;
    }
}

void igloo_str_free(igloo_str *istr)
{
    if (istr != NULL){
        explicit_bzero(istr->str, istr->len);
        istr->len = 0;
        free(istr->str);
        free(istr);
    }
}

static int encode_str(FILE *st, const igloo_str *istr, enum flag password_encode)
{
    int err = 0;
    const unsigned char *str;
    size_t len;

    str = istr->str;
    len = istr->len;
    while (len-- > 0){
        if ((password_encode == ON &&
                (*str < 32 || *str > 126)) ||
                (password_encode == OFF &&
                (*str <= 32 || *str > 126 || *str == ',' || *str == '\\'))){
            if (fprintf(st, "\\x%02X", *str++) != 4){
                err = 1;
                break;
            }
        }
        else if (fputc(*str++, st) == EOF){
            err = 1;
            break;
        }
    }

    return err;
}

#define IGLOO_STR_EQ(str, len, type) (  \
    (len == (sizeof(type) - 1)) && \
    strncmp(str, type, len) == 0)

#define IGLOO_CMD_SETUP(icmd, icmd_t, t1, t2, t3) ({  \
    int ret;                                          \
    icmd->cmd_t = icmd_t;                             \
    icmd->arg1_t = t1;                                \
    icmd->arg2_t = t2;                                \
    icmd->arg3_t = t3;                                \
    ret = (t1 != NONE) + (t2 != NONE) + (t3 != NONE); \
    ret;})

int igloo_cmd_map_type(igloo_cmd *icmd, const char *str, size_t len)
{
    int ret;

    if (IGLOO_STR_EQ(str, len, "cat")){
        ret = IGLOO_CMD_SETUP(icmd, CAT, MIXED, STRING, NONE);
    }
    else if (IGLOO_STR_EQ(str, len, "salt")){
        ret = IGLOO_CMD_SETUP(icmd, SALT, MIXED, NONE, NONE);
    }
    else if (IGLOO_STR_EQ(str, len, "sub")){
        ret = IGLOO_CMD_SETUP(icmd, SUB, STRING, STRING, SIZE_T);
    }
    else if (IGLOO_STR_EQ(str, len, "cut")){
        ret = IGLOO_CMD_SETUP(icmd, CUT, INDEX, NONE, NONE);
    }
    else if (IGLOO_STR_EQ(str, len, "rev")){
        ret = IGLOO_CMD_SETUP(icmd, REV, NONE, NONE, NONE);
    }
    else if (IGLOO_STR_EQ(str, len, "xor")){
        ret = IGLOO_CMD_SETUP(icmd, XOR, STRING, NONE, NONE);
    }
    else if (IGLOO_STR_EQ(str, len, "replace")){
        ret = IGLOO_CMD_SETUP(icmd, REPLACE, STRING, INDEX, NONE);
    }
    else if (IGLOO_STR_EQ(str, len, "shuffle")){
        ret = IGLOO_CMD_SETUP(icmd, SHUFFLE, SIZE_T, NONE, NONE);
    }
    else if (IGLOO_STR_EQ(str, len, "ccipher")){
        ret = IGLOO_CMD_SETUP(icmd, CCIPHER, INT, NONE, NONE);
    }
    else if (IGLOO_STR_EQ(str, len, "rcipher")){
        ret = IGLOO_CMD_SETUP(icmd, RCIPHER, SIZE_T, NONE, NONE);
    }
    else if (IGLOO_STR_EQ(str, len, "complement")){
        ret = IGLOO_CMD_SETUP(icmd, COMPLEMENT, NONE, NONE, NONE);
    }
    else if (IGLOO_STR_EQ(str, len, "genxor")){
        ret = IGLOO_CMD_SETUP(icmd, GENXOR, SIZE_T, NONE, NONE);
    }
    else if (IGLOO_STR_EQ(str, len, "passwordify")){
        ret = IGLOO_CMD_SETUP(icmd, PASSWORDIFY, SIZE_T, STRING, NONE);
    }
    else if (IGLOO_STR_EQ(str, len, "fork")){
        ret = IGLOO_CMD_SETUP(icmd, FORK, NONE, NONE, NONE);
    }
    else if (IGLOO_STR_EQ(str, len, "substr")){
        ret = IGLOO_CMD_SETUP(icmd, SUBSTR, INDEX, INDEX, NONE);
    }
    else if (IGLOO_STR_EQ(str, len, "join")){
        ret = IGLOO_CMD_SETUP(icmd, JOIN, STRING, NONE, NONE);
    }
    else if (IGLOO_STR_EQ(str, len, "iterate")){
        ret = IGLOO_CMD_SETUP(icmd, ITERATE, SIZE_T, NONE, NONE);
    }
    else if (IGLOO_STR_EQ(str, len, "iterate_end")){
        ret = IGLOO_CMD_SETUP(icmd, ITERATE_END, NONE, NONE, NONE);
    }
    else if (IGLOO_STR_EQ(str, len, "sha2-256")){
        ret = IGLOO_CMD_SETUP(icmd, IG_SHA2_256, NONE, NONE, NONE);
    }
    else if (IGLOO_STR_EQ(str, len, "sha2-512")){
        ret = IGLOO_CMD_SETUP(icmd, IG_SHA2_512, NONE, NONE, NONE);
    }
    else if (IGLOO_STR_EQ(str, len, "sha3-256")){
        ret = IGLOO_CMD_SETUP(icmd, IG_SHA3_256, NONE, NONE, NONE);
    }
    else if (IGLOO_STR_EQ(str, len, "sha3-512")){
        ret = IGLOO_CMD_SETUP(icmd, IG_SHA3_512, NONE, NONE, NONE);
    }
    else if (IGLOO_STR_EQ(str, len, "blake2s")){
        ret = IGLOO_CMD_SETUP(icmd, IG_BLAKE2S, SIZE_T, NONE, NONE);
    }
    else if (IGLOO_STR_EQ(str, len, "blake2b")){
        ret = IGLOO_CMD_SETUP(icmd, IG_BLAKE2B, SIZE_T, NONE, NONE);
    }
    else if (IGLOO_STR_EQ(str, len, "shake128")){
        ret = IGLOO_CMD_SETUP(icmd, IG_SHAKE128, SIZE_T, NONE, NONE);
    }
    else if (IGLOO_STR_EQ(str, len, "shake256")){
        ret = IGLOO_CMD_SETUP(icmd, IG_SHAKE256, SIZE_T, NONE, NONE);
    }
    else{
        ret = -1;
    }

    return ret;
}

int igloo_cmd_add_arg(igloo_cmd *icmd, int arg_num, const char *str, size_t len)
{
    int err = 0;
    unsigned char *base;
    enum igloo_arg_type *arg_t;
    union igloo_arg *arg;
    char *endptr;

    base = (unsigned char *)icmd;
    if (arg_num == 1){
        arg_t = (enum igloo_arg_type *)(base + offsetof(struct igloo_cmd, arg1_t));
        arg = (union igloo_arg *)(base + offsetof(struct igloo_cmd, arg1));
    }
    else if (arg_num == 2){
        arg_t = (enum igloo_arg_type *)(base + offsetof(struct igloo_cmd, arg2_t));
        arg = (union igloo_arg *)(base + offsetof(struct igloo_cmd, arg2));
    }
    else if (arg_num == 3){
        arg_t = (enum igloo_arg_type *)(base + offsetof(struct igloo_cmd, arg3_t));
        arg = (union igloo_arg *)(base + offsetof(struct igloo_cmd, arg3));
    }
    else{
        err = 1;
    }

    if (err == 0){
        switch (*arg_t){
        case INT:
            arg->i = strtol(str, &endptr, 10);
            err = str == endptr;
            break;

        case SIZE_T:
            arg->sz = strtoul(str, &endptr, 10);
            err = str == endptr;
            break;

        case INDEX:
            arg->sz = strtoul(str, &endptr, 10);
            err = str == endptr;
            if (err == 0 && *endptr == '%'){
                *arg_t = PERCENT;
                err = arg->sz > 100;
            }
            break;

        case STRING:
            arg->istr = igloo_str_create((unsigned char *)str, len);
            err = arg->istr == NULL;
            break;

        case MIXED:
            if (icmd->cmd_t == CAT || icmd->cmd_t == SALT){
                if (IGLOO_STR_EQ(str, len, "front")){
                    arg->i = CAT_FRONT;
                }
                else if (IGLOO_STR_EQ(str, len, "back")){
                    arg->i = CAT_BACK;
                }
                else{
                    err = 1;
                }
            }
            else{
                err = 1;
            }
            break;

        case PERCENT:
        case NONE:
            err = 1;
            break;
        }
    }

    return err;
}

static int encode_arg(FILE *st, enum igloo_cmd_type cmd_t,
    const union igloo_arg arg, enum igloo_arg_type arg_t)
{
    int err = 0;

    if (arg_t == NONE){
        err = 0;
    }
    else if (arg_t == INT){
        if (fprintf(st, "%d", arg.i) <= 0){
            err = 1;
        }
    }
    else if (arg_t == SIZE_T || arg_t == INDEX){
        if (fprintf(st, "%zu", arg.sz) <= 0){
            err = 1;
        }
    }
    else if (arg_t == PERCENT){
        if (fprintf(st, "%zu%%", arg.sz) <= 0){
            err = 1;
        }
    }
    else if (arg_t == STRING){
        err = encode_str(st, arg.istr, OFF);
    }
    else if (arg_t == MIXED){
        if (cmd_t == CAT || cmd_t == SALT){
            if (arg.i == CAT_FRONT){
                if (fprintf(st, "front") != 5){
                    err = 1;
                }
            }
            else if (arg.i == CAT_BACK){
                if (fprintf(st, "back") != 4){
                    err = 1;
                }
            }
            else{
                err = 1;
            }
        }
        else{
            err = 1;
        }
    }
    else{
        err = 1;
    }

    return err;
}

static int encode_cmd(FILE *st, const igloo_cmd *icmd)
{
    const char *s;

    switch (icmd->cmd_t){
    case CAT:
        s = "cat";
        break;
    case SALT:
        s = "salt";
        break;
    case SUB:
        s = "sub";
        break;
    case CUT:
        s = "cut";
        break;
    case REV:
        s = "rev";
        break;
    case XOR:
        s = "xor";
        break;
    case REPLACE:
        s = "replace";
        break;
    case SHUFFLE:
        s = "shuffle";
        break;
    case CCIPHER:
        s = "ccipher";
        break;
    case RCIPHER:
        s = "rcipher";
        break;
    case COMPLEMENT:
        s = "complement";
        break;
    case GENXOR:
        s = "genxor";
        break;
    case PASSWORDIFY:
        s = "passwordify";
        break;
    case FORK:
        s = "fork";
        break;
    case SUBSTR:
        s = "substr";
        break;
    case JOIN:
        s = "join";
        break;
    case ITERATE:
        s = "iterate";
        break;
    case ITERATE_END:
        s = "iterate_end";
        break;
    case IG_SHA2_256:
        s = "sha2-256";
        break;
    case IG_SHA2_512:
        s = "sha2-512";
        break;
    case IG_SHA3_256:
        s = "sha3-256";
        break;
    case IG_SHA3_512:
        s = "sha3-512";
        break;
    case IG_BLAKE2S:
        s = "blake2s";
        break;
    case IG_BLAKE2B:
        s = "blake2b";
        break;
    case IG_SHAKE128:
        s = "shake128";
        break;
    case IG_SHAKE256:
        s = "shake256";
        break;
    default:
        return 1;
    }

    if ((size_t)fprintf(st, "%s", s) != strlen(s)){
        return 1;
    }
    else{
        return 0;
    }
}

#ifdef DEBUG
static int print_igloo_arg(enum igloo_cmd_type cmd_t, const union igloo_arg arg,
    enum igloo_arg_type arg_t)
{
    int err = 0;
    const char *s;

    switch (arg_t){
    case INT:
        s = "INT    ";
        break;
    case SIZE_T:
        s = "SIZE_T ";
        break;
    case INDEX:
        s = "INDEX  ";
        break;
    case PERCENT:
        s = "PERCENT";
        break;
    case STRING:
        s = "STRING ";
        break;
    case MIXED:
        s = "MIXED  ";
        break;
    case NONE:
        s = "NONE   ";
        break;
    default:
        return 1;
    }

    if ((size_t)fprintf(stderr, "%10s ", s) != 11){
        err = 1;
    }
    else if (arg_t == STRING &&
            (size_t)fprintf(stderr, "\n%12s ", "str:") != 14){
        err = 1;
    }
    else if (encode_arg(stderr, cmd_t, arg, arg_t) != 0){
        err = 1;
    }
    else if(arg_t == STRING &&
            (size_t)fprintf(stderr, "\n%12s %zu", "len:", arg.istr->len) <= 14){
        err = 1;
    }
    else if (fputc('\n', stderr) == EOF){
        err = 1;
    }

    return err;
}

int print_igloo_cmd(const igloo_cmd *icmd)
{
    int err = 0;
    const char *s;

    switch (icmd->cmd_t){
    case CAT:
        s = "CAT";
        break;
    case SALT:
        s = "SALT";
        break;
    case SUB:
        s = "SUB";
        break;
    case CUT:
        s = "CUT";
        break;
    case REV:
        s = "REV";
        break;
    case XOR:
        s = "XOR";
        break;
    case REPLACE:
        s = "REPLACE";
        break;
    case SHUFFLE:
        s = "SHUFFLE";
        break;
    case CCIPHER:
        s = "CCIPHER";
        break;
    case RCIPHER:
        s = "RCIPHER";
        break;
    case COMPLEMENT:
        s = "COMPLEMENT";
        break;
    case GENXOR:
        s = "GENXOR";
        break;
    case PASSWORDIFY:
        s = "PASSWORDIFY";
        break;
    case FORK:
        s = "FORK";
        break;
    case SUBSTR:
        s = "SUBSTR";
        break;
    case JOIN:
        s = "JOIN";
        break;
    case ITERATE:
        s = "ITERATE";
        break;
    case ITERATE_END:
        s = "ITERATE_END";
        break;
    case IG_SHA2_256:
        s = "SHA2_256";
        break;
    case IG_SHA2_512:
        s = "SHA2_512";
        break;
    case IG_SHA3_256:
        s = "SHA3_256";
        break;
    case IG_SHA3_512:
        s = "SHA3_512";
        break;
    case IG_BLAKE2S:
        s = "BLAKE2S";
        break;
    case IG_BLAKE2B:
        s = "BLAKE2B";
        break;
    case IG_SHAKE128:
        s = "SHAKE128";
        break;
    case IG_SHAKE256:
        s = "SHAKE256";
        break;
    default:
        return 1;
    }

    if ((size_t)fprintf(stderr, "%s ", s) != (strlen(s) + 1) ||
            igloo_cmd_encode(stderr, icmd) != 0){
        err = 1;
    }
    if (err == 0 &&
            print_igloo_arg(icmd->cmd_t, icmd->arg1, icmd->arg1_t) != 0){
        err = 1;
    }
    if (err == 0 &&
            print_igloo_arg(icmd->cmd_t, icmd->arg2, icmd->arg2_t) != 0){
        err = 1;
    }
    if (err == 0 &&
            print_igloo_arg(icmd->cmd_t, icmd->arg3, icmd->arg3_t) != 0){
        err = 1;
    }

    return err;
}
#endif /* DEBUG */

int igloo_cmd_encode(FILE *st, const igloo_cmd *icmd)
{
    int err = 0;

    if (encode_cmd(st, icmd) != 0){
        err = 1;
    }
    else if (icmd->arg1_t != NONE && (fputc(' ', st) == EOF ||
            encode_arg(st, icmd->cmd_t, icmd->arg1, icmd->arg1_t) != 0)){
        err = 1;
    }
    else if (icmd->arg2_t != NONE && (fputc(' ', st) == EOF ||
            encode_arg(st, icmd->cmd_t, icmd->arg2, icmd->arg2_t) != 0)){
        err = 1;
    }
    else if (icmd->arg3_t != NONE && (fputc(' ', st) == EOF ||
            encode_arg(st, icmd->cmd_t, icmd->arg3, icmd->arg3_t) != 0)){
        err = 1;
    }
    else if (fputc('\n', st) == EOF){
        err = 1;
    }

    return err;
}

int igloo_cmds_init(igloo_cmds* icmds)
{
    int err = 0;

    icmds->cmds = malloc(sizeof(igloo_cmd) * IGLOO_CMDS_INIT_SIZE);
    err = icmds->cmds == NULL;
    if (err == 0){
        icmds->size = IGLOO_CMDS_INIT_SIZE;
        icmds->used = 0;

#ifdef DEBUG
        fprintf(stderr, "\nInitialized icmds:\n");
#endif
    }

    return err;
}

int igloo_cmds_add(igloo_cmds *icmds, const igloo_cmd *icmd)
{
    int err = 0;
    igloo_cmd *new_icmd;

    if (icmds->used >= icmds->size && igloo_cmds_grow(icmds) != 0){
        err = 1;
    }
    else{
        new_icmd = icmds->cmds + icmds->used;
        memcpy(new_icmd, icmd, sizeof(igloo_cmd));
        icmds->used++;

#ifdef DEBUG
        fprintf(stderr, "Added icmd:\n");
        print_igloo_cmd(new_icmd);
#endif
    }

    return err;
}

int igloo_cmds_grow(igloo_cmds *icmds)
{
    int err = 0;
    igloo_cmd *new_cmds;
    size_t new_size = icmds->size * 2;

    new_cmds = reallocarray(icmds->cmds, new_size, sizeof(igloo_cmd));
    err = new_cmds == NULL;
    if (err == 0){
        icmds->cmds = new_cmds;
        icmds->size = new_size;

#ifdef DEBUG
        fprintf(stderr, "grew icmds (%zu)\n\n", new_size);
#endif
    }

    return err;
}

void igloo_cmds_free(igloo_cmds *icmds)
{
    igloo_cmd *index = icmds->cmds;
    size_t used = icmds->used;

    if (icmds == NULL){
        return;
    }

    for(; used-- > 0; index++){
        if (index->arg1_t == STRING){
            igloo_str_free(index->arg1.istr);
        }
        if (index->arg2_t == STRING){
            igloo_str_free(index->arg2.istr);
        }
        if (index->arg3_t == STRING){
            igloo_str_free(index->arg3.istr);
        }
    }

    free(icmds->cmds);
    explicit_bzero(icmds, sizeof(igloo_cmds));
}

#ifdef DEBUG
static int print_igloo_str_stack(const igloo_str_stack *istack)
{
    int err = 0;
    igloo_str **index;
    size_t used;
    size_t ind;

    index = istack->stack + istack->used - 1;
    used = istack->used;
    ind = 0;
    while (used-- > 0){
        if (fprintf(stderr, "%zu: ", ind++) <= 0 ||
                encode_str(stderr, *index--, ON) != 0){
            err = 1;
            break;
        }
        fprintf(stderr, "\n");
    }

    return err;
}
#endif /* DEBUG */

int igloo_str_stack_init(igloo_str_stack *istack, igloo_str *istr)
{
    int err = 0;

    istack->stack = malloc(sizeof(igloo_str *) * IGLOO_STR_STACK_INIT_SIZE);
    err = istack->stack == NULL;
    if (err == 0){
        *istack->stack = istr;
        istack->size = IGLOO_STR_STACK_INIT_SIZE;
        istack->used = 1;

#ifdef DEBUG
        fprintf(stderr, "\nInitialized istack:\n");
        print_igloo_str_stack(istack);
        fprintf(stderr, "\n");
#endif
    }

    return err;
}

igloo_str * igloo_str_stack_dup(igloo_str_stack *istack, size_t ind1, size_t ind2)
{
    int err = 0;
    igloo_str *istr_top;
    igloo_str *istr_new = NULL;

    if (istack->used >= istack->size && igloo_str_stack_grow(istack) != 0){
        err = 1;
    }
    if (err == 0){
        istr_top = *(istack->stack + istack->used - 1);
        istr_new = igloo_str_create(istr_top->str + ind1, ind2 - ind1);
        err = istr_new == NULL;
    }
    if (err == 0){
        *(istack->stack + istack->used) = istr_new;
        istack->used++;

#ifdef DEBUG
        fprintf(stderr, "\nDuplicated top of istack (%zu, %zu):\n", ind1, ind2);
        print_igloo_str_stack(istack);
#endif
    }

    return istr_new;
}

igloo_str * igloo_str_stack_pop(igloo_str_stack *istack)
{
    igloo_str *istr = NULL;

    if (istack->used > 1){
        istack->used--;
        istr = *(istack->stack + istack->used);
    }

#ifdef DEBUG
    fprintf(stderr, "\npopped istack:\n");
    if (istr != NULL){
        encode_str(stderr, istr, ON);
    }
    else{
        fprintf(stderr, "NULL");
    }
    fprintf(stderr, "\n\n");
#endif

    return istr;
}

igloo_str * igloo_str_stack_peek(igloo_str_stack *istack)
{
    igloo_str *istr = NULL;

    if (istack->used > 0){
        istr = *(istack->stack + istack->used - 1);
    }

#ifdef DEBUG
        fprintf(stderr, "\npeeked istack:\n");
        if (istr != NULL){
            encode_str(stderr, istr, ON);
        }
        else{
            fprintf(stderr, "NULL");
        }
        fprintf(stderr, "\n\n");
#endif

    return istr;
}

int igloo_str_stack_grow(igloo_str_stack *istack)
{
    int err = 0;
    igloo_str **new_stack;
    size_t new_size = istack->size * 2;

    new_stack = reallocarray(istack->stack, new_size, sizeof(igloo_str *));
    err = new_stack == NULL;
    if (err == 0){
        istack->stack = new_stack;
        istack->size = new_size;

#ifdef DEBUG
        fprintf(stderr, "grew istack (%zu)\n\n", new_size);
#endif
    }

    return err;
}

int igloo_str_stack_free(igloo_str_stack *istack)
{
    int err = 0;
    igloo_str **index;

    if (istack == NULL){
        return 0;
    }

    if (istack->used != 1){
        err = 1;
    }

    if(istack->used > 0){
        istack->used--;

        index = istack->stack + 1;
        while (istack->used-- > 0){
            igloo_str_free(*index++);
        }
    }

    free(istack->stack);
    explicit_bzero(istack, sizeof(igloo_str_stack));

    return err;
}

int igloo_cmd_state_init(igloo_cmd_state *icmd_state)
{
    int err = 0;

    icmd_state->state = malloc(sizeof(struct igloo_cmd_state_node) * IGLOO_CMD_STATE_INIT_SIZE);
    err = icmd_state->state == NULL;
    if (err == 0){
        icmd_state->size = IGLOO_CMD_STATE_INIT_SIZE;
        icmd_state->used = 0;

#ifdef DEBUG
        fprintf(stderr, "\nInitialized icmd_state:\n");
#endif
    }

    return err;
}

int igloo_cmd_state_push(igloo_cmd_state *icmd_state, igloo_cmd *icmd,
    size_t cmd_ind, size_t iteration_count)
{
    int err = 0;
    struct igloo_cmd_state_node *node;

    if (icmd_state->used >= icmd_state->size &&
            igloo_cmd_state_grow(icmd_state) != 0){
        err = 1;
    }
    else if(iteration_count == 0){
        err = 1;
    }
    else{
        node = icmd_state->state + icmd_state->used;
        node->icmd = icmd;
        node->iteration_count = iteration_count;
        node->cmd_ind = cmd_ind;
        icmd_state->used++;

#ifdef DEBUG
        fprintf(stderr, "Pushed icmd to state:\n");
        fprintf(stderr, "iter: %zu, cmd_ind: %zu\n", iteration_count, cmd_ind);
#endif
    }

    return err;
}

int igloo_cmd_state_restore(igloo_cmd_state *icmd_state, igloo_cmd **icmd,
    size_t *cmd_ind)
{
    int err = 0;
    struct igloo_cmd_state_node *node;

    if (icmd_state->used == 0){
        err = 1;
    }
    else{
        node = icmd_state->state + icmd_state->used - 1;
        /* note: guaranteed to not be zero because of *_state_push() */
        node->iteration_count--;
        if (node->iteration_count > 0){
            *icmd = node->icmd;
            *cmd_ind = node->cmd_ind;
        }
        else{
            icmd_state->used--;
        }

#ifdef DEBUG
        if (node->iteration_count > 0){
            fprintf(stderr, "\nrestored icmd state:\n");
            fprintf(stderr, "icmd %p, cmd_ind %zu\n", node->icmd, node->cmd_ind);
            fprintf(stderr, "iterations left: %zu\n\n", node->iteration_count);
        }
        else{
            fprintf(stderr, "\niterations complete\n\n");
        }
#endif
    }

    return err;
}

int igloo_cmd_state_grow(igloo_cmd_state *icmd_state)
{
    int err = 0;
    struct igloo_cmd_state_node *new_nodes;
    size_t new_size = icmd_state->size * 2;

    new_nodes = reallocarray(icmd_state->state, new_size, sizeof(struct igloo_cmd_state_node));
    err = new_nodes == NULL;
    if (err == 0){
        icmd_state->state = new_nodes;
        icmd_state->size = new_size;

#ifdef DEBUG
        fprintf(stderr, "grew cmds_state (%zu)\n\n", new_size);
#endif
    }

    return err;
}

int igloo_cmd_state_free(igloo_cmd_state *icmd_state){
    int err = 0;

    if (icmd_state == NULL){
        return 0;
    }

    if (icmd_state->used != 0){
        err = 1;
    }

    free(icmd_state->state);
    explicit_bzero(icmd_state, sizeof(igloo_cmd_state));

    return err;
}
