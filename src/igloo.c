#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <argp.h>

#include "igloo.h"
#include "igloo_internal.h"
#include "igloo_parser.h"
#include "igloo_befuddle.h"

#define PRINT_ONE_COL(str1_type, str1_args) \
    printf("%-15s%-65s\n", str1_type, str1_args)
#define PRINT_TWO_COL(str1_type, str1_args, str2_type, str2_args) \
    printf("%-15s%-25s%-15s%-25s\n", str1_type, str1_args, str2_type, str2_args)
static void list_befuddle(void)
{
    printf("%-80s\n", "standard befuddles:");
    PRINT_TWO_COL("cat", "MIXED STRING", "salt", "MIXED");
    PRINT_TWO_COL("sub", "STRING STRING SIZE_T", "cut", "SIZE_T");
    PRINT_TWO_COL("rev", "NONE", "xor", "STRING");
    PRINT_TWO_COL("replace", "STRING SIZE_T", "shuffle", "INT");
    PRINT_TWO_COL("bshuffle", "INT INT", "ccipher", "INT");
    PRINT_TWO_COL("rcipher", "INT", "complement", "NONE");
    PRINT_TWO_COL("genxor", "SIZE_T", "passwordify", "SIZE_T STRING");
    printf("\n%-80s\n", "meta befuddles:");
    PRINT_TWO_COL("fork", "NONE", "substr", "SIZE_T SIZE_T");
    PRINT_ONE_COL("join", "NONE");
    printf("\n%-80s\n", "join befuddles:");
    PRINT_TWO_COL("cat", "MIXED COMMENT", "sub", "STRING COMMENT SIZE_T");
    PRINT_TWO_COL("xor", "COMMENT", "replace", "COMMENT SIZE_T");
    printf("\n%-80s\n", "wolfssl befuddles:");
    PRINT_TWO_COL("sha2-256", "NONE", "sha2-512", "NONE");
    PRINT_TWO_COL("sha3-256", "NONE", "sha3-512", "NONE");
    PRINT_TWO_COL("blake2b", "NONE", "blake2s", "NONE");
}

static int parse_opt(int key, char *arg, struct argp_state *state)
{
    int err = 0;
    igloo *ig;
    char *endptr;

    ig = state->input;
    switch (key){
    case 's':
        ig->profile = state->argv[state->next - 1];
        break;

    case 'o':
        ig->output = state->argv[state->next - 1];
        break;

    case 'l':
        if (igloo_load(ig, state->argv[state->next - 1]) != 0){
            err = 1;
            QUIET_PRINT_ERR(ig, "unable to load igloo profile");
        }
        break;

    case LIST_CONST:
        list_befuddle();
        err = -1;
        break;

    case 'p':
        ig->pwd_amt = strtoul(arg, &endptr, 10);
        if (arg == endptr){
            err = 1;
            QUIET_PRINT_ERR(ig, "unable to convert password amount");
        }
        break;

    case 'q':
        ig->quiet = ON;
        break;

    case 't':
        ig->truncate = strtoul(arg, &endptr, 10);
        if (arg == endptr){
            err = 1;
            QUIET_PRINT_ERR(ig, "unable to set truncate length");
        }
        break;

    case ARGP_KEY_ARG:
        if (igloo_parse_commands(&ig->cmds, arg) != 0){
            err = 1;
            QUIET_PRINT_ERR(ig, "unable to parse igloo commands");
        }
        break;

    case ARGP_KEY_END:
        if(ig->profile != NULL && igloo_save(ig) != 0){
            err = 1;
            QUIET_PRINT_ERR(ig, "unable to save profile");
        }

        if (err == 0 && ig->cmds.used == 0){
            QUIET_PRINT(ig, "no commands parsed, loading default profile\n");
            if (igloo_load(ig, "default") != 0){
                err = 1;
                QUIET_PRINT_ERR(ig, "unable to load default profile");
            }
        }
        break;
    }

    return err;
}

