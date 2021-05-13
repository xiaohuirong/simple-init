#ifndef SHELL_H
#define SHELL_H
#include<stdbool.h>
#include<sys/types.h>

// builtin command main
typedef int cmd_main(int,char**);

// builtin command info
struct shell_command{
	bool enabled,fork;
	char name[32],help[256];
	cmd_main*main;
};

#ifdef ENABLE_INITSHELL

// src/shelld/shell.c: next status code
extern unsigned char exit_code;

// src/shelld/shell.c: run a new shell
extern void run_shell();

// src/shelld/shell.c: run_shell wrapper
extern int initshell_main(int argc,char**argv);

#endif

// src/shelld/cmd.c: find internal command by name
extern struct shell_command*find_internal_cmd(char*name);

// src/shelld/cmd.c: fork and invoke internal command
extern int invoke_internal_cmd(struct shell_command*cmd,bool background,char**args);

// src/shelld/cmd.c: invoke internal command
extern int invoke_internal_cmd_nofork(struct shell_command*cmd,char**args);

// src/shelld/cmd.c: install commands
extern int install_cmds(int dfd);
#endif
