#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

int enter = 0;
int checked = 0;
int numImm = 0;
int numJud = 0;
int numSpe = 0;
sem_t Nojudge; // 0
sem_t mutex; //1
sem_t confirmed; //0
sem_t allSignedIn; //0

void shortdelay()
{
	sleep(0.5 + (double)rand() /20 );
}
void *func_Immigrants(void *args)
{
	int curNum = numImm;
	numImm++;
	sem_wait(&Nojudge);
	sem_wait(&mutex);
	enter ++;
	printf("An immigrant #%d is entering.\n ", curNum);
	shortdelay();
	sem_post(&mutex);	
	sem_post(&Nojudge);
	
	sem_wait(&mutex);
	checked ++;
	printf("An immigrant #%d checked.\n", curNum);
	shortdelay();
	sem_post(&mutex);

	sem_wait(&mutex);
	if (numJud == 1 && enter == checked)
	{
		sem_post(&allSignedIn);
	}
	sem_post(&mutex);

	printf("An immigrant #%d sit down.\n", curNum);
	sem_post(&confirmed);
	shortdelay();

	printf("An immigrant #%d swear.\n", curNum);
	shortdelay();
	printf("An immigrant #%d get certificate.\n", curNum);
	shortdelay();

	sem_wait(&Nojudge);
	printf("An immigrant #%d left.\n", curNum);
	sem_post(&Nojudge);
}

void *func_Judges(void *args)
{
	int curNum = numJud;
	numJud ++;
	sem_wait(&Nojudge);
	sem_wait(&mutex);
	printf("A judge #%d entered.\n", curNum);
	shortdelay();
	sem_post(&mutex);
	sem_post(&Nojudge);

	sem_wait(&mutex);
	if (enter > checked)
	{
		sem_post(&mutex);
		sem_wait(&allSignedIn);
	}

	printf("The judge #%d is confirming.\n", curNum);
	shortdelay();

	


}

void *func_Spectators(void *args)
{

}

int main (int argc, char **argv)
{
	pthread_t Immigrants, Judges, Spectators;
	pthread_attr_t attr[3];
	for (int i =0; i < 3; i++)
	{
		pthread_attr_init(&attr[i]);
		pthread_attr_setdetachstate(&attr[i], PTHREAD_CREATE_JOINABLE);
		pthread_attr_setscope(&attr[i], PTHREAD_SCOPE_SYSTEM);
	}
	
	int ret = sem_init(&allSignedIn, 0, 0);
	if (ret == -1)
	{
		printf("Sem create failed!\n");
		return 1;
	}
	ret = sem_init (&Nojudge, 0, 0);
	if (ret == -1)
	{
		printf("Sem create failed!\n");
		return 1;
	}
	ret = sem_init(&mutex, 0, 1);
	if (ret == -1)
	{
		printf("Sem create failed!\n");
		return 1;
	}
	
}
