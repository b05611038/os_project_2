#ifndef _USEFUL_H
#define _USEFUL_H
	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <errno.h>

#define PRINT(ARGS...)  do{fprintf(stdout,ARGS);fflush(stdout);}while(0)
	
#define PAGE_SIZE 4096UL

void perror_exit(char *string, int value){
	perror(string); exit(value);
}
#endif
