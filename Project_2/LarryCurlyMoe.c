#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

#define NUM 3
int Maxnum;
sem_t Shovel;//1
sem_t filled;//Max
sem_t unfilled;//0
sem_t unfilled_with_seed;//0


void *func_Larry(void *args)
{
	int Holes_dug = 0;
	while (Holes_dug < Maxnum)
	{
		sem_wait(&filled);
		sem_wait(&Shovel);
		
		printf("Larry is digging now!\n");
		sleep(rand()%NUM);
		Holes_dug++;
		printf("Larry has dug %d holes.\n", Holes_dug);

		sem_post(&Shovel);
		sem_post(&unfilled);
	}
	printf("Larry: OFF DUTY!\n");		
	pthread_exit(NULL);	
}

void *func_Moe(void *args)
{
	int Holes_seeded = 0;
	while (Holes_seeded < Maxnum)
	{
		sem_wait(&unfilled);
		
		printf("Moe is seeding now");
		sleep(rand()%NUM);
		Holes_seeded++;
		printf("Moe has seeded %d holes.\n", Holes_seeded);
		
		sem_post(&unfilled_with_seed);
	}
	printf("Moe: OFF DUTY!\n");		
	pthread_exit(NULL);	
}

void *func_Curly(void *args)
{
	int Holes_filled = 0;
	while (Holes_filled < Maxnum)
	{
		sem_wait(&unfilled_with_seed);
		sem_wait(&Shovel);
		
		printf("Curly is filling now!\n");
		sleep(rand()%NUM);
		Holes_filled ++;
		printf ("Curly has filled %d holes.\n", Holes_filled);
		
		sem_post(&Shovel);
		sem_post(&filled);
	}
	printf("Curly: OFF DUTY!\n");
	pthread_exit(NULL);
}


int main (int argc, char **argv)
{
	
	Maxnum = atoi(argv[1]);

	pthread_t Larry, Moe, Curly;
	pthread_attr_t attr[3];
	for (int i =0; i < 3; i++)
	{
		pthread_attr_init(&attr[i]);
		pthread_attr_setdetachstate(&attr[i], PTHREAD_CREATE_JOINABLE);
		pthread_attr_setscope(&attr[i], PTHREAD_SCOPE_SYSTEM);
	}
	
	int ret = sem_init(&Shovel,0,1);
	if (ret == -1)
	{
		printf("Sem create failed!\n");
		return 1;
	}
	ret = sem_init(&filled,0,Maxnum);
	if (ret == -1)
	{
		printf("Sem create failed!\n");
		return 1;
	}
	ret = sem_init(&unfilled,0,0);
	if (ret == -1)
	{
		printf("Sem create failed!\n");
		return 1;
	}
	ret = sem_init(&unfilled_with_seed,0,0);
	if (ret == -1)
	{
		printf("Sem create failed!\n");
		return 1;
	}
	
	int rc = pthread_create(&Larry, &attr[0], func_Larry, NULL);
	if (rc)
	{
		printf("Join error!\n");
		return 1;
	}
	rc = pthread_create(&Moe, &attr[1], func_Moe, NULL);
	if (rc)
	{
		printf("Join error!\n");
		return 1;
	}
	rc = pthread_create(&Curly, &attr[2], func_Curly, NULL);
	if (rc)
	{
		printf("Join error!\n");
		return 1;
	}
	
	
	void *status;
	rc = pthread_join(Larry, &status);
    	if (rc)
    	{
    		printf("ERROR: Join error\n");
    		exit(-1);
    	}	
    	rc = pthread_join(Moe, &status);
    	if (rc)
    	{
    		printf("ERROR: Join error\n");
    		exit(-1);
    	}
    	rc = pthread_join(Curly, &status);
    	if (rc)
    	{
    		printf("ERROR: Join error\n");
    		exit(-1);
    	}
	
	
}
