#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(){
	int fd = open("test.txt", O_RDWR|O_CREAT|O_TRUNC);
	if(fd < 0){
		perror("Failed to open: "); exit(1);
	}
	exit(0);
}
