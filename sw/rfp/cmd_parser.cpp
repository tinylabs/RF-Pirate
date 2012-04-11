#include "cmd_parser.h"

/* Internal data structure */
typedef struct {
  const char *cmd;
  cmd_cb_t cb;
} cmd_entry_t;

/* Array to store all commands */
static cmd_entry_t cmd_tbl[MAX_CMDS];

/* Single global, reused for all dispatches */
static tok_t token;

void cmdp_init (void)
{
  uint8_t i;

  for (i = 0; i < MAX_CMDS; i++) {
    cmd_tbl[i].cmd = NULL;
    cmd_tbl[i].cb = (cmd_cb_t)NULL;
  }
}

uint8_t cmdp_register_cmd (const char *cmd, cmd_cb_t cb)
{
  uint8_t i;

  for (i = 0; i < MAX_CMDS; i++) {
    // find empty command
    if (cmd_tbl[i].cmd == NULL) {
      cmd_tbl[i].cmd = cmd;
      cmd_tbl[i].cb = cb;
    }
  }
  return (i != MAX_CMDS) ? 0 : 1;
}

/* Search for cmd in list and parse args
 * then dispatch to handler */
void cmdp_parse_cmd (string_t *str)
{
  uint8_t i, arg_idx = 0;
  char *tok;

  // Parse cmd into constituent parts
  memset (&token, 0, sizeof (tok_t));
  tok = str->data;
  //ser.printf ("#len = %u\r\n", str->len);
  for (i = 0; i < str->len && arg_idx < MAX_ARGS; i++) {
    // First instance is cmd
    if ((str->data[i] == ' ') || (str->data[i] == '\0')) {
      if (token.cmd == NULL) {
	token.cmd = tok;
      }
      // else it is an argument
      else {
	token.arg[arg_idx++] = tok;
      }
      str->data[i] = '\0'; // Null terminate
      //ser.printf("#tok = [%s]\r\n", tok);
      tok = &str->data[i + 1]; // increment tok ptr
    }
  }

  // Match command
  for (i = 0; i < MAX_CMDS; i++) {
    if (strncmp (cmd_tbl[i].cmd, token.cmd, strlen(cmd_tbl[i].cmd)) == 0) {
      // Dispatch if cb is available
      if (cmd_tbl[i].cb != NULL)
	cmd_tbl[i].cb(arg_idx, &token);
      break;
    }
  }

  // Couldn't find a match
  if (i == MAX_CMDS) {
    ser.printf ("Bad cmd [%s]\r\n", token.cmd);
  }

  // Release buffer whether or not we found a match
  RELEASE_BUF(token.cmd);
}
