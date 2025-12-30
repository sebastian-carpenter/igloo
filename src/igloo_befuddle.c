#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifndef DISABLE_WOLFSSL
#ifndef WOLFSSL_USER_SETTINGS
#include <wolfssl/options.h>
#endif
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/ssl.h>
#include <wolfssl/wolfcrypt/hash.h>
#include <wolfssl/wolfcrypt/blake2.h>
#include <wolfssl/wolfcrypt/error-crypt.h>
#endif /* !DISABLE_WOLFSSL */

#include "igloo_befuddle.h"
#include "igloo_internal.h"
#include "igloo_parser.h"
#include "igloo_random.h"

static void clear_free(void *ptr, size_t len)
{
    if (ptr != NULL){
        explicit_bzero(ptr, len);
        free(ptr);
    }
}

static int convert_percent(enum igloo_arg_type arg_t, size_t *sz, igloo_str *p)
{
    int err = 0;

    if (arg_t == PERCENT){
        if (p->len > ((size_t)-1) / 100){
            err = 1;
        }
        else{
            *sz = p->len * *sz / 100;
        }
    }

    return err;
}

static int convert_percents(igloo_cmd *icmd, igloo_str *p)
{
    int err = 0;
    err |= convert_percent(icmd->arg1_t, &icmd->arg1.sz, p);
    err |= convert_percent(icmd->arg2_t, &icmd->arg2.sz, p);
    err |= convert_percent(icmd->arg3_t, &icmd->arg3.sz, p);
    return err;
}

int igloo_befuddle(igloo *ig)
{
    int err = 0;
    igloo_str_stack istack;
    igloo_cmd_state icmd_state;
    igloo_str *pass;
    igloo_cmd *icmd;
    size_t cmd_ind = 0;

    pass = &ig->pwd;
    icmd = ig->cmds.cmds;

    err = igloo_str_stack_init(&istack, pass) != 0;
    if (err == 0){
        err = igloo_cmd_state_init(&icmd_state) != 0;
    }

    while(err == 0 && cmd_ind < ig->cmds.used){
        err = convert_percents(icmd, pass);
        if (err != 0){
            break;
        }

        switch (icmd->cmd_t){
        case CAT:
            err = bef_cat(pass, icmd->arg1.i, icmd->arg2.istr);
            break;
        case SALT:
            err = bef_salt(ig, pass, icmd->arg1.i);
            break;
        case SUB:
            err = bef_sub(pass, icmd->arg1.istr, icmd->arg2.istr, icmd->arg3.sz);
            break;
        case CUT:
            err = bef_cut(pass, icmd->arg1.sz);
            break;
        case REV:
            bef_rev(pass);
            break;
        case XOR:
            bef_xor(pass, icmd->arg1.istr);
            break;
        case REPLACE:
            err = bef_replace(pass, icmd->arg1.istr, icmd->arg2.sz);
            break;
        case SHUFFLE:
            err = bef_shuffle(pass, icmd->arg1.sz);
            break;
        case CCIPHER:
            bef_ccipher(pass, icmd->arg1.i);
            break;
        case RCIPHER:
            err = bef_rcipher(pass, icmd->arg1.sz);
            break;
        case COMPLEMENT:
            bef_complement(pass);
            break;
        case GENXOR:
            bef_genxor(pass, icmd->arg1.sz);
            break;
        case PASSWORDIFY:
            bef_passwordify(pass, icmd->arg1.sz, icmd->arg2.istr);
            break;
        case FORK:
            err = bef_fork(&istack, &pass);
            break;
        case SUBSTR:
            err = bef_substr(&istack, &pass, icmd->arg1.sz, icmd->arg2.sz);
            break;
        case JOIN:
            err = bef_join(&istack, &pass, icmd->arg1.istr);
            break;
        case ITERATE:
            err = igloo_cmd_state_push(&icmd_state, icmd, cmd_ind, icmd->arg1.sz);
            break;
        case ITERATE_END:
            err = igloo_cmd_state_restore(&icmd_state, &icmd, &cmd_ind);
            break;
        case IG_SHA2_256:
            err = bef_sha2_256(pass);
            break;
        case IG_SHA2_512:
            err = bef_sha2_512(pass);
            break;
        case IG_SHA3_256:
            err = bef_sha3_256(pass);
            break;
        case IG_SHA3_512:
            err = bef_sha3_512(pass);
            break;
        case IG_BLAKE2B:
            err = bef_blake2b(pass, icmd->arg1.sz);
            break;
        case IG_BLAKE2S:
            err = bef_blake2s(pass, icmd->arg1.sz);
            break;
        case IG_SHAKE128:
            err = bef_shake128(pass, icmd->arg1.sz);
            break;
        case IG_SHAKE256:
            err = bef_shake256(pass, icmd->arg1.sz);
            break;
        }

#ifdef DEBUG
        fprintf(stderr, "completed ");
        print_igloo_cmd(icmd);
#endif
        icmd++;
        cmd_ind++;
    }

    err |= igloo_str_stack_free(&istack) != 0;
    err |= igloo_cmd_state_free(&icmd_state) != 0;

    return err;
}

