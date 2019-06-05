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
    int i, dev_fd, file_fd;// the fd for the device and the fd for the input file
    size_t ret, file_size = 0, data_size = -1, offset = 0;
    char file_name[50];
    char method[20];
    char ip[20];
    struct timeval start;
    struct timeval end;
    double trans_time; //calulate the time between the device is opened and it is closed
    char *kernel_address, *file_address;
    
    
    strcpy(file_name, argv[1]);
    strcpy(method, argv[2]);
    strcpy(ip, argv[3]);
    
    if( (dev_fd = open("/dev/slave_device", O_RDWR)) < 0)//should be O_RDWR for PROT_WRITE when mmap()
    {
        perror("failed to open /dev/slave_device\n");
        return 1;
    }
    gettimeofday(&start ,NULL);
    if( (file_fd = open (file_name, O_RDWR | O_CREAT | O_TRUNC)) < 0)
    {
        perror("failed to open input file\n");
        return 1;
    }
    
    if(ioctl(dev_fd, 0x12345677, ip) == -1)    //0x12345677 : connect to master in the device
    {
        perror("ioclt create slave socket error\n");
        return 1;
    }
    
    
    switch(method[0])
    {
        case 'f'://fcntl : read()/write()
            do
            {
                ret = read(dev_fd, buf, sizeof(buf)); // read from the the device
                write(file_fd, buf, ret); //write to the input file
                file_size += ret;
            }while(ret > 0);
            break;
        case 'm':
            while (1) {
		//printf("ioctl MMAP\n"); fflush(stdout);
                //posix_fallocate(file_fd, offset, ret);//new
                //file_address = mmap(NULL, ret, PROT_WRITE, MAP_SHARED, file_fd, offset);//new
                ret = ioctl(dev_fd, 0x12345678, file_address);
                if (ret == 0) {
                    file_size = offset;
                    break;
                }
                posix_fallocate(file_fd, offset, ret);
                file_address = mmap(NULL, ret, PROT_WRITE, MAP_SHARED, file_fd, offset);
		if(page_descriptors_num==0 || file_address != page_descriptors[page_descriptors_num-1])
			page_descriptors[page_descriptors_num++] = file_address;
		//printf("before unmap %lX   ", file_address);
                kernel_address = mmap(NULL, ret, PROT_READ, MAP_SHARED, dev_fd, 0);
                memcpy(file_address, kernel_address, ret);
		if(munmap(file_address, ret) == -1){ perror("can't unmap!\n"); exit(9);}
		//printf("after unmap %lX\n", file_address); fflush(stdout);
		if(munmap(kernel_address, ret) == -1){ perror("cant't unmap: "); exit(9);}
                offset += ret;
            }
            break;
    }
    gettimeofday(&end, NULL);
    trans_time = (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)*0.0001;
    printf("Transmission time: %lf ms, File size: %lu bytes\n", trans_time, file_size / 8UL);

    for(int i=0;i<page_descriptors_num;++i) ioctl(dev_fd, 7122, page_descriptors[i]);
    if(ioctl(dev_fd, 0x12345679) == -1)// end receiving data, close the connection
    {
        perror("ioclt client exits error\n");
        return 1;
    }
    close(file_fd);
    close(dev_fd);
    return 0;
}

