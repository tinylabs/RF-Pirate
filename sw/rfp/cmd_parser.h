#ifndef _CMD_PARSER_H_
#define _CMD_PARSER_H_

#include <stdint.h>
#include "usb_serial.h"

#define MAX_ARGS 2
#define MAX_CMDS 10

typedef struct {
  char *cmd;
  char *arg[MAX_ARGS];
} tok_t;

/* callback type for parsed commands */
typedef void (*cmd_cb_t)(uint8_t argc, tok_t *tok);

/* Register a cmd for parsing */
uint8_t cmdp_register_cmd (const char *cmd, cmd_cb_t);
void    cmdp_parse_cmd (string_t *str);
void    cmdp_init (void);

#endif /* _CMD_PARSER_H_ */