int main(int argc, char *argv[])
{
    int err = 0;
    igloo ig;
    FILE *st = stdout;
    char *pwd_output = NULL;
    size_t output_len;
    size_t pwd_num;

    struct argp_option options[] =
    {
        {"load", 'l', "PATH", 0, "Load command(s) from a file", 0},
        {"list", LIST_CONST, NULL, 0, "List befuddle/COMMAND information", 0},
        {"output", 'o', "PATH", 0, "File to save obscured password(s) to\n"
            "Multiple passwords will be saved in multiple files (PATH1, PATH2,"
            " ...)", 0},
        {NULL, 'p', "NUM", 0, "Number of passwords to obscure (default is 1)", 0},
        {"quiet", 'q', NULL, 0, "Suppress printing of error and warning messages", 0},
        {"save", 's', "PATH", 0, "Save command(s) to a file", 0},
        {"truncate", 't', "NUM", 0, "Truncate length of output password(s)", 0},
        {0}
    };
    const char *args_doc = "COMMANDS";
    const char *doc = "Obfuscate password(s) using COMMANDS\v"
        "Example 1: igloo \"cat front abc\" \"rev\"\n"
        "  Prefix password with \"abc\" then reverse the order of it's "
        "characters\n"
        "Example 2: igloo -l my_profile -t 16\n"
        "  Load commands from ~/.igloo/my_profile and truncate the obscured "
        "password to 16 characters\n";
    struct argp argp = {options, parse_opt, args_doc, doc, NULL, NULL, NULL};

    argp_program_version = "igloo " PACKAGE_VERSION;

    if (igloo_init(&ig) == 0){
        if ((err = argp_parse(&argp, argc, argv, ARGP_IN_ORDER, 0, &ig)) != 0){
            /* err == -1 when --list is used */
            if (err == 1){
                QUIET_PRINT_ERR(&ig, "parsing error");
            }
        }
        else if (ig.output != NULL &&
                (pwd_output = igloo_prepare_output(&ig, &output_len)) == NULL){
            err = 1;
            QUIET_PRINT_ERR(&ig, "unable to prepare space for output buffer");
        }

        pwd_num = 1;
        while (err == 0){
            if (pwd_output != NULL && ig.pwd_amt != 1){
                err = snprintf(pwd_output, output_len, "%s%zu", ig.output, pwd_num);
                err = (size_t)err >= output_len || err <= 0;
            }
            if (err == 0 && pwd_output != NULL){
                st = fopen(pwd_output, "w");
                err = st == NULL;
            }
            if (err == 1){
                QUIET_PRINT_ERR(&ig, "unable to open output file");
            }

            QUIET_PRINT(&ig, "enter password: ");
            if (err == 0 && igloo_parse_stdin(&ig.pwd, ig.quiet) != 0){
                err = 1;
                QUIET_PRINT_ERR(&ig, "unable to parse password");
            }
            if (err == 0 && igloo_befuddle(&ig) != 0){
                err = 1;
                QUIET_PRINT_ERR(&ig, "befuddling error");
            }
            if (err == 0 && igloo_print_password(st, &ig) != 0){
                err = 1;
                QUIET_PRINT_ERR(&ig, "unable to print obscured password");
            }

            if (st != stdout && st != NULL){
                fclose(st);
                st = NULL;
            }
            if (err == 0){
                if (ig.pwd_amt != NO_LIMIT && pwd_num >= ig.pwd_amt){
                    break;
                }
                else{
                    explicit_bzero(ig.pwd.str, sizeof(unsigned char) * ig.pwd.len);
                    pwd_num++;
                }
            }
        }

        free(pwd_output);
        igloo_free(&ig);
    }
    else{
        err = 1;
        QUIET_PRINT_ERR(&ig, "unable to initialize igloo");
    }

    if (err == -1){
        err = 0;
    }
    return err;
}

int igloo_init(igloo *ig)
{
    int err = 0;

    if (igloo_cmds_init(&ig->cmds) != 0){
        err = 1;
    }
    else{
        ig->profile = NULL;
        ig->pwd.str = NULL;
        ig->pwd.len = 0;
        ig->pwd_amt = 1;
        ig->truncate = NO_LIMIT;
        ig->output = NULL;
        ig->quiet = OFF;

        err = 0;
    }

    return err;
}

