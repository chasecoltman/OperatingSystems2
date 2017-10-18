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

//mutex
pthread_mutex_t buff_mutex;

//semaphores for blocking
sem_t* s1=NULL, *s2=NULL;

struct item
{
	long num;
	long sleep_time;
};

//buffer
struct item list[10];
int length = 0;

struct args{
	long tid;
	long sleep_time;
};

void *hello(void *tid)
{
	struct args *a = (struct args*)tid;
	sleep(a->sleep_time);
	return (void*)printf("Hello from thread %ld! I did %ld units of work!\n", a->tid, a->sleep_time);
}

void *work_producer(void *tid)
{
	char *id = (char *) tid;

	while(1)
	{
		//do stuff
		if(pthread_mutex_trylock(&buff_mutex) == 0)
		{
			if(length < 10)
			{
				printf("Producing item...\n");
				length++;
				long index = (length - 1);
				list[index].num = 3;
				list[index].sleep_time = 4;
				
				pthread_mutex_unlock(&buff_mutex);
				if(length == 1)
				{
					sem_post(s2);
				}
			}

			else
			{
				pthread_mutex_unlock(&buff_mutex);
				sem_wait(s1);
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
				printf("\nThread %ld consuming item %d...", a->tid, list[index].num);
				length--;
				sleep(list[index].sleep_time);
				printf("\n\n...Consumed in %d!\n", list[index].sleep_time);

				pthread_mutex_unlock(&buff_mutex);
				if(length == 9)
				{
					sem_post(s1);
				}
			}

			else
			{
				pthread_mutex_unlock(&buff_mutex);
				sem_wait(s2);
				//sleep(3);	
			}
		}
	}
}

int main(int argc, char **argv)
{
	s1 = sem_open("s1", O_CREAT, S_IRUSR | S_IWUSR, 0);
	s2 = sem_open("s2", O_CREAT, S_IRUSR | S_IWUSR, 0);
	assert(s1 && s2 && "Failed to allocate semaphores.");
	
	unsigned long init[4] = {0x123, 0x234, 0x345, 0x456};
	unsigned long length = 4;
	init_by_array(init, length);


	pthread_t producer;
	char c = 'p';
	pthread_t threads[atoi(argv[1])];
	struct args a[atoi(argv[1])];

	//create producer thread
	pthread_create(&producer, NULL, work_producer, &c);

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
	
	sem_close(s1);
	sem_close(s2);
	sem_unlink("s1");
	sem_unlink("s2");

	return 0;
}
