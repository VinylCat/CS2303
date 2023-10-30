#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include <netdb.h>
#include <ctype.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#define MAX_COMMANDS 256

typedef struct 
{
    int rw;//0 for r, 1 for w ;2 for formatted
    int beginPos;
    int size;
    int oriPos;
}TRANS;
typedef struct //28 bytes in all. So a block can hold 8 files inall.
{
    uint32_t f_inodeNum;
    char f_fileName[24];
    int a;
}FILEDIRECTORY;


int main(int argc, char  **argv)
{
    char filename[50];   
    if (argc != 6)
    {
        printf("Invlaid input!\n");
        return -1;
    }
    strncpy(filename, argv[4], sizeof(filename));
    int CYLINEDERS = atoi(argv[1]);
    int SECTORS = atoi(argv[2]);//sectors per cylinder
    int BLOCKSIZE = 256;
    int DELAY = atoi(argv[3]);
    char buffer[256] = {0};

    int fd = open (filename, O_RDWR | O_CREAT, 0);
    if (fd < 0)
    {
        printf("ERROR: Could not open files '%s'.\n", filename);
        return -1;
    }
    long FILESIZE = BLOCKSIZE * SECTORS * CYLINEDERS;
    //strectch the file size to size of the simulated disk   
    int res = lseek(fd, FILESIZE - 1, SEEK_SET);
    if (res == -1)
    {
        perror("ERROR calling lseek() to 'stretch' the file");
        close (fd);
        return -1;
    }
    res = write (fd, "", 1);
    if (res != 1) 
    {
        perror("Error writing last byte of the file");
        close(fd);
        exit(-1);
    }
    char * diskfile;
    diskfile = (char* )mmap(NULL, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (diskfile == MAP_FAILED)
    {
        close(fd);
        printf("ERRORL Could not map file.\n");
        return -1;
    }
    FILE* fp = fopen ("disk.log", "w");

    close(fd);
    char line[MAX_COMMANDS];
    char data[SECTORS];
    //handle the socket
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    portno = atoi(argv[5]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        herror("ERROR opening socket");
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) < 0)
        herror("ERROR on binding");
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    char buf[sizeof(TRANS)];
    int formatted = 0;
    printf("Connected successfully\n");
    int c, s, lastc, lasts;
    c = s = lastc = lasts =0;
    while (1) 
    {
        
        //handle the receiving content 
        bzero(buf, sizeof(TRANS));
        n = read(newsockfd, buf, sizeof(TRANS));
        if (n < 0)
        {
            printf("Error: fail to communicate with fs\n");
            continue;
        }
        else if(n == 0) 
        {
            printf("Oops!Connection failed.\n");
            newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
            printf("New connection is built.\n");      
        }
        TRANS t;
        memcpy(&t, buf, sizeof(TRANS));
        //&diskfile[BLOCKSIZE * (c * SECTORS + s)]
        c = (t.beginPos/BLOCKSIZE)/SECTORS;
        s = (t.beginPos/BLOCKSIZE)%SECTORS;
        int delay = abs(c-lastc)*DELAY;
        printf("c: %d s: %d DELAY: %d\n",c,s,delay);
        lastc = c;
        lasts = s;
        char* content = NULL;
        switch(t.rw)
        {
            case -1:
                printf("Invalid data\n");
                n = write(newsockfd, "0\n", strlen("0\n"));
                if (n < 0)
                {
                    printf("Error: fail to communicate with disk\n");
                    n = write(newsockfd, "0\n", strlen("0\n"));
                }
                break;
            case 0://read
                if (formatted != 1 )
                {
                    printf("Error: not formatted yet\n");
                    break;
                }
                
                content = malloc(t.size * sizeof(char));
                memcpy(content, &diskfile[t.beginPos], t.size);
            
                n = write(newsockfd, content, t.size);
                if (n < 0)
                {
                    printf("Error: fail to communicate with disk\n");
                }
                free(content);
                fprintf(fp, "Yes\n");
                break;

            case 1://write
                if (formatted != 1 )
                {
                    n = write(newsockfd, "0\n", strlen("0\n"));
                    printf("Error: not formatted yet\n");
                    break;
                }
                n = write(newsockfd, "1\n", strlen("1\n"));
                if (n < 0)
                {
                    printf("Error: fail to communicate with disk\n");
                    break;
                }
                bzero(buf,sizeof(TRANS));
                
                content = malloc(t.size * sizeof(char));
                n = read(newsockfd, content, t.size);
                if (n < 0)
                {
                    printf("Error: fail to communicate with disk\n");
                    break;
                }
                memcpy(&diskfile[t.beginPos], content, t.size); 
                n = write(newsockfd, "1\n", strlen("1\n"));
                fprintf(fp, "Yes\n");
                free(content);
                break;
            case 2:
                formatted = 1;
                if (SECTORS * CYLINEDERS < 162)
                {
                    printf("Error: disk size is too small\n");
                    n = write(newsockfd, "0\n", strlen("0\n"));
                    break;
                }
                else
                {
                    n = write(newsockfd, "1\n", strlen("1\n"));
                    fprintf(fp, "%d %d\n",CYLINEDERS, SECTORS);
                    break;
                }
            case 3://move
                if (formatted != 1 )
                {
                    n = write(newsockfd, "0\n", strlen("0\n"));
                    printf("Error: not formatted yet\n");
                    break;
                }
                memcpy(&diskfile[t.beginPos], &diskfile[t.oriPos], t.size);
                n = write(newsockfd, "1\n", strlen("1\n"));
                if (n < 0)
                {
                    printf("Error: fail to communicate with disk\n");
                }
                break;
            case 4://clear
                if (formatted != 1 )
                {
                    n = write(newsockfd, "0\n", strlen("0\n"));
                    printf("Error: not formatted yet\n");
                    break;
                }
                memset(&diskfile[t.beginPos], 0, t.size);
                n = write(newsockfd, "1\n", strlen("1\n"));
                if (n < 0)
                {
                    printf("Error: fail to communicate with disk\n");
                }
                break;
            case 5:
                printf("退出程序\n");
                fprintf(fp,"end\n");
                if (munmap(diskfile, CYLINEDERS * SECTORS * BLOCKSIZE) == -1)
                {
                    printf("Error unmapping disk storage file\n");
                    return 1;
                }
                n = write(newsockfd, "1\n", strlen("1\n"));
                if (n < 0)
                {
                    printf("Error: fail to communicate with disk\n");
                }
                return 0;

        }

        //disk operation
        /*
        switch (line[0])
        {
            case 'I':
                fprintf(fp, "%d %d\n",CYLINEDERS, SECTORS);
                break;
            case 'R':
                sscanf(line, "R %d %d", &c, &s);
                if (c > CYLINEDERS || s > SECTORS)
                {
                    fprintf(fp, "No\n");
                    continue;
                }

                memcpy (buffer, &diskfile[BLOCKSIZE * (c * SECTORS + s)], BLOCKSIZE);//read
                if (buffer[0] == 0)
                {
                    fprintf(fp, "Yes\n");
                }
                else 
                {
                    fprintf(fp, "Yes ");
                    int i = 0;
                    while (buffer[i])
                    {
                        fprintf(fp, "%c", buffer[i]);
                        i++;
                    }
                    fprintf(fp, "\n");
                }
                break;
            case 'W':
                sscanf(line, "W %d %d %[^\n]", &c, &s, data);
                if (c > CYLINEDERS || s > SECTORS)
                {
                    fprintf(fp, "No\n");
                    continue;
                }
                memcpy (&diskfile[BLOCKSIZE * (c * SECTORS + s)], data, strlen(data));
                fprintf(fp, "Yes\n");

                break;
            case 'E':
                printf("退出程序\n");
                fprintf(fp,"end\n");
                if (munmap(diskfile, CYLINEDERS * SECTORS * BLOCKSIZE) == -1)
                {
                    printf("Error unmapping disk storage file\n");
                    return 1;
                }  
                return 0;
            default:
                printf("Commands error. Please retype in the cpmmands.\n");
                break;
        }
        */
        
    }
}
