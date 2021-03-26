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
    pid_t pid;
    int status;

    printf("before fork\n");
    pid = fork();
    printf("after fork\n");
    
    if(pid ==0){
        printf("자식 프로세스\n");
    }
    else{
        printf("부모 프로세스\n");
        wait(&status);
    }

    return 0;
}