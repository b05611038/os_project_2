#ifndef _FOR_MASTER_H
#define _FOR_MASTER_H
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <sys/mman.h>
	#include <sys/types.h>
	#include "useful.h"

void munmap_for_read(char *mmap_buffer, size_t count){
	if(munmap(mmap_buffer, count) == -1) perror_exit("Failed to munmap for read content: ", 1);
}

char *mmap_read(int fd, size_t offset, size_t count){
	char *ret;
	if((ret = (char *)mmap(NULL, count, PROT_READ, MAP_SHARED, fd, (off_t)offset)) == MAP_FAILED)
		perror_exit("Failed to mmap for read: ", 1);
	#ifdef DEBUG
		PRINT("read %u\n", count);
	#endif
	return ret;
}

void mmap_write(int fd, char *buf, size_t count){
	size_t *send; char *mmap_buffer;
	if((send = (size_t *)mmap(NULL, sizeof(size_t), PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
		perror_exit("Failed to mmap for write length: ", 1);
	*send = count;
	#ifdef DEBUG
		PRINT("send %u\n", count);
	#endif
	if(munmap(send, sizeof(size_t)) == -1) perror_exit("Failed to munmap for write lenght: ", 1);
	
	if((mmap_buffer = (char *)mmap(NULL, count, PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
		perror_exit("Failed to mmap for write content: ", 1);
	if(memcpy(mmap_buffer, buf, count) == NULL) perror_exit("Failed to memcpy: ", 1);
	if(munmap(mmap_buffer, count) == -1) perror_exit("Failed to munmap for write content: ", 1);
}
#endif
