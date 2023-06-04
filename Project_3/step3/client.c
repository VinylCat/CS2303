#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include <sys/wait.h>

int main(int argc, char  **argv)
{
    
	int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[2048];
    if (argc != 2)
    {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }
    portno = atoi(argv[1]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        herror("ERROR opening socket");
    server = gethostbyname("localhost");
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        herror("ERROR connecting");
    bzero(buffer, 2048);
    while(1)
    {
        printf("Please type in the command\n");
        fgets(buffer, 2048, stdin);
        n = write (sockfd, buffer, strlen(buffer));
        if ( n < 0)
            herror("Error: fail to wirte to socket\n");
        bzero(buffer, 2048);
        n = read(sockfd, buffer, 2047);
        if(n < 0)
            herror("Error: fail to read from the socket\n");
        if (strcmp(buffer, "E\n") == 0)
        {
            printf("Goodbye!\n");
            close(sockfd);
            return 0;
        }
        printf("%s", buffer);
        bzero(buffer, 2048);       
    }
    close(sockfd);
    return 0;
}