#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>

#define PAGE_SIZE 4096
#define BUF_SIZE 512
#define MAP_SIZE PAGE_SIZE * 100

char *page_descriptors[128];
int page_descriptors_num = 0;

int main (int argc, char* argv[])
{
    char buf[BUF_SIZE];
    int i, dev_fd, file_fd;// dev_fd  the device and the file_fd for the input file
    size_t ret, file_size = 0, data_size = -1, offset = 0;
    char file_name[50]; // file_name 
    char method[20]; // file_name  or mmap 
    char ip[20]; //ip address
    struct timeval start;//start time
    struct timeval end;//finish time
    double trans_time; //calulate the time between the device is opened and it is closed
    char *kernel_address, *file_address;////finish time time and file_address 
    
    
    strcpy(file_name, argv[1]);
    strcpy(method, argv[2]);
    strcpy(ip, argv[3]);
    
    if( (dev_fd = open("/dev/slave_device", O_RDWR)) < 0)//open salve device file
    {
        perror("failed to open /dev/slave_device\n");
        return 1;
    }
    gettimeofday(&start ,NULL); // set start time
    if( (file_fd = open (file_name, O_RDWR | O_CREAT | O_TRUNC)) < 0)// open input file
    {
        perror("failed to open input file\n");
        return 1;
    }
    
    if(ioctl(dev_fd, 0x12345677, ip) == -1)    //check if socket connection is successfull
    {
        perror("ioclt create slave socket error\n");
        return 1;
    }
    
    
    switch(method[0])
    {
        case 'f'://fcntl
            do
            {
                ret = read(dev_fd, buf, sizeof(buf)); // read from the the device
                write(file_fd, buf, ret); //write to the input file
                file_size += ret; //caculate file size
            }while(ret > 0);
            break;
        case 'm'://mmap
            while (1) {
                ret = ioctl(dev_fd, 0x12345678, file_address); 
                if (ret == 0) {
                    file_size = offset;
                    break;
                }
                posix_fallocate(file_fd, offset, ret); //allocate file space
                file_address = mmap(NULL, ret, PROT_WRITE, MAP_SHARED, file_fd, offset); //file address
		if(page_descriptors_num==0 || file_address != page_descriptors[page_descriptors_num-1])
			page_descriptors[page_descriptors_num++] = file_address; //insert file_address to page descriptors
                kernel_address = mmap(NULL, ret, PROT_READ, MAP_SHARED, dev_fd, 0);
                memcpy(file_address, kernel_address, ret); //copy size:ret from memcpy to kernel_address
		if(munmap(file_address, ret) == -1){ perror("can't unmap!\n"); exit(9);} //munmap fail 
		if(munmap(kernel_address, ret) == -1){ perror("cant't unmap: "); exit(9);}//munmap fail 
                offset += ret;
            }
            break;
    }
    gettimeofday(&end, NULL); //set end time
    trans_time = (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)*0.0001; // cacluate transcation time
    printf("Transmission time: %lf ms, File size: %lu bytes\n", trans_time, file_size / 8UL);

    for(int i=0;i<page_descriptors_num;++i) ioctl(dev_fd, 7122, page_descriptors[i]); // page information
    if(ioctl(dev_fd, 0x12345679) == -1)// end receiving data, close the connection
    {
        perror("ioclt client exits error\n");
        return 1;
    }
    close(file_fd); //close input file
    close(dev_fd); //close device file
    return 0;
}

