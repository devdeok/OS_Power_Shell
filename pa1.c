/**********************************************************************
 * Copyright (c) 2021
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>

#include <string.h>

#include "types.h"
#include "list_head.h"
#include "parser.h"

#include <wait.h>

/***********************************************************************
 * struct list_head history
 *
 * DESCRIPTION
 *   Use this list_head to store unlimited command history.
 */
LIST_HEAD(history);

struct entry{	
	struct list_head list;
	char* command;
	int index;
};

static int __process_command(char * command);
/***********************************************************************
 * run_command()
 *
 * DESCRIPTION
 *   Implement the specified shell features here using the parsed
 *   command tokens.
 *
 * RETURN VALUE
 *   Return 1 on successful command execution
 *   Return 0 when user inputs "exit"
 *   Return <0 on error
 */
static int run_command(int nr_tokens, char *tokens[]){
	// pipe 구현하기 |

	struct entry *cursor;

	if (strcmp(tokens[0], "exit") == 0) return 0;

	else if (!strcmp(tokens[0],"cd")){ // 	implement change directory
		if(nr_tokens > 1){ // cd ~~
			if(!strcmp(tokens[1],"~")){ //move home directory
				chdir(getenv("HOME"));
			}
			else{ // input specify directory
				chdir(tokens[1]);
			}
		}
		else{ //input only command cd
				chdir(getenv("HOME"));
		}
		return 1;
	}//  									implement change directory

	else if(!strcmp(tokens[0],"history")){ //		implement history
		list_for_each_entry(cursor,&history,list){
			fprintf(stderr,"%2d: %s",cursor->index, cursor->command);
		} // history 출력할 때 이상한거 붙어있음, 이거 코드 좀 봐야할듯
		return 1;
	}//												implement history

	else if(!strcmp(tokens[0],"!") && nr_tokens>1){//implement !
		char* tempstr = malloc(sizeof(char)*MAX_COMMAND_LEN);

		list_for_each_entry(cursor,&history,list){
			if(cursor->index==atoi(tokens[1])){
				strcpy(tempstr, cursor->command);
			}
		}// list_for_each_safe
		__process_command(tempstr);
		free(tempstr);

		return 1;
	}//												implement !

	else{ // 						implement external command & pipe
		pid_t pid;
		int pipefd[2];
		int status;

		// if(pipe(pipefd)==-1){ // create the pipe
		// 	fprintf(stderr,"pipe error");
		// 	exit(1);
		// }

		
        pid = fork();
		
        if(pid == 0){ // child process
            execvp(*tokens, tokens); // (file, array)
            fprintf(stderr,"Unable to execute %s\n", *tokens);
			exit(1);
        }
        else{ // parent process
			pid = waitpid(pid, &status, 0); 
		}
	} // 							implement external command & pipe
	
	return -EINVAL;
}

/***********************************************************************
 * append_history()
 *
 * DESCRIPTION
 *   Append @command into the history. The appended command can be later
 *   recalled with "!" built-in command
 */

int tempint = 0;

static void append_history(char * const command)
{
	// 명령어는 ! <숫자> 숫자에 있는 command 실행

	struct entry *temp = (struct entry *)malloc(sizeof(struct list_head));
	temp->command = (char*)malloc(sizeof(char)*(strlen(command)));

	temp->index = tempint;
	strcpy(temp->command,command);
	INIT_LIST_HEAD(&temp->list);
	list_add_tail(&temp->list, &history);

	tempint++;
}


/***********************************************************************
 * initialize()
 *
 * DESCRIPTION
 *   Call-back function for your own initialization code. It is OK to
 *   leave blank if you don't need any initialization.
 *
 * RETURN VALUE
 *   Return 0 on successful initialization.
 *   Return other value on error, which leads the program to exit.
 */
static int initialize(int argc, char * const argv[])
{

	return 0;
}


/***********************************************************************
 * finalize()
 *
 * DESCRIPTION
 *   Callback function for finalizing your code. Like @initialize(),
 *   you may leave this function blank.
 */
static void finalize(int argc, char * const argv[])
{

}


/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING BELOW THIS LINE ******      */
/*          ****** BUT YOU MAY CALL SOME IF YOU WANT TO.. ******      */
static int __process_command(char * command)
{
	char *tokens[MAX_NR_TOKENS] = { NULL };
	int nr_tokens = 0;

	if (parse_command(command, &nr_tokens, tokens) == 0)
		return 1;

	return run_command(nr_tokens, tokens);
}

static bool __verbose = true;
static const char *__color_start = "[0;31;40m";
static const char *__color_end = "[0m";

static void __print_prompt(void)
{
	char *prompt = "$";
	if (!__verbose) return;

	fprintf(stderr, "%s%s%s ", __color_start, prompt, __color_end);
}

/***********************************************************************
 * main() of this program.
 */
int main(int argc, char * const argv[])
{
	char command[MAX_COMMAND_LEN] = { '\0' };
	int ret = 0;
	int opt;

	while ((opt = getopt(argc, argv, "qm")) != -1) {
		switch (opt) {
		case 'q':
			__verbose = false;
			break;
		case 'm':
			__color_start = __color_end = "\0";
			break;
		}
	}

	if ((ret = initialize(argc, argv))) return EXIT_FAILURE;

	/**
	 * Make stdin unbuffered to prevent ghost (buffered) inputs during
	 * abnormal exit after fork()
	 */
	setvbuf(stdin, NULL, _IONBF, 0);

	while (true) {
		__print_prompt();

		if (!fgets(command, sizeof(command), stdin)) break;

		append_history(command);
		ret = __process_command(command);

		if (!ret) break;
	}

	finalize(argc, argv);

	return EXIT_SUCCESS;
}
