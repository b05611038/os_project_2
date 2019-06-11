#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#define BUF_SIZE 512
#define PAGE_SIZE 4096
#define MAP_SIZE PAGE_SIZE * 100

char *page_des[128];
int page_des_num = 0;

//get size of file
size_t get_filesize(const char* filename){
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}


int main (int argc, char* argv[])
{
    //buffer for read/write I/O
    char buf[BUF_SIZE];

    //device file and input file
    int dev_fd, file_fd;

    //counters for read() and mmap
    size_t ret, file_size, offset = 0;

    //input file name and method (fcntl or mmap)
    char file_name[50], method[20];

    //address for mmap
    char *kernel_addr = NULL, *file_addr = NULL;

    //start and end time for file transfer
    struct timeval start;
    struct timeval end;

    //end-star
    double trans_time;
    
    strcpy(file_name, argv[1]);
    strcpy(method, argv[2]);
    
    //open device
    if( (dev_fd = open("/dev/master_device", O_RDWR)) < 0){
        perror("failed to open /dev/master_device\n");
        return 1;
    }

    //start timing
    gettimeofday(&start ,NULL);

    //open input file
    if( (file_fd = open (file_name, O_RDWR)) < 0 ){
        perror("failed to open input file\n");
        return 1;
    }
    
    //get size of file
    if( (file_size = get_filesize(file_name)) < 0){
        perror("failed to get filesize\n");
        return 1;
    }
    
    //0x12345677 : create socket and accept connection from the slave device
    if(ioctl(dev_fd, 0x12345677) == -1){
        perror("ioclt server create socket error\n");
        return 1;
    }
    
    
    switch(method[0]){

        //method : fcntl
        case 'f': 
            do {
                //read from input file
                ret = read(file_fd, buf, sizeof(buf));
                //write to device
                write(dev_fd, buf, ret);
            } while(ret > 0);
            break;

        //method : mmap
        case 'm':
            while (offset < file_size) {
		//set the length to map
		size_t length = MAP_SIZE;
                if ((file_size - offset) < length) {
                    length = file_size - offset;
                }
		//get file address
                file_addr = mmap(NULL, length, PROT_READ, MAP_SHARED, file_fd, offset);
		//insert file address to page descriptors
		if(page_des_num==0 || page_des[page_des_num-1] != file_addr){
			page_des[page_des_num++] = file_addr;
		}
		//get kernel address
                kernel_addr = mmap(NULL, length, PROT_WRITE, MAP_SHARED, dev_fd, offset);
                //copy to kernel address
		memcpy(kernel_addr, file_addr, length);
		//unmap
		if(munmap(file_addr, length) == -1){ perror("Failed to munmap: "); exit(9);}
		if(munmap(kernel_addr, length) == -1){ perror("Failed to munmap: "); exit(9);}
                offset += length;
                ioctl(dev_fd, 0x12345678, length);
            }
            break;
    }

    //stop timing
    gettimeofday(&end, NULL);

    //count and print file transfer time (ms)
    trans_time = (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)*0.0001;
    printf("Transmission time: %lf ms, File size: %lu bytes\n", trans_time, file_size / 8LU);

    //page information
    for(int i=0;i<page_des_num;++i) ioctl(dev_fd, 7122, page_des[i]);

    //end sending data, close the connection
    if(ioctl(dev_fd, 0x12345679) == -1) {
        perror("ioclt server exits error\n");
        return 1;
    }

    //close input file and device
    close(file_fd);
    close(dev_fd);
    
    return 0;
}
