#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

int main()
{
    int create_socket = 0, n = 0;
    char data_received[1024];
    struct sockaddr_in ip_of_server;

    memset(data_received, '0', sizeof(data_received));

    if ((create_socket =socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket not created.\n");
        return 1;
    }

    ip_of_server.sin_family = AF_INET;
    ip_of_server.sin_port = htons(8888);
    ip_of_server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(create_socket, (struct sockaddr*)&ip_of_server, sizeof(ip_of_server)) < 0)
    {
        //printf("Connection failed due to port and ip problems.\n");
        perror("Connect error.\n");
        return 1;
    }

    while ((n = read(create_socket, data_received, sizeof(data_received) -1)) > 0)
    {
        data_received[n] = 0;
        if(fputs(data_received, stdout) == EOF)
        {
            printf("\nStandard putput error.");
        }

        printf("\n");
    }

    if (n < 0)
    {
        printf("Standard input error.\n");
    }
    return 0;
}