int bef_cat(igloo_str *p, int dir, const igloo_str *istr)
{
    int err = 0;
    unsigned char *buf;

    buf = malloc(sizeof(unsigned char) * (p->len + istr->len));
    if (buf == NULL){
        err = 1;
    }
    else{
        if (dir == CAT_FRONT){
            memcpy(buf, istr->str, istr->len);
            memcpy(buf + istr->len, p->str, p->len);
        }
        else if (dir == CAT_BACK){
            memcpy(buf, p->str, p->len);
            memcpy(buf + p->len, istr->str, istr->len);
        }
        else{
            clear_free(buf, sizeof(unsigned char) * (p->len + istr->len));
            err = 1;
        }

        if (err == 0){
            clear_free(p->str, sizeof(char) * p->len);
            p->str = buf;
            p->len += istr->len;
        }
    }

    return err;
}

int bef_salt(igloo *ig, igloo_str *p, int dir)
{
    int err = 0;
    igloo_str salt;

    salt.str = NULL;
    salt.len = 0;

    QUIET_PRINT(ig, "enter salt: ");
    err = igloo_parse_stdin(&salt, ig->quiet);
    if (err == 0){
        err = bef_cat(p, dir, &salt);
    }

    clear_free(salt.str, sizeof(unsigned char) * salt.len);

    return err;
}

int bef_sub(igloo_str *p, const igloo_str *old, const igloo_str *new, size_t x)
{
    int err;
    size_t len;
    size_t count = 0;
    unsigned char *stri;
    unsigned char *buf = NULL;
    unsigned char *bufi;

    stri = p->str;
    len = p->len;
    while (len >= old->len){
        if (memcmp(stri, old->str, old->len) == 0){
            stri += old->len;
            len -= old->len;
            count++;
        }
        else{
            stri++;
            len--;
        }
    }
    if (count < x || x == NO_LIMIT){
        x = count;
    }

    len = p->len + (new->len - old->len) * x;
    buf = malloc(sizeof(unsigned char) * len);
    err = buf == NULL;

    if (err == 0){
        stri = p->str;
        bufi = buf;
        while (x > 0){
            if (memcmp(stri, old->str, old->len) == 0){
                memcpy(bufi, new->str, new->len);
                bufi += new->len;
                stri += old->len;
                x--;
            }
            else{
                *bufi++ = *stri++;
            }
        }
        memcpy(bufi, stri, (p->str + p->len) - stri);

        clear_free(p->str, sizeof(unsigned char) * p->len);
        p->str = buf;
        p->len = len;
    }

    if (err != 0){
        clear_free(buf, sizeof(unsigned char) * len);
    }

    return err;
}

