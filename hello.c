#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

#include "types.h"
#include "list_head.h"
#include "parser.h"

#define MAX_ARGS 256

int main(int argc, const char *argv[]){
    char buf[MAX_ARGS];
    pid_t pid;
    int status;

    fprintf(stdout,"%% ");

    while(fgets(buf,MAX_ARGS,stdin)!=NULL){

        if(buf[strlen(buf)-1] == '\n'){
            buf[strlen(buf)-1] = 0;
        }

        if((pid = fork()) < 0){
            fprintf(stderr,"fork error");
        }
        else if(pid == 0){ // 자식의 경우
            execlp(buf, buf,(char*)0);
            fprintf(stderr,"couldn't execute: %s\n", buf);
            exit(127);
        }
		// 부모의 경우
        if((pid = waitpid(pid, &status, 0)) < 0)
            fprintf(stderr, "waitpid error");

        if(strcmp(buf,"exit")==0) return 0;

        fprintf(stdout, "%% ");
    }
    
    exit(0);
}