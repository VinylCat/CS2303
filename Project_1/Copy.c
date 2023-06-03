//利用管道实现了copy的功能
//你可以使用./Copy <src.txt> <dest.txt> <buffersize> 执行
//文件夹中已经提供了src.txt文件，每次复制会重新更新dest.txt内容
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
int main(int argc, char **argv)
{
	char line[100];
	char file_src[50], file_dest[50];
	int buf_size;
	char* buffer;
	clock_t start, end;
	double elapsed;
	
	if (argc != 4)
	{
		printf("Usage:%s <source_file> <destination_file> <buffer_size>\n", argv[0]);
		return -1;
	}
	strncpy(file_src, argv[1], sizeof(file_src));
	strncpy(file_dest, argv[2], sizeof(file_dest));
	buf_size = atoi (argv[3]);
	buffer = (char*) malloc(buf_size* sizeof(char));
		
	if (buffer == NULL)
	{
		printf("Failed to allocate memory for buffer.\n");
		return -1;
	}
	
	int mypipe[2];
	if (pipe(mypipe))
	{
		printf("Error: Failed to create the pipe.\n");
		return -1;
	}

	pid_t pid =fork();
	start = clock();
	if (pid < 0)
	{
		printf("Failed to fork.\n");
		return -1;
	}
	if (pid == 0)
	{
		//子进程writer
		close(mypipe[0]);
		FILE *src;
		src = fopen(file_src, "r");
		if (src == NULL)
		{
			printf("Failed to open the file.\n");
			fclose(src);
			return -1;
		}		
		size_t nread;
		size_t sum =0; 
		while ((nread = fread(buffer, 1, buf_size, src)) > 0)
		{
		    	write(mypipe[1], buffer, nread);
		  	sum += nread;
		   	if (nread < buf_size) break; // 文件读取完毕
		}
		printf("THe total is %ld.\n",sum);
		fclose(src);
				
	}
	if (pid > 0)
	{
		close(mypipe[1]);
		FILE *dest;
		dest = fopen(file_dest, "w+");
		if (dest == NULL)
		{
			printf("Filed to write the file.\n");
			fclose(dest);
			return -1;
		}
		size_t nwritten;
	    	while((nwritten = read(mypipe[0], buffer, buf_size)) > 0)
		{
			fwrite(buffer, 1, nwritten, dest); // 只写入实际读取的字节数
		}
		fclose(dest);
		end = clock();
		elapsed =( (double)(end - start))/CLOCKS_PER_SEC *1000;
		printf("Time used:%f millisecond.\n",elapsed);
	}
	
	close(mypipe[0]);
	close(mypipe[1]);
	return 0;
}
