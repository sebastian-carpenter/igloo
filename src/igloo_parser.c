#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#include "igloo_parser.h"
#include "igloo_internal.h"

// process an escape sequence in str, moving the processed value into the first
//  slot in index
// all characters can be escaped with backslash except x, the special sequence
//  '\xFF' can be used to escape bytes in the range 0-255
// return 0 on failure, size of escape sequence on success ('\xFF'->4, '\~'->2)
static int escape_char(char *index, const char *str)
{
    int ret;
    char tmp[3];

    switch (*++str){
    case 'X':
    case 'x':
        char *endptr;

        if (*++str == '\0' || str[1] == '\0'){
            ret = -1;
        }
        else{
            /* only interpret two hex characters */
            tmp[0] = *str++;
            tmp[1] = *str;
            tmp[2] = '\0';

            *index = strtol(tmp, &endptr, 16);
            if (tmp == endptr){
                ret = -1;
            }
            else{
                ret = 4;
            }
        }
        break;

    case '\0':
        ret = -1;
        break;

    default:
        *index = *str;
        ret = 2;
        break;
    }

    return ret;
}

#define SKIP_CHARACTERS_COMMAND(strs) \
    for (; (*strs <= 32 || *strs > 126 ) && *strs != '\0'; strs++)
#define SKIP_CHARACTERS_PASSWORD(strs) \
    for (; (*strs < 32 || *strs > 126 ) && *strs != '\0'; strs++)

int igloo_parse_commands(igloo_cmds *icmds, const char *strs)
{
    int err = 0;
    char *str = NULL;
    char *stri = NULL;
    int ret;
    igloo_cmd icmd;
    int arg_amt;
    int arg_num = 0;

#ifdef DEBUG
    fprintf(stderr, "\nparse_igloo_commands input: %s\n", strs);
#endif

    if ((ret = strlen(strs)) == 0){
        err = 1;
    }
    else{
        str = malloc(sizeof(char) * (ret + 1));
        stri = str;
        err = str == NULL;
    }

    if (err == 0){
        SKIP_CHARACTERS_COMMAND(strs);
    }

    while (err == 0 && *strs != '\0'){
        switch (*strs){
        case '\\':
            if ((ret = escape_char(stri++, strs)) == -1){
                err = 1;
            }
            else{
                strs += ret;
            }
            break;

        case '\n':
        case '\r':
        case ',':
        case ' ':
            *stri = '\0';
            if (arg_num == 0){
                if ((arg_amt = igloo_cmd_map_type(&icmd, str,
                                                        stri - str)) == -1){
                    err = 1;
                }
                /* a space implies more arguments which would be an error
                 * since this is a zero arg command */
                else if (arg_amt == 0 && *strs == ' '){
                    err = 1;
                }
            }
            else{
                if (igloo_cmd_add_arg(&icmd, arg_num, str, stri - str) != 0){
                    err = 1;
                }
            }

            arg_num++;
            if (err == 0 && arg_num > arg_amt){
                arg_num = 0;
                err = igloo_cmds_add(icmds, &icmd);
            }

            if (err == 0){
                strs++;
                stri = str;
                SKIP_CHARACTERS_COMMAND(strs);
            }

            break;

        default:
            *stri++ = *strs++;
            break;
        }
    }

    if (stri - str > 0 && err == 0){
        *stri = '\0';
        if (arg_num == 0){
            err = (arg_amt = igloo_cmd_map_type(&icmd, str, stri - str)) == -1;
            err |= arg_amt != 0;
        }
        else if (arg_num == arg_amt){
            err = igloo_cmd_add_arg(&icmd, arg_num, str, stri - str);
        }
        else{
            err = 1;
        }
    }

    if (stri - str > 0 && err == 0){
        err = igloo_cmds_add(icmds, &icmd);
    }

    explicit_bzero(str, sizeof(char) * (strlen(strs) + 1));
    free(str);

    return err;
}

static int disable_echo(struct termios *restore)
{
    int err = 0;
    int fd;
    struct termios altered;

    fd = fileno(stdin);
    if (fd == -1){
        err = 1;
    }
    else if (tcgetattr(fd, restore) != 0){
        err = 1;
    }
    else{
        altered = *restore;

        altered.c_lflag &= ~ECHO;
        altered.c_lflag |= ECHONL;
        err = tcsetattr(fd, TCSANOW, &altered) != 0;
    }

    return err;
}

static int restore_echo(struct termios *restore)
{
    int err = 0;
    int fd;

    fd = fileno(stdin);
    if (fd == -1){
        err = 1;
    }
    else if (tcsetattr(fd, TCSANOW, restore) != 0){
        err = 1;
    }

    return err;
}

static int secure_read(char **str, size_t *len, enum flag quiet)
{
    int err = 0;
    struct termios restore;

    if (quiet == OFF && disable_echo(&restore) != 0){
        err = 1;
    }
    else if (igloo_dynamic_read(stdin, str, len) != 0){
        err = 1;
    }
    else if (quiet == OFF && restore_echo(&restore) != 0){
        err = 1;
    }

    if (err == 1){
        explicit_bzero(*str, sizeof(char) * *len);
        free(*str);
        *str = NULL;
    }
#ifdef DEBUG
    else{
        fprintf(stderr, "\nsecure_read read: %s\n", *str);
    }
#endif


    return err;
}

int igloo_parse_stdin(igloo_str *istr, enum flag quiet)
{
    int err = 0;
    size_t len;
    int ret;
    char *strs;
    char *str;
    char *stri = NULL;

    str = (char *)istr->str;
    len = istr->len;

    if (secure_read(&str, &len, quiet) != 0){
        err = 1;
    }
    else{
        stri = strs = str;
        SKIP_CHARACTERS_PASSWORD(strs);
    }

    while (err == 0 && *strs != '\0' && *strs != '\n'){
        switch (*strs){
        case '\\':
            if ((ret = escape_char(stri++, strs)) == -1){
                err = 1;
            }
            else{
                strs += ret;
            }
            break;

        default:
            *stri++ = *strs++;
            break;
        }
    }

    if (stri - str <= 0){
        err = 1;
    }
    else if (err == 0){
        istr->str = (unsigned char *)str;
        istr->len = stri - str;
    }
    else if (err == 1){
        explicit_bzero(istr->str, sizeof(unsigned char) * istr->len);
        free(istr->str);
        istr->str = NULL;
        istr->len = 0;
    }

    return err;
}
