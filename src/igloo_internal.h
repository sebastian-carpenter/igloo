#ifndef IGLOO_INTERNAL_H
#define IGLOO_INTERNAL_H

#include <stdio.h>

#include "igloo_types.h"

#define QUIET_PRINT(ig, str)  \
    if ((ig)->quiet == OFF){  \
        fprintf(stderr, str); \
    }

#define QUIET_PRINT_ERR(ig, str) QUIET_PRINT(ig, "ERR: "str"\n")

/* begin receiving input into a provided str from some FILE and allocate more
 *  space as neccesary to hold one line of input
 *  if *str is NULL an array will be allocated
 * return 0 on success, -1 on EOF/error, 1 on error */
#define BUFFER_INIT_SIZE 1024
int igloo_dynamic_read(FILE *st, char **str, size_t *len);

/* data structure to hold arbitrary data
 * return NULL on failure, igloo_str * on success */
igloo_str * igloo_str_create(const unsigned char *str, size_t len);
void igloo_str_free(igloo_str *istr);

/* initialize cmd based on str
 * returns -1 on error, otherwise number of args in command */
int igloo_cmd_map_type(igloo_cmd *icmd, const char *str, size_t len);
/* returns 0 on success, 1 on error */
int igloo_cmd_add_arg(igloo_cmd *icmd, int arg_num, const char *str, size_t len);

#ifdef DEBUG
int print_igloo_cmd(const igloo_cmd *icmd);
#endif

/* output icmd to file in parseable format
 * return 0 on success, 1 on failure */
int igloo_cmd_encode(FILE *st, const igloo_cmd *icmd);

/* data structure to hold igloo_cmd
 * return 0 on success, 1 on failure */
int igloo_cmds_init(igloo_cmds *icmds);
int igloo_cmds_add(igloo_cmds *icmds, const igloo_cmd *icmd);
int igloo_cmds_grow(igloo_cmds *icmds);
void igloo_cmds_free(igloo_cmds *icmds);

/* data structure to help with forking/joining passwords in the befuddler
 * return 0 on success, 1 on failure */
int igloo_str_stack_init(igloo_str_stack *istack, igloo_str *istr);
igloo_str * igloo_str_stack_dup(igloo_str_stack *istack, size_t ind1, size_t ind2);
igloo_str * igloo_str_stack_pop(igloo_str_stack *istack);
igloo_str * igloo_str_stack_peek(igloo_str_stack *istack);
int igloo_str_stack_grow(igloo_str_stack *istack);
/* always free memory but return 1 if stack does not contain only 1 password */
int igloo_str_stack_free(igloo_str_stack *istack);

/* data structure to help with iteration in the befuddler
 * return 0 on success, 1 on failure */
int igloo_cmd_state_init(igloo_cmd_state *icmd_state);
int igloo_cmd_state_push(igloo_cmd_state *icmd_state, igloo_cmd *icmd,
    size_t cmd_ind, size_t iteration_count);
int igloo_cmd_state_restore(igloo_cmd_state *icmd_state, igloo_cmd **icmd,
    size_t *cmd_ind);
int igloo_cmd_state_grow(igloo_cmd_state *icmd_state);
/* always free memory but return 1 if state(s) have not been restored */
int igloo_cmd_state_free(igloo_cmd_state *icmd_state);

#endif