int bef_cut(igloo_str *p, size_t x)
{
    int err;
    unsigned char *tmp;

    err = x > p->len;
    if (err == 0){
        tmp = malloc(sizeof(unsigned char) * x);
        err = tmp == NULL;
    }
    if (err == 0){
        memcpy(tmp, p->str, x);
        memmove(p->str, p->str + x, p->len - x);
        memcpy(p->str + p->len - x, tmp, x);
        clear_free(tmp, sizeof(unsigned char) * x);
    }

    return err;
}

void bef_rev(igloo_str *p)
{
    unsigned char *start;
    unsigned char *end;
    unsigned char c;

    start = p->str;
    end = start + p->len - 1;
    while (end > start){
        c = *start;
        *start++ = *end;
        *end-- = c;
    }
}

void bef_xor(igloo_str *p, const igloo_str *istr)
{
    unsigned char *str1;
    unsigned char *str2;
    size_t len;

    str1 = p->str;
    str2 = istr->str;
    len = MIN(p->len, istr->len);

    for (; len-- > 0; *str1++ ^= *str2++);
}

int bef_replace(igloo_str *p, const igloo_str *istr, size_t x)
{
    int err;
    size_t len;
    unsigned char *buf = NULL;

    err = x > p->len;
    if (err == 0 && istr->len + x > p->len){
        len = istr->len + x;
        buf = malloc(sizeof(unsigned char) * len);
        err = buf == NULL;
    }

    if (err == 0 && buf != NULL){
        memcpy(buf, p->str, x);
        memcpy(buf + x, istr->str, istr->len);

        clear_free(p->str, sizeof(unsigned char) * p->len);
        p->str = buf;
        p->len = len;
    }
    else if (err == 0){
        memcpy(p->str + x, istr->str, istr->len);
    }

    return err;
}

int bef_shuffle(igloo_str *p, size_t x)
{
    int err = 0;
    size_t middle;
    unsigned char *p1;
    unsigned char *p2;
    unsigned char *p3;
    unsigned char *end;
    unsigned char *tmp = NULL;
    unsigned char *tmp_end;

    if (x == 0){
        return 0;
    }

    tmp = malloc(sizeof(unsigned char) * p->len);
    err = tmp == NULL;
    if (err == 0){
        end = p->str + p->len;
        tmp_end = tmp + p->len;
        middle = p->len / 2 + (p->len % 2);

        while (x-- > 0){
            memcpy(tmp, p->str, p->len);

            p1 = p->str;
            p2 = tmp;
            p3 = tmp + middle;
            while (p1 < end){
                *p1++ = *p2++;
                if (p3 < tmp_end){
                    *p1++ = *p3++;
                }
            }
        }
    }

    clear_free(tmp, sizeof(unsigned char) * p->len);

    return err;
}

void bef_ccipher(igloo_str *p, int x)
{
    size_t len;
    unsigned char *str;

    len = p->len;
    str = p->str;

    for (; len-- > 0; *str++ += x);
}

int bef_rcipher(igloo_str *p, size_t x)
{
    int err = 0;
    size_t len;
    unsigned char *str;
    unsigned char *end;
    unsigned char *buf;
    unsigned char *bufi;
    size_t rail;
    int dir;

    if (x == 0){
        return 1;
    }
    else if (x == 1){
        return 0;
    }

    len = p->len;
    str = p->str;
    end = p->str + p->len;

    buf = malloc(sizeof(unsigned char) * len);
    err = buf == NULL;

    if (err == 0){
        rail = 1;
        dir = 0;
        bufi = buf;
        while (len-- > 0){
            *bufi++ = *str;
            if (dir == 0 || rail == 1){
                str += (x - rail) * 2;
            }
            else{
                str += (rail - 1) * 2;
            }
            dir ^= 0x01;

            if (str >= end){
                str = p->str + rail;
                dir = 0;

                if (++rail == x){
                    rail = 1;
                }
            }
        }
    }
    if (err == 0){
        clear_free(p->str, sizeof(unsigned char) * p->len);
        p->str = buf;
    }

    return err;
}

