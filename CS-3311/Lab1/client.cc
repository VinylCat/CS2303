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
#include <iostream>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>


struct file_trans
{
    char filename[50];
    long len;
};


int main()
{
    int create_socket = 0, n = 0;
    char data_received[1024];
    int bytes_received;
    struct sockaddr_in ip_of_server;

    memset(data_received, '0', sizeof(data_received));

    char folderName[100];
    printf("Enter the folder name: ");
    scanf("%s", folderName);

    if (mkdir(folderName, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
        perror("Error creating folder");
        return 1; // Return an error code
    }
    char filePath[200]; // Assuming a maximum path length of 200 characters
    sprintf(filePath, "%s/test.txt", folderName);
    FILE *file = fopen(filePath, "wb");
    if (file == NULL)
    {
        perror("Fail to open the file.\n");
    }

    if ((create_socket =socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket not created.\n");
        return 1;
    }
    std::cout<<"Please type in the port number: \n";
    int port_number;
    std::cin>>port_number;
    ip_of_server.sin_family = AF_INET;
    ip_of_server.sin_port = htons(port_number);
    ip_of_server.sin_addr.s_addr = inet_addr("10.0.0.1");
    

    if (connect(create_socket, (struct sockaddr*)&ip_of_server, sizeof(ip_of_server)) < 0)
    {
        perror("Connect error.\n");
        return 1;
    }
    time_t begin, end;
    begin = time(NULL);

    bytes_received = recv(create_socket, data_received, 26, 0);
    if (bytes_received == -1)
    {
        perror("Fail to receive.\n");
        return 1;
    }
    data_received[bytes_received] = '\0';
    printf("%s", data_received);

    std::cout<<"time len: "<<bytes_received<<std::endl;
    
    if (data_received[0] == 'B' && data_received[1] == 'u' && 
        data_received[2] == 's' && data_received[3] == 'y')
    {
        printf("Fail to connect: Server busy.\n");
        return 0;
    }
    
    struct file_trans ft;
    bytes_received = recv(create_socket, &ft, sizeof(ft), 0);
    if (bytes_received != sizeof(ft))
    {
        perror("Fail to receive file head.\n");
        return 1;
    }
    std::cout<<"recv len:"<<bytes_received<<"\nft size:"<<sizeof(ft)<<std::endl;
    
    std::cout<<"file name: "<<ft.filename<<" length: "<< ft.len<<std::endl;
    
    
    bzero(data_received, 1024);

    long file_received = 0;
    while (file_received < ft.len )
    {
        
        int bytes_read = recv(create_socket, data_received, std::min<long>(1024, ft.len-file_received), 0);
        if (bytes_read < 0)
        {
            perror("Receive file error.\n");
            break;
        }
        
        if (bytes_read != 1024)
            printf("Received %d bytes in hex:\n", bytes_read);
        /*
        for (int i = 0; i < bytes_read; i++)
        {
            printf("%02x ", data_received[i]);
        }
        printf("\n");
        */
        fwrite(data_received, 1, bytes_read, file);
        file_received += bytes_read;
        bzero(data_received, 1024);
    }

    fclose(file);
    end = time(NULL);
    //
    bytes_received = recv(create_socket, data_received, sizeof(data_received), 0);
    std::cout<<data_received<<std::endl;
    std::cout<<"time used: "<<end - begin<<"s.\n";
    return 0;
}