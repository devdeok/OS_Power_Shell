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

#define READ_END 0
#define WRITE_END 1

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
	int pipecheck=0;
	
	for(int i=0;i<nr_tokens;i++){ // pipe인지 체크 pipe면 1 아니면 0
		if(!strcmp(tokens[i],"|")) pipecheck++;
	}

	struct entry *cursor_h; // history용
	struct entry *cursor_b; // command !용

	if (strcmp(tokens[0], "exit") == 0) return 0;

	else if (!strcmp(tokens[0],"cd")){ //	implement change directory
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

	else if(!strcmp(tokens[0],"history")){ //	implement history
		list_for_each_entry(cursor_h,&history,list){
			// fprintf(stderr,"%2d: %s",cursor_h->index, cursor_h->command);
			printf("%2d: %s",cursor_h->index, cursor_h->command);
			
		}// history를 입력하면서 맨 앞에 !가 붙음
		return 1;
	}//											implement history

	else if(!strcmp(tokens[0],"!")){//	implement !
		char* tempstr = malloc(sizeof(char)*MAX_COMMAND_LEN);

		list_for_each_entry(cursor_b,&history,list){
			if(cursor_b->index==atoi(tokens[1])){
				strcpy(tempstr, cursor_b->command);
			}
		}// list_for_each_entry
		__process_command(tempstr);

		free(tempstr);

		return 1;
	}//									implement !

	else{ //	implement external command & pipe
		pid_t pid;

		if(pipecheck){ // pipe일 경우
			int temp;
			char* first[nr_tokens];
			char* second[nr_tokens];
			int fint=0,sint=0;

			for(int i=0;i<nr_tokens;i++){ // for
				if(!strcmp(tokens[i],"|"))
					temp = i;
			} // | 이거 들어간 인덱스 찾기
			
			for(int j=0;j<nr_tokens;j++){
				if(j<temp){
					first[fint++] = tokens[j];
				}
				else if(j>temp){
					second[sint++] = tokens[j];
				}
			}// | 이거 기준으로 문자열 나누기

			first[fint] = NULL;
			second[sint] = NULL;

			int pipefd[2];
			pipe(pipefd);
    		pid = fork();
			
			if(pid==0){ // child process
				close(pipefd[READ_END]);
				dup2(pipefd[WRITE_END], STDOUT_FILENO);
				close(pipefd[WRITE_END]);
				execvp(first[0], first);
				fprintf(stderr, "Failed to execute '%s'\n", first);
				exit(1);
			} // child process

			else{ // parent process
       			pid=fork();

				if(pid==0){
					close(pipefd[WRITE_END]);
					dup2(pipefd[READ_END], STDIN_FILENO);
					close(pipefd[READ_END]);
					execvp(second[0], second);
					fprintf(stderr, "Failed to execute '%s'\n", second);
					exit(1);
				}
				else{
					int status;
					close(pipefd[READ_END]);
					close(pipefd[WRITE_END]);
					waitpid(pid, &status, 0);
				}
			} // parent process

		} // pipe일 경우

		else{	// pipe가 아닐 경우
			int status;

			pid = fork();
			
			if(pid == 0){ // child process
				execvp(*tokens, tokens); // (file, array)
				fprintf(stderr,"Unable to execute %s\n", *tokens);
				exit(1);
			}
			else{ // parent process
				pid = waitpid(pid, &status, 0); 
			}
		} // pipe가 아닐 경우
	} // 		implement external command & pipe

	return -EINVAL;
} // run_command

/***********************************************************************
 * append_history()
 *
 * DESCRIPTION
 *   Append @command into the history. The appended command can be later
 *   recalled with "!" built-in command
 */

int tempint = 0; // history index

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
