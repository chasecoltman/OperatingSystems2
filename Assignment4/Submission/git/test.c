#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef __NR_mem_free
#define __NR_mem_free 359
#endif

#ifndef __NR_mem_used
#define __NR_mem_used 360
#endif

int main() {
        char * buffer0, buffer1, buffer2, buffer3, buffer4, buffer5, buffer6, buffer7;

        buffer0 = (char*) malloc (1234);
        buffer1 = (char*) malloc (1234);
        buffer2 = (char*) malloc (1234);
        buffer3 = (char*) malloc (1234);
        buffer4 = (char*) malloc (1234);
        buffer5 = (char*) malloc (12341234);
        buffer6 = (char*) malloc (12341234);
        buffer7 = (char*) malloc (12341234);

        
        double pused = (double)syscall(__NR_mem_used);
        printf("pused: %f\n",pused);
		double pfree = (double)syscall(__NR_mem_free);
        printf("pfree: %f\n",100 - pused);
		
        return 0;
}