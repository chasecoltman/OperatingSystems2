#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <semaphore.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "mt.h"

//made by Alec Zitzelberger and Chase Coltman

//pthread conditional control for blocking
enum { STATE_A, STATE_B } state = STATE_A;
pthread_cond_t condA  = PTHREAD_COND_INITIALIZER;

//mutex
pthread_mutex_t buff_mutex;

//semaphores for blocking
//sem_t* s1=NULL, *s2=NULL;

struct item
{
	long num;
	long sleep_time;
};

//buffer
struct item list[32];
int length = 0;

struct args{
	long tid;
	long sleep_time;
};

void *print_list(){
        printf("Current List \n");
        for (int i=0;i<length;i++){
                printf("%d ", list[i]);
        }
        printf("\n");
}

void *work_producer(void *tid)
{
	struct args *a = (struct args*)tid;

	while(1)
	{
		//do stuff
		if(pthread_mutex_trylock(&buff_mutex) == 0)
		{
			if(length < 32)
			{
				printf("Producer %ld Producing item...\n", a->tid);
				length++;
				long index = (length - 1);
				unsigned int wait;
				unsigned int eax;
				unsigned int ebx;
				unsigned int ecx;
				unsigned int edx;
				
				char vendor[13];
				
				eax = 0x01;
				
				__asm__ __volatile__(
	                     			"cpuid;"
	                     			: "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
	                     			: "a"(eax)
	                     			);
	
				if(ecx & 0x40000000){
					//use rdrand
					//unsigned int eax;

					__asm__ __volatile__(
	                     			"rdrand %%eax;"
	                     			: "=a"(eax)
	                     			);

					wait = eax % 8;
                                        if(wait < 3)
                                        {
                                                wait = 3;
                                        }
                                        list[index].num = eax % 20;
                                        list[index].sleep_time = eax % 10;
                                        if(list[index].sleep_time < 2)
                                        {
                                                list[index].sleep_time = 2;
                                        }

					
				}
				else{
					//use mt19937
					wait =  genrand_int32() % 8;
					if(wait < 3)
					{
						wait = 3;
					}
					list[index].num = genrand_int32() % 20;
					list[index].sleep_time = genrand_int32() % 10;
					if(list[index].sleep_time < 2)
					{
						list[index].sleep_time = 2;	
					}
				}

				//list[index].num = 3;
				//list[index].sleep_time = 4;
				print_list();
			
				if(length == 1)
				{
					state = STATE_B;
        				pthread_cond_signal(&condA);
        				//pthread_mutex_unlock(&buff_mutex);
					//sem_post(s2);
				}

				pthread_mutex_unlock(&buff_mutex);
				sleep(wait);
			}

			else
			{
				state = STATE_B;
				while(state != STATE_A)
				{
					printf("\nBuffer full, waiting on consumers...\n");
					pthread_cond_wait(&condA, &buff_mutex);
				}
				pthread_mutex_unlock(&buff_mutex);
				//sem_wait(s1);
				//sleep(3);	
			}
		}
	}
}

void *work_consumer(void *tid)
{
	struct args *a = (struct args*)tid;

	while(1)
	{
		//do stuff
		if(pthread_mutex_trylock(&buff_mutex) == 0)
		{
			if(length > 0)
			{
				long index = (length - 1);
				printf("\nConsumer %ld consuming item %d...", a->tid, list[index].num);
				length--;
				sleep(list[index].sleep_time);
				printf("\n\n...Consumed in %d!\n", list[index].sleep_time);

				if(length == 31)
				{
					//sem_post(s1);
					state = STATE_A;
        				pthread_cond_signal(&condA);
				}
				print_list();
				pthread_mutex_unlock(&buff_mutex);
			}

			else
			{
				state = STATE_A;
				while(state != STATE_B)
                                {
					printf("\nEmpty buffer, waiting on producers...\n");
                                        pthread_cond_wait(&condA, &buff_mutex);
                                }
                                pthread_mutex_unlock(&buff_mutex);
				//pthread_mutex_unlock(&buff_mutex);
				//sem_wait(s2);
				//sleep(3);	
			}
		}
	}
}

int main(int argc, char **argv)
{
	//s1 = sem_open("s1", O_CREAT, S_IRUSR | S_IWUSR, 0);
	//s2 = sem_open("s2", O_CREAT, S_IRUSR | S_IWUSR, 0);
	//assert(s1 && s2 && "Failed to allocate semaphores.");
	
	if((atoi(argv[1]) == 0  || atoi(argv[2]) == 0))
	{
		printf("Please use positive non-zero numbers for number of consumers and producers\n");
		return 1;
	}

	if(argc != 3)
	{
		printf("Incorrect command. Please enter ./#program -numConsumer -numProducers\n");
		return 1;
	}	
	unsigned long init[4] = {0x123, 0x234, 0x345, 0x456};
	unsigned long length = 4;
	init_by_array(init, length);


	pthread_t producer;
	char c = 'p';
	pthread_t producers[atoi(argv[2])];
	pthread_t threads[atoi(argv[1])];
	struct args b[atoi(argv[2])];
	struct args a[atoi(argv[1])];

	//create producer thread
	//pthread_create(&producer, NULL, work_producer, &c);
	for(long i = 0; i < atoi(argv[2]); ++i){
		//printf("ld", i);
                /* int pthread_create(pthread_t *thread, const pthread_attr_t *attr, */
                /*                    void *(*start_routine) (void *), void *arg); */

                b[i].tid = i;
                b[i].sleep_time = genrand_int32() % 10;

                pthread_create(&(producers[i]),
                               NULL,
                               work_producer,
                               (void*)&b[i]);
        }
	

	for(long i = 0; i < atoi(argv[1]); ++i){

		/* int pthread_create(pthread_t *thread, const pthread_attr_t *attr, */
		/*                    void *(*start_routine) (void *), void *arg); */

		a[i].tid = i;
		a[i].sleep_time = genrand_int32() % 10;

		pthread_create(&(threads[i]),
		               NULL,
		               work_consumer,
		               (void*)&a[i]);
	}

	for(long i = 0; i < atoi(argv[1]); ++i){
		pthread_join(threads[i], NULL);
	}
	
	pthread_join(producer, NULL);
	
	//sem_close(s1);
	//sem_close(s2);
	//sem_unlink("s1");
	//sem_unlink("s2");

	return 0;
}
