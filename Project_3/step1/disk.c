#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include <fcntl.h>
#define MAX_COMMANDS 256


int main(int argc, char  **argv)
{
    char filename[50];   
    if (argc != 5)
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
    

    //test
    //memcpy(&diskfile[BLOCKSIZE * (2 * SECTORS + 1)], "Hello World!", strlen("Hello World!"));//write
    //memcpy (buffer, &diskfile[BLOCKSIZE * (2 * SECTORS + 1)], BLOCKSIZE);//read
    close(fd);
    char line[MAX_COMMANDS];
    char data[SECTORS];
    while (1) 
    {
        printf("请输入指令：\n");
        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("读取输入时出错\n");
            break;
        }
        int c, s;
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
        
        
    }
}


