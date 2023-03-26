//可以通过./single运行程序，读入data.in的文件，将结果写入data.out中
//！！！输出的结果第一行是1based，即3x3会输出3
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

int main(int argc, char *argv[])
{
	char file_src[50];
	char cwd[1024];
	int num;
	char *line = NULL;
	size_t line_size = 0;
	ssize_t line_len;
	char *ptr;
	
	// 获取当前工作目录的路径
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
		perror("getcwd() error");
		exit(EXIT_FAILURE);
	}
	
	// 构造文件名
	strcpy(file_src, cwd);
	strcat(file_src, "/Matrix1000.in");
	
	// 打开文件
	FILE *src;
	src = fopen(file_src, "r");
	if (src == NULL) {
		perror("fopen() error");
		exit(EXIT_FAILURE);
	}
	
	
	// 读取第一行文本
	line_len = getline(&line, &line_size, src);
	if (line_len == -1) 
	{
  	  	perror("getline() error");
  	  	exit(EXIT_FAILURE);
	}

	// 解析第一行文本，获取数字
	num = strtol(line, NULL, 10);
	printf("The size is %dx%d.\n", num, num);
	
	// 动态分配数组的空间
	// 声明指向二维 int 数组的指针
	int **arr;
	arr = (int **)malloc(num * sizeof(int *));
    	for (int i = 0; i < num; i++) 
    	{
        	arr[i] = (int *)malloc(num * sizeof(int));
    	}
    	for (int i = 0; i < num; i++) 
    	{
        	for (int j = 0; j < num; j++) 
        	{
        	    arr[i][j] = 0;
        	}
    	}
	for (int k = 0; k < num; k++)
	{
		line_len = getline(&line, &line_size, src);
		if (line_len == -1) 
		{
			perror("getline() error");
			exit(EXIT_FAILURE);
		}	

		ptr = line;
		for (int i = 0; i < num; i++) 
		{
			char* endptr = strchr(ptr, ' ');
			if (endptr != NULL)
			{
				*endptr = '\0';
				arr[k][i] = strtol(ptr, NULL, 10);
				ptr = endptr + 1;
			
			}
			else
			{
				arr[k][i] = strtol(ptr, NULL, 10);
			}
		}
	}
	// 打印数组的内容 for test
	/*
	for (int i =0; i< num; i++)
	{
		for (int j = 0; j < num; j++) 
		{
			printf("%d ", arr[i][j]);
		}
		printf("\n");
	}
	*/
	
	int **ans;
	ans = (int **)malloc(num * sizeof(int *));
    	for (int i = 0; i < num; i++) 
    	{
        	ans[i] = (int *)malloc(num * sizeof(int));
    	}
    	for (int i = 0; i < num; i++) 
    	{
        	for (int j = 0; j < num; j++) 
        	{
        	    ans[i][j] = 0;
        	}
    	}
    	//开始计算
    	double start = clock();
    	for (int i = 0; i < num; ++i)
    	{
    		for (int j = 0; j < num; j++)
    		{
    			int sum = 0;
    			for(int k =0;k < num; k++)
    			{
    				sum += arr[i][k] * arr[k][j];
    			}
    			ans[i][j] = sum;		
    		}
    	}
    	double end = clock();
    	//print
	for (int i =0; i< num; i++)
	{
		for (int j = 0; j < num; j++) 
		{
			printf("%d ", ans[i][j]);
		}
		printf("\n");
	}
	double time_consumed = (end - start)/CLOCKS_PER_SEC *1000;
	printf("Time consumed: %f\n millisecond", time_consumed);
	// 释放内存并关闭文件
	for (int i = 0; i < num; i++) 
	{
        	free(arr[i]);
    	}
   	free(arr);
	for (int i = 0; i < num; i++) 
	{
        	free(ans[i]);
    	}
   	free(ans);
	free(line);
	fclose(src);
	
	return 0;
}


