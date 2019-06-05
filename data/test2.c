#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

int main(){
	int fd = open("test.txt", O_RDWR|O_CREAT/*|O_TRUNC*/);
	if(fd < 0){
		perror("Failed to open: "); exit(1);
	}
	if(posix_fallocate(fd, 0, 512) != 0){
		perror("Failed to fallocate: "); exit(1);
	}
	char *map = (char *)mmap(NULL, 512, PROT_WRITE|PROT_READ|PROT_EXEC, MAP_SHARED, fd, 0);
	if(map == MAP_FAILED){
		perror("Failed to mmap: "); exit(1);
	}
	sprintf(map, "SHIT"); fprintf(stdout, "PRINT SHIT\n"); fflush(stdout);
	if(munmap(map, 512) == -1){
		perror("Failed to munmap: "); exit(1);
	}
	map = (char *)mmap(NULL, 512, PROT_WRITE|PROT_READ|PROT_EXEC, MAP_SHARED, fd, 3);
	if(map == MAP_FAILED){
		perror("Failed to mmap: "); exit(1);
	}
	sprintf(map, "2SHIT"); fprintf(stdout, "PRINT 2SHIT\n"); fflush(stdout);
	if(munmap(map, 512) == -1){
		perror("Failed to munmap: "); exit(1);
	}
	fprintf(stdout, "Ended!\n"); fflush(stdout);
	exit(0);
}
