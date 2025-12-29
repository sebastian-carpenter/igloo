#ifndef IGLOO_PARSER_H
#define IGLOO_PARSER_H

#include "igloo_types.h"

// parse commands from the input string
//  multiple commands may be delimited by commas ('cmd1,cmd2')
//  escape sequences are processed
// return 0 on success, 1 on failure
int igloo_parse_commands(igloo_cmds *icmds, const char *cmds);

// receive a password from stdin of unrestricted length
//  escape sequences are processed
// return 0 on success, 1 on failure
int igloo_parse_stdin(igloo_str *istr, enum flag quiet);

#endif
