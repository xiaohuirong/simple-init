#ifdef ENABLE_INITSHELL
#include<errno.h>
#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<sys/select.h>
#include<readline/readline.h>
#include<readline/history.h>
#include"shell_internal.h"
#include"output.h"
#include"array.h"

#define DEF_PS1 "\\$ "

static bool sigwinch=false;
bool shell_running=false,shell_executing=false;
unsigned char exit_code=0;
static char prompt[1024],*last=NULL;

static void update_prompt(){
	char*ps1=getenv("PS1");
	if(!ps1)ps1=DEF_PS1;
	shell_replace(prompt,ps1,sizeof(prompt)-1);
	rl_set_prompt(prompt);
}

static void shell_exit(int x){
	if(last)free(last);
	last=NULL;
	printf("\nexit\n");
	rl_callback_handler_remove();
	shell_running=false;
	exit(x);
}

static void sighand(int s __attribute__((unused))){
	switch(s){
		case SIGINT:
			if(shell_executing)return;
			printf("^C\r\n");
			exit_code=128|s;
			update_prompt();
			rl_replace_line("",0);
			rl_forced_update_display();
		return;
		case SIGWINCH:sigwinch=true;return;
		case SIGQUIT:case SIGTERM:shell_exit(s);
		default:;
	}
}

static void linehandler(char*line){
	bool removed=false;
	shell_executing=true;
	if(!line)shell_exit(exit_code);
	else{
		char**args;
		if((args=args2array(line,0))){
			if(args[0]){
				switch(line[0]){
					case ' ':case '\t':case '\n':break;
					default:if(
						!last||
						strlen(last)!=strlen(line)||
						strcmp(line,last)!=0
					)add_history(line);
				}
				rl_callback_handler_remove();
				exit_code=run_cmd(args,false);
				removed=true;
			}
			free_args_array(args);
		}
		if(last)free(last);
		last=line;
		update_prompt();
		if(removed)rl_callback_handler_install(prompt,linehandler);
	}
	shell_executing=false;
}

void run_shell(){
	int r;
	fd_set fds;
	signal(SIGINT,sighand);
	signal(SIGTERM,sighand);
	signal(SIGQUIT,sighand);
	signal(SIGWINCH,sighand);
	if(!getenv("PS1"))setenv("PS1","\\VINIT(\\u@\\h):\\w \\$ ",1);
	update_prompt();
	rl_callback_handler_install(prompt,linehandler);
	shell_running=true;
	while(shell_running){
		FD_ZERO(&fds);
		FD_SET(fileno(rl_instream),&fds);
		r=select(FD_SETSIZE,&fds,NULL,NULL,NULL);
		if(r<0){
			if(errno!=EINTR){
				perror("select");
				rl_callback_handler_remove();
				break;
			}else continue;
		}
		if(sigwinch)rl_resize_terminal();
		sigwinch=false;
		if(FD_ISSET(fileno(rl_instream),&fds))
		rl_callback_read_char();
	}
	shell_exit(exit_code);
}

int initshell_main(int argc __attribute__((unused)),char**argv __attribute__((unused))){
	run_shell();
	return 0;
}
#endif
