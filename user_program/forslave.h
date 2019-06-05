#ifndef _FOR_SLAVE_H
#define _FOR_SLAVE_H
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <sys/mman.h>
	#include <sys/types.h>
	#include "useful.h"
	

void print_file_size(size_t file_size, size_t disk_file_size){
	PRINT("file size %lu  disk size %lu\n", file_size, disk_file_size);
}

void munmap_for_read(char *mmap_buffer, size_t count){
	if(munmap(mmap_buffer, count) == -1) perror_exit("Failed to munmap for read content: ", 1);
}

/*char *mmap_read(int fd, size_t count){
	char *ret;
	if((ret = (char *)mmap(NULL, PAGE_SIZE, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED)
		perror_exit("Failed to mmap for read content: ", 1);
	#ifdef DEBUG
		PRINT("read %u\n", ret);
		#ifdef CONTENT
			write(STDOUT_FILENO, mmap_buffer, ret);
		#endif
	#endif
	return ret;
}
*/

char *prepare_write_buffer(size_t fsize, size_t *disk_size, int fd){
	size_t dsize = *disk_size;
	if(posix_fallocate(fd, dsize, fsize + PAGE_SIZE - dsize) != 0) perror_exit("Failed to fallocate: ", 1);
	*disk_size = (fsize + PAGE_SIZE);
	char *ret = (char *)mmap(NULL, PAGE_SIZE, PROT_WRITE|PROT_READ|PROT_EXEC, MAP_SHARED, fd, (off_t)fsize);
	if(ret == MAP_FAILED) perror_exit("Failed to mmap for write: ", 1);
	return ret;
}

void munmap_write_buffer(char *mmap_buffer){
	if(munmap(mmap_buffer, PAGE_SIZE) == -1) perror_exit("Failed to munmap write_buffer: ", 1);
}

/*void mmap_write(size_t *file_size, size_t *disk_file_size, int fd, char *buf, size_t count){
	size_t fs = *file_size, dfs = *disk_file_size;
	char *mmap_buffer;
	if(posix_fallocate(fd, dfs, fs + PAGE_SIZE - dfs) != 0)	perror_exit("Failed to fallocate: ", 1);
	*disk_file_size = (fs + PAGE_SIZE);
	if((mmap_buffer = (char *)mmap(NULL, PAGE_SIZE, PROT_WRITE|PROT_READ|PROT_EXEC, MAP_SHARED, fd, fs)) == MAP_FAILED)
		
	if(memcpy(mmap_buffer, buf, count) == NULL) perror_exit("Failed to memcpy: ", 1);
	*file_size += count;
	#ifdef DEBUG
		PRINT("wrote %lu   ", count); print_file_size(*file_size, *disk_file_size);
		#ifdef CONTENT
			write(STDOUT_FILENO, mmap_buffer);
		#endif
	#endif
	if(munmap(mmap_buffer, PAGE_SIZE) == -1) perror_exit("Failed to munmap for write: ", 1);
}*/

#endif
