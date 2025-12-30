#ifndef IGLOO_BEFUDDLE_H
#define IGLOO_BEFUDDLE_H

#include "igloo_types.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

/* functions return 0 on success, 1 on failure unless otherwise specified */

int igloo_befuddle(igloo *ig);

/* concatenate string to front or back of password */
int bef_cat(igloo_str *p, int dir, const igloo_str *str);

/* concatenate string to front or back of password, string read from stdin */
int bef_salt(igloo *ig, igloo_str *p, int dir);

/* replace instances of string in password with new string, substitution amount
 * limited to x substitutions */
int bef_sub(igloo_str *p, const igloo_str *old, const igloo_str *new, size_t x);

/* choose a position to cut the password and switch the characters of both sides
 * (ex: "hello" @ pos 3 -> "lohel") */
int bef_cut(igloo_str *p, size_t x);

/* reverse the order of the password
 * (ex: "hello" -> "olleh") */
void bef_rev(igloo_str *p);

/* xor the password with a string, if the string is too short the rest of the
 * password remains unchanged */
void bef_xor(igloo_str *p, const igloo_str *str);

/* replace characters at position x with string, extending password if needed */
int bef_replace(igloo_str *p, const igloo_str *str, size_t x);

/* perform a 'deck' shuffle x times */
int bef_shuffle(igloo_str *p, size_t x);

/* do a caesar cipher with a shift of x */
void bef_ccipher(igloo_str *p, int x);

/* do a rail cipher with x rails */
int bef_rcipher(igloo_str *p, size_t x);

/* bitwise complement each char in the password */
void bef_complement(igloo_str *p);

/* use a PRNG implemented in igloo_random.h to generate bytes, each character
 *  in the password is xor'd with a byte from the PRNG
 * The PRNG may be seeded with 0 to select a seed based on the first 8 values in
 *  the password, or an explicit seed may be given
 *  a explicit seed might be limited to 32 bits or less, rather than 64 bits */
void bef_genxor(igloo_str *p, size_t seed);

/* very similar to genxor: characters are xor'd at least once and thereafter
 *  xor'd until they are printable
 * bad_chars allows extra characters to be excluded from the accepted values */
void bef_passwordify(igloo_str *p, size_t seed, igloo_str *bad_chars);


/* meta commands */


/* duplicate the password. Subsequent obfuscations will operate on the
 * duplicated password, which must later be joined with the parent */
int bef_fork(igloo_str_stack *istack, igloo_str **p);

/* similar to fork, but choose a length of the password to operate on */
int bef_substr(igloo_str_stack *istack, igloo_str **p, size_t ind1, size_t ind2);

/* join the top password of the stack with the password below
 * multiple join types are supported and are documented elsewhere
 *  see `igloo --list`
 *  the overriden string argument for these commands may be used as a comment */
int bef_join(igloo_str_stack *istack, igloo_str **p, igloo_str *istr);


/* hashes */


/* 32 or 64 byte digests */
int bef_sha2_256(igloo_str *p);
int bef_sha2_512(igloo_str *p);
int bef_sha3_256(igloo_str *p);
int bef_sha3_512(igloo_str *p);

/* up to 32 byte digest */
int bef_blake2s(igloo_str *p, size_t digest_size);
/* up to 64 byte digest */
int bef_blake2b(igloo_str *p, size_t digest_size);

/* digests with no limit on output size (technically 32 bit limit though) */
int bef_shake128(igloo_str *p, size_t digest_size);
int bef_shake256(igloo_str *p, size_t digest_size);

#endif
