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
#include <stdbool.h>
#include <time.h>

//pthread conditional control for blocking
enum { STATE_A, STATE_B } state = STATE_A;
pthread_cond_t condA  = PTHREAD_COND_INITIALIZER;

//mutex
pthread_mutex_t buff_mutex;

pthread_mutex_t forks[5];

struct item
{
	long num;
	long sleep_time;
};

struct fork
{
	int id;
	bool held;
};

struct philosopher
{
	char name[12];
	int id;
};

//the table setup
struct fork table[5]; 

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

void think(char* name)
{
	unsigned long wait;

	wait = genrand_int32() % 21;
	if(wait == 0)
	{
		wait = 1;
	}

	printf("%s will think for %ld seconds...\n", name, wait);
	sleep(wait);
	printf("%s is done thinking. Boy is he hungry!\n", name);
}

void eat(char* name)
{
	unsigned long wait;

	wait = genrand_int32() % 10;
	if(wait <= 1)
	{
		wait = 2;
	}

	printf("%s will eat for %ld seconds...\n", name, wait);
	sleep(wait);
	printf("%s is done eating. Time to drop the forks!\n", name);
}

void get_forks(char* name, int id)
{
	unsigned long wait;
	bool got_left = false;
	bool got_right = false;
	int left = id;
	int right = (id + 1) % 5;

	while(got_left == false || got_right == false)
	{

		if(pthread_mutex_trylock(&forks[left])==0)
		{
			if(table[left].held == false)
			{
				got_left = true;
				table[left].held = true;
				printf("%s picked up fork %ld\n\n", name, left);
			}
	
			pthread_mutex_unlock(&forks[left]);
		}

		if(pthread_mutex_trylock(&forks[right])==0)
		{
			if(table[right].held == false)
			{
				got_right = true;
				table[right].held = true;
				printf("%s picked up fork %ld\n\n", name, right);
			}

			pthread_mutex_unlock(&forks[right]);
		}

		if(got_right == false || got_left == false)
		{
			printf("%s failed to attain two forks, time to cry DX.\n", name);
			if(got_right == true)
			{
				got_right = false;
				table[right].held = false;
				printf("%s drops fork %ld\n\n", name, right);
			}

			if(got_left == true)
			{
				got_left = false;
				table[left].held = false;
				printf("%s drops fork %ld\n\n", name, left);
			}
			
			wait = genrand_int32() % 4;
			if(wait == 0)
			{
				wait = 1;
			}
			sleep(wait);
		}
	}

	printf("%s successfully acquired forks! Time to eat :D\n", name);
}

void drop_forks(char* name, int id)
{
	bool got_left = true;
	bool got_right = true;
	int left = id;
	int right = (id + 1) % 5;
	
	while(got_left == true || got_right == true)
	{
		if(pthread_mutex_trylock(&forks[left]) == 0)
		{
			if(got_left = true)
			{
				table[left].held = false;
				got_left = false;
			}
			pthread_mutex_unlock(&forks[left]);
		}

		if(pthread_mutex_trylock(&forks[right]) == 0)
		{
			if(got_right = true)
			{
				table[right].held = false;
				got_right = false;
			}
			pthread_mutex_unlock(&forks[right]);
			
		}
	}

	printf("%s succesfully dropped forks %ld and %ld!\n", name, left, right);
}

void *work_philosopher(void *tid)
{
	struct philosopher *a = (struct philosopher*)tid;

	printf("Philosopher %ld %s at the table!\n", a->id, a->name);

	while(1)
	{
		think(a->name);
		get_forks(a->name, a->id);
		eat(a->name);
		drop_forks(a->name, a->id);
	};
}

int main(int argc, char **argv)
{	
	unsigned long init[4] = {0x123, 0x234, 0x345, 0x456};
	unsigned long length = 4;

	init_genrand((unsigned long)time(NULL));

	for(int i = 0; i < 5; i++)
	{
		pthread_mutex_init(&forks[i], NULL);
		table[i].id = i;
		table[i].held = false;
		printf("Fork %ld set on the table.\n", table[i].id);
	};

	pthread_t philosophers[5];
	struct philosopher a[5];

	char names[5][12];
	strcpy(names[0], "Kant");
	strcpy(names[1], "Descartes");
	strcpy(names[2], "Plato");
	strcpy(names[3], "Marx");
	strcpy(names[4], "Aristotle");

	for(long i = 0; i < 5; ++i){

		strcpy(a[i].name,  names[i]);
		a[i].id = i;

		pthread_create(&(philosophers[i]),
		               NULL,
		               work_philosopher,
		               (void*)&a[i]);
	}

	for(long i = 0; i < 5; ++i){
		pthread_join(philosophers[i], NULL);
	}
	
	return 0;
}