void bef_complement(igloo_str *p)
{
    unsigned char *start;
    size_t len;

    start = p->str;
    len = p->len;

    for (; len-- > 0; *start = ~(*start), start++);
}

void bef_genxor(igloo_str *p, size_t seed)
{
    unsigned char *str;
    size_t len;
    struct xoshiro256pp_state xos256state;
    uint64_t t64 = 0;
    unsigned char t8;
    int loop4;

    str = p->str;
    len = p->len;

    if (seed == DYNAMIC_SEED){
        if (len >= 8){
            memcpy(&t64, str, 8);
        }
        else{
            memcpy(&t64, str, len);
        }
    }
    else{
        t64 = seed;
    }

    xoshiro256pp_init(&xos256state, t64);

    /* upper 32 bits have better entropy so ignore the bottom 32 */
    while (len > 0){
        t64 = xoshiro256pp(&xos256state);
        t64 >>= 32;
        for (loop4 = 4; loop4 > 0 && len > 0; loop4--, len--){
            t8 = t64 & 0xFF;
            *str++ ^= t8;
            t64 >>= 8;
        }
    }
}

static int is_bad_char(unsigned char *bad_chars, size_t len, unsigned char c)
{
    while (len-- > 0){
        if (*bad_chars++ == c){
            return 1;
        }
    }

    return 0;
}

void bef_passwordify(igloo_str *p, size_t seed, igloo_str *bad_chars)
{
    unsigned char *str;
    size_t len;
    struct xoshiro256pp_state xos256state;
    uint64_t t64 = 0;
    unsigned char t8;
    int loop4;

    str = p->str;
    len = p->len;

    if (seed == DYNAMIC_SEED){
        if (len >= 8){
            memcpy(&t64, str, 8);
        }
        else{
            memcpy(&t64, str, len);
        }
    }
    else{
        t64 = seed;
    }

    xoshiro256pp_init(&xos256state, t64);

    /* upper 32 bits have better entropy so ignore the bottom 32 */
    while (len > 0){
        t64 = xoshiro256pp(&xos256state);
        t64 >>= 32;
        for (loop4 = 4; loop4 > 0 && len > 0; loop4--){
            t8 = t64 & 0xFF;
            *str ^= t8;
            t64 >>= 8;

            if (*str >= 33 && *str <= 126 &&
                    !is_bad_char(bad_chars->str, bad_chars->len, *str)){
                str++;
                len--;
            }
        }
    }
}

int bef_fork(igloo_str_stack *istack, igloo_str **p)
{
    igloo_str *p_new = NULL;

    p_new = igloo_str_stack_dup(istack, 0, (*p)->len);
    if (p_new != NULL){
        *p = p_new;
    }

    return p_new == NULL;
}

int bef_substr(igloo_str_stack *istack, igloo_str **p, size_t ind1, size_t ind2)
{
    igloo_str *p_new = NULL;

    if (ind2 > ind1 && ind2 <= (*p)->len){
        p_new = igloo_str_stack_dup(istack, ind1, ind2);
    }
    if (p_new != NULL){
        *p = p_new;
    }

    return p_new == NULL;
}

