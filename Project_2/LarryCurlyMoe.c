//use multithread to show the work among Larry, Moe and Curly
//the state of their work will be printed out
//when the work is finished, "off duty" will be printed out
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
	while (Holes_dug < Maxnum)//whether Larry dig enough holes
	{
		sem_wait(&filled);//to check whether there are holes unfilled
		sem_wait(&Shovel);// is Shovel available
		
		printf("Larry is digging now!\n");
		sleep(rand()%NUM);
		Holes_dug++;
		printf("Larry has dug %d holes.\n", Holes_dug);

		sem_post(&Shovel);// Shovel is available now
		sem_post(&unfilled);// a hole unfilled
	}
	printf("Larry: OFF DUTY!\n");		
	pthread_exit(NULL);	
}

void *func_Moe(void *args)
{
	int Holes_seeded = 0;
	while (Holes_seeded < Maxnum)//whether Moe seed enough holes
	{
		sem_wait(&unfilled);//to check whether there are holes unseeded
		
		printf("Moe is seeding now");
		sleep(rand()%NUM);
		Holes_seeded++;
		printf("Moe has seeded %d holes.\n", Holes_seeded);
		
		sem_post(&unfilled_with_seed);//an unfilled hole but with seed
	}
	printf("Moe: OFF DUTY!\n");		
	pthread_exit(NULL);	
}

void *func_Curly(void *args)
{
	int Holes_filled = 0;
	while (Holes_filled < Maxnum)//whether Curly fill enough holes
	{
		sem_wait(&unfilled_with_seed);// whether there is an unfilled hole with seeds
		sem_wait(&Shovel);//is Shovel availbale
		
		printf("Curly is filling now!\n");
		sleep(rand()%NUM);
		Holes_filled ++;
		printf ("Curly has filled %d holes.\n", Holes_filled);
		
		sem_post(&Shovel);// Shovel is available now
		sem_post(&filled);//In fact, this is not necessory
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
