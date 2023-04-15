#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

#define MAX_THREAD 500 
//create queue
typedef struct
{
	/* data */
	int start;
	int end;
	int max_size;
	int *data;
}queue;

void queue_init(queue *q, int max)
{
	q->max_size = max;
	q->start = 0;
	q->end = 1;
	q->data = malloc(q->max_size * sizeof(int));
}

void doublespace(queue *q)
{
	int *tmp = q->data;
	q->data = malloc(q->max_size * 2 *sizeof(int));
	int i;
	for (i =0; i< q->max_size; ++i)
		q->data[i] = tmp[i];
	q->max_size *=2;
	free(tmp);
}

void push(queue *q, int item)
{
	q->data[q->end] =item;
	if ((q->end+1)%q->max_size == q->start)
		doublespace(q);
	q->end =(q->end + 1) % q->max_size;
}

int pop(queue*q)
{
	if((q->start + 1)%q ->max_size == q->end)
	{
		printf("Pop Errod!\n");
		exit(EXIT_FAILURE);
	}
	q->start = (q->start + 1)%q->max_size;
	return q->data[q->start];
}

void free_vector(queue *q)
{
	free(q->data);
}
//

int immNum = 0, specNum = 0, judNum = 0;
int uncheck = 0;

sem_t judIn, check, sitDown, immConfirmed[MAX_THREAD];

queue Sitdown;// to record the immigrants who sit down

int sitNum = 0, unconfirmNum = 0;

sem_t immEnter;//to prevent judege enter while there is no immigrant in the hall

void delay()
{
	sleep(rand()%3 + 1);
}

void shortdelay()
{
	sleep(0.5 + (double)rand()/RAND_MAX);
}

void *func_immigrant()
{
	sem_wait(&judIn);
	sem_post(&judIn);
	int imm_no = immNum;
	++ immNum;

	++ uncheck;
	++ unconfirmNum;
	//enter
	shortdelay();
	printf("Immigrant #%d enter\n", imm_no);
	sem_post(&immEnter);
	//checkIn
	shortdelay();
	printf("Immigrant #%d checkIn\n", imm_no);
	sem_post(&check);
	--uncheck;
	
	shortdelay();
	printf("Immigrant #%d sitDown\n", imm_no);
	push(&Sitdown, imm_no);//save the no of the immigrant
	sem_post(&sitDown);

	sem_wait(&immConfirmed[imm_no]);
	shortdelay();
	printf("Immigrant#%d getCertificate\n", imm_no);
	
	sem_wait(&judIn);
	shortdelay();
	printf("Immigrant #%d leave\n", imm_no);
	sem_post(&judIn);
	sem_wait(&immEnter);

	pthread_exit(NULL);
}

void *func_spectator()
{
	sem_wait(&judIn);
	sem_post(&judIn);
	int spec_no = specNum;
	++specNum;

	shortdelay();
	printf("Spectator #%d enter\n", spec_no);
	
	shortdelay();
	printf("Spectator #%d spectate\n", spec_no);
	
	shortdelay();
	printf("Spectator #%d leave\n", spec_no);
	
	pthread_exit(NULL);
}

void *func_judge()
{
	sem_wait(&immEnter);
	sem_post(&immEnter);

	sem_wait(&judIn);

	int jud_no = judNum;
	++judNum;
	shortdelay();
	printf("Judge #%d enter\n", jud_no);

	//wait for unchecked immigrants
	for (int i = 0; i < uncheck; ++i)
		sem_wait(&check);
	
	if (unconfirmNum != 0)
	{
		do{
			sem_wait(&sitDown);
			int imm_no = pop(&Sitdown);
			shortdelay();
			printf("Judege #%d confrm the immigrant #%d \n", jud_no, imm_no);
			--unconfirmNum;

			sem_post(&immConfirmed[imm_no]);
		}while(unconfirmNum > 0);
	}
	shortdelay();
	printf("Judge #%d leave\n", jud_no);
	sem_post(&judIn);
	pthread_exit(NULL);
}

int Nthread = 0;
pthread_t *threads;

void new_thread(int tid)
{
	switch(rand()%3)
	{
		case 0:pthread_create(&threads[tid], NULL, func_immigrant, NULL);break;
		case 1:pthread_create(&threads[tid], NULL, func_spectator, NULL);break;
		case 2:pthread_create(&threads[tid], NULL, func_judge, NULL);break;
	}
}

int main ()
{
	srand(time(NULL));
	threads = malloc(MAX_THREAD *sizeof(pthread_t));
	queue_init(&Sitdown, 500);
	sem_init(&judIn, 0, 1);
	sem_init(&immEnter, 0, 0);
	sem_init(&check, 0, 0);
	sem_init(&sitDown, 0, 0);
	int i;
	for (i = 0; i<MAX_THREAD; ++i)
		sem_init(&immConfirmed[i], 0, 0);
	while(1)
	{
		new_thread(Nthread);
		++Nthread;
		delay();
	}
	free_vector(&Sitdown);
	return 0;
	
}