int bef_join(igloo_str_stack *istack, igloo_str **p, igloo_str *operation)
{
    int err = 0;
    igloo_cmds icmds;
    igloo_cmd *icmd;
    char *str = NULL;
    igloo_str *fork = NULL;
    igloo_str *base;

    if (igloo_cmds_init(&icmds) != 0){
        err = 1;
    }
    else if ((str = malloc(sizeof(char) * (operation->len + 1))) == NULL){
        err = 1;
    }

    if (err == 0){
        memcpy(str, operation->str, operation->len);
        *(str + operation->len) = '\0';
        err = igloo_parse_commands(&icmds, str) != 0;
    }
    if (err == 0 && icmds.used == 1 &&
            (fork = igloo_str_stack_pop(istack)) != NULL &&
            (base = igloo_str_stack_peek(istack)) != NULL){
        icmd = icmds.cmds;
        if (icmd->cmd_t == CAT){
            err = bef_cat(base, icmd->arg1.i, fork);
        }
        else if(icmd->cmd_t == SUB){
            err = bef_sub(base, icmd->arg1.istr, fork, icmd->arg3.i);
        }
        else if (icmd->cmd_t == XOR){
            bef_xor(base, fork);
        }
        else if (icmd->cmd_t == REPLACE){
            err = bef_replace(base, fork, icmd->arg2.sz);
        }
        else{
            err = 1;
        }
    }
    else{
        err = 1;
    }

    igloo_str_free(fork);
    igloo_cmds_free(&icmds);
    clear_free(str, sizeof(char) * (operation->len + 1));

    if (base != NULL){
        *p = base;
    }

    return err;
}

int bef_sha2_256(igloo_str *p)
{
    int err = 1;
    (void)p;

#if !defined(DISABLE_WOLFSSL) && !defined(NO_SHA256)
    unsigned char *digest = NULL;

    digest = malloc(sizeof(unsigned char) * WC_SHA256_DIGEST_SIZE);
    err = digest == NULL;

    if (err == 0 && wc_Sha256Hash(p->str, p->len, digest) != 0){
        err = 1;
    }
    else if(err == 0){
        clear_free(p->str, sizeof(unsigned char) * p->len);
        p->str = digest;
        p->len = WC_SHA256_DIGEST_SIZE;
    }

    if (err != 0){
        clear_free(digest, sizeof(unsigned char) * WC_SHA256_DIGEST_SIZE);
    }
#endif

    return err;
}

int bef_sha2_512(igloo_str *p)
{
    int err = 1;
    (void)p;

#if !defined(DISABLE_WOLFSSL) && defined(WOLFSSL_SHA512)
    unsigned char *digest = NULL;

    digest = malloc(sizeof(unsigned char) * WC_SHA512_DIGEST_SIZE);
    err = digest == NULL;

    if (err == 0 && wc_Sha512Hash(p->str, p->len, digest) != 0){
        err = 1;
    }
    else if(err == 0){
        clear_free(p->str, sizeof(unsigned char) * p->len);
        p->str = digest;
        p->len = WC_SHA512_DIGEST_SIZE;
    }

    if (err != 0){
        clear_free(digest, sizeof(unsigned char) * WC_SHA512_DIGEST_SIZE);
    }
#endif

    return err;
}

int bef_sha3_256(igloo_str *p)
{
    int err = 1;
    (void)p;

#if !defined(DISABLE_WOLFSSL) && \
        defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_256)
    unsigned char *digest = NULL;

    digest = malloc(sizeof(unsigned char) * WC_SHA3_256_DIGEST_SIZE);
    err = digest == NULL;

    if (err == 0 && wc_Sha3_256Hash(p->str, p->len, digest) != 0){
        err = 1;
    }
    else if(err == 0){
        clear_free(p->str, sizeof(unsigned char) * p->len);
        p->str = digest;
        p->len = WC_SHA3_256_DIGEST_SIZE;
    }

    if (err != 0){
        clear_free(digest, sizeof(unsigned char) * WC_SHA3_256_DIGEST_SIZE);
    }
#endif

    return err;
}

int bef_sha3_512(igloo_str *p)
{
    int err = 1;
    (void)p;

#if !defined(DISABLE_WOLFSSL) && \
        defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_512)
    unsigned char *digest = NULL;

    digest = malloc(sizeof(unsigned char) * WC_SHA3_512_DIGEST_SIZE);
    err = digest == NULL;

    if (err == 0 && wc_Sha3_512Hash(p->str, p->len, digest) != 0){
        err = 1;
    }
    else if(err == 0){
        clear_free(p->str, sizeof(unsigned char) * p->len);
        p->str = digest;
        p->len = WC_SHA3_512_DIGEST_SIZE;
    }

    if (err != 0){
        clear_free(digest, sizeof(unsigned char) * WC_SHA3_512_DIGEST_SIZE);
    }
