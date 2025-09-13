#ifndef IGLOO_H
#define IGLOO_H

#include <stdio.h>

#include "igloo_types.h"

#define PROFILE_DIR "/.igloo/"
#define LIST_CONST 257

/* returns 0 on success, 1 on failure */
int igloo_init(igloo *ig);

/* prepare a char array to store output file path
 * returns NULL on failure, char * otherwise */
char * igloo_prepare_output(const igloo *ig, size_t *output_len);

/* prints a password
 *  if printing to stdout a newline will be appended
 *  if printing to a file no newline will be appended
 * returns 0 on success, 1 on failure */
int igloo_print_password(FILE *st, const igloo *ig);

/* open a specified profile location
 *  relative locations will be preprended with PROFILE_DIR
 *  abosolute locations will not be tampered with
 * returns NULL on failure, FILE * on success */
FILE * open_profile(const igloo *ig, const char *profile_loc, const char *type);

/* parse igloo commands from a file
 * return 0 on success, 1 on failure */
int igloo_load(igloo *ig, const char *profile_input);

/* save igloo commands to a file
 * return 0 on success, 1 on failure */
int igloo_save(const igloo *ig);

/* clear sensitive data and free memory */
void igloo_free(igloo *ig);

#endif
