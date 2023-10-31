#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <iostream>

#define MAX_CLIENTS 5

pthread_t thread_pool[MAX_CLIENTS];
pthread_mutex_t pool_mutex;

struct thread_data
{
    int sockfd;
}

void *client_handler(void *thread_arg)
{
    //handle the transition

    pthread_mutex_lock(&pool_mutex);
    pthread_t cur_pid = pthread_self();
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if(thread_pool[i] == cur_pid)
        {
            printf("Erase the thread[%d]: %d.\n", i, cur_pid);
            thread_pool[i] == -1;
            break;
        }
    }
    pthread_mutex_unlock(&pool_mutex);
    pthread_exit(NULL);
}

int main()
{
    for (int i = 0; i < MAX_CLIENTS; i++)
        thread_pool[i] = -1;
    pthread_mutex_init(&pool_mutex, NULL);

    time_t clock;

    char data_sending[1025];
    int client_listen = 0;
    int client_connect = 0;
    struct sockaddr_in ip_of_server, new_addr;
    client_listen = socket(AF_INET, SOCK_STREAM, 0);

    memset(&ip_of_server, '0', sizeof(ip_of_server));
    memset(data_sending, '0', sizeof(data_sending));

    ip_of_server.sin_family = AF_INET;
    ip_of_server.sin_addr.s_addr = htonl(INADDR_ANY);
    ip_of_server.sin_port = htons(8888);

    if (bind(client_listen, (struct sockaddr*)&ip_of_server, sizeof(ip_of_server)) < 0)
    {
        printf("Error bind.\n");
        return 1;
    }

    listen(client_listen, MAX_CLIENTS);
    printf("listening...\n");
    
    socklen_t addr_size;
    addr_size = sizeof(new_addr);

    while(1)
    {
        //accept a client
        printf("\n\nHi, I am running a server.\n");
        client_connect = accept(client_listen, (struct sockaddr*)&new_addr, &addr_size);
        if (client_connect < 0)
        {
            perror("Accept error\n");
            continue;
        }

        //check whether there is an available thread
        int available_thread_index = -1;
        pthread_mutex_lock(&pool_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (thread_pool[i] == -1)
            {
                available_thread_index = i;
                break;
            }
        }
        pthread_mutex_unlock(&pool_mutex);

        //create new thread
        if (available_thread_index > -1)
        {
            //exist an idle thread
        }
        else
        {
            //server is busy, turn down the request
        }

        char IPaddr[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &(new_addr.sin_addr), IPaddr, INET_ADDRSTRLEN) == NULL)
        {
            perror("IP conversion error.\n");
            return 1;
        }
        int clientPort = ntohs(new_addr.sin_port);
        printf("client connected: ip:<%s>, port:%d", IPaddr, clientPort);
    
        clock = time(NULL);
        snprintf(data_sending, sizeof(data_sending), "%.24s\r\n", ctime(&clock));
        write(client_connect, data_sending, strlen(data_sending));

        close(client_connect);
        sleep(1);
    }

    return 0;
}