#endif

    return err;
}

int bef_blake2s(igloo_str *p, size_t digest_size)
{
    int err = 1;
    (void)p;
    (void)digest_size;

#if !defined(DISABLE_WOLFSSL) && defined(HAVE_BLAKE2S)
    Blake2s b2s;
    unsigned char *digest = NULL;

    if((digest = malloc(sizeof(unsigned char) * digest_size)) == NULL){
        err = 1;
    }
    else if (wc_InitBlake2s(&b2s, digest_size) != 0 ||
             wc_Blake2sUpdate(&b2s, p->str, p->len) != 0 ||
             wc_Blake2sFinal(&b2s, digest, 0) != 0){
        err = 1;
    }
    else{
        clear_free(p->str, sizeof(unsigned char) * p->len);
        p->str = digest;
        p->len = digest_size;
        err = 0;
    }

    if (err != 0){
        clear_free(digest, sizeof(unsigned char) * digest_size);
    }
#endif

    return err;
}

int bef_blake2b(igloo_str *p, size_t digest_size)
{
    int err = 1;
    (void)p;
    (void)digest_size;

#if !defined(DISABLE_WOLFSSL) && defined(HAVE_BLAKE2B)
    Blake2b b2b;
    unsigned char *digest = NULL;

    if((digest = malloc(sizeof(unsigned char) * digest_size)) == NULL){
        err = 1;
    }
    else if (wc_InitBlake2b(&b2b, digest_size) != 0 ||
             wc_Blake2bUpdate(&b2b, p->str, p->len) != 0 ||
             wc_Blake2bFinal(&b2b, digest, 0) != 0){
        err = 1;
    }
    else{
        clear_free(p->str, sizeof(unsigned char) * p->len);
        p->str = digest;
        p->len = digest_size;
        err = 0;
    }

    if (err != 0){
        clear_free(digest, sizeof(unsigned char) * digest_size);
    }
#endif

    return err;
}

int bef_shake128(igloo_str *p, size_t digest_size)
{
    int err = 1;
    (void)p;
    (void)digest_size;

#if !defined(DISABLE_WOLFSSL) && \
        defined(WOLFSSL_SHA3) && defined(WOLFSSL_SHAKE128)
    unsigned char *digest = NULL;

    digest = malloc(sizeof(unsigned char) * digest_size);
    err = digest == NULL;
    if (err == 0 && wc_Shake128Hash(p->str, p->len, digest, digest_size) != 0){
        err = 1;
    }
    else if(err == 0){
        clear_free(p->str, sizeof(unsigned char) * p->len);
        p->str = digest;
        p->len = digest_size;
    }

    if (err != 0){
        clear_free(digest, sizeof(unsigned char) * digest_size);
    }
#endif

    return err;
}

int bef_shake256(igloo_str *p, size_t digest_size)
{
    int err = 1;
    (void)p;
    (void)digest_size;

#if !defined(DISABLE_WOLFSSL) && \
        defined(WOLFSSL_SHA3) && defined(WOLFSSL_SHAKE256)
    unsigned char *digest = NULL;

    digest = malloc(sizeof(unsigned char) * digest_size);
    err = digest == NULL;
    if (err == 0 && wc_Shake256Hash(p->str, p->len, digest, digest_size) != 0){
        err = 1;
    }
    else if(err == 0){
        clear_free(p->str, sizeof(unsigned char) * p->len);
        p->str = digest;
        p->len = digest_size;
    }

    if (err != 0){
        clear_free(digest, sizeof(unsigned char) * digest_size);
    }
#endif

    return err;
}