char * igloo_prepare_output(const igloo *ig, size_t *output_len)
{
    int err = 0;
    char *output;

    /* +20 because 64 bit integers can be represented by 20 characters
     * (assuming 64 bit integers) */
    *output_len = strlen(ig->output) + 21;
    output = malloc(sizeof(char) * *output_len);
    err = output == NULL;

    if (err == 0 && ig->pwd_amt == 1){
        err = snprintf(output, *output_len, "%s", ig->output);
        err = (size_t)err >= *output_len || err <= 0;
    }

    if (err == 0){
        return output;
    }
    else{
        free(output);
        *output_len = 0;
        return NULL;
    }
}

int igloo_print_password(FILE *st, const igloo *ig)
{
    int err = 0;
    const unsigned char *str;
    size_t len;
    enum flag bad_char;

    bad_char = OFF;
    str = ig->pwd.str;
    len = ig->pwd.len;

    if (ig->truncate != NO_LIMIT && len > ig->truncate){
        len = ig->truncate;
    }

    while (len-- > 0){
        if (*str < 32 || *str > 126){
            bad_char = ON;
        }
        if (fputc(*str++, st) == EOF){
            err = 1;
            break;
        }
    }
    if (err == 0 && st == stdout && fputc('\n', st) == EOF){
        err = 1;
    }

    if (err == 0 && st == stdout && bad_char == ON){
        QUIET_PRINT(ig, "unprintable character(s) detected in obscured"
                        " password\n");
    }

    return err;
}

FILE * open_profile(const igloo *ig, const char *profile_loc, const char *type)
{
    char *profile_adj;
    char *home = NULL;
    char *end;
    FILE *profile;
    size_t len;

    if (profile_loc == NULL || (len = strlen(profile_loc)) == 0){
        return NULL;
    }
    else if (*profile_loc == '/'){
        len++;
    }
    else{
        if ((home = getenv("HOME")) == NULL){
            return NULL;
        }
        len += strlen(home) + sizeof(PROFILE_DIR);
    }

    if ((profile_adj = malloc(sizeof(char) * len)) == NULL){
        return NULL;
    }
    else if (*profile_loc == '/'){
        stpcpy(profile_adj, profile_loc);
    }
    else{
        end = stpcpy(profile_adj, home);
        end = stpcpy(end, PROFILE_DIR);
        stpcpy(end, profile_loc);
    }

    if ((profile = fopen(profile_adj, type)) == NULL && ig->quiet == OFF){
        printf("could not open: %s\n", profile_adj);
    }

    free(profile_adj);
    return profile;
}

int igloo_load(igloo *ig, const char *profile_input)
{
    int err = 0;
    int ret;
    FILE *profile;
    char *buffer = NULL;
    size_t len = 0;

    if ((profile = open_profile(ig, profile_input, "r")) == NULL){
        err = 1;
    }
    else{
        while (1){
            ret = igloo_dynamic_read(profile, &buffer, &len);
            if (ret == -1){
                err = 0;
                break;
            }
            else if (ret != 0 || igloo_parse_commands(&ig->cmds, buffer) != 0){
                err = 1;
                break;
            }
        }

        fclose(profile);
        explicit_bzero(buffer, sizeof(char) * len);
        free(buffer);
    }

    return err;
}

int igloo_save(const igloo *ig)
{
    int err = 0;
    const char *profile_output;
    FILE *profile;
    size_t line = 0;

    profile_output = ig->profile;
    if ((profile = open_profile(ig, profile_output, "w")) == NULL){
        err = 1;
    }
    else{
        while (line < ig->cmds.used){
            if (igloo_cmd_encode(profile, ig->cmds.cmds + line) != 0){
                err = 1;
                break;
            }
            line++;
        }

        fclose(profile);
    }

    return err;
}

void igloo_free(igloo *ig)
{
    igloo_cmds_free(&ig->cmds);
    explicit_bzero(ig->pwd.str, sizeof(unsigned char) * ig->pwd.len);
    free(ig->pwd.str);
    ig->pwd.len = 0;
    ig->truncate = 0;
}
