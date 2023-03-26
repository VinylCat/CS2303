//实现了多线程计算矩阵的功能
//可以通过更改Numofthread来设置线程数量，MAX来设置随机生成的矩阵的最大值
//可以通过./multi 运行，此时程序会默认对data.in文件中的矩阵进行自乘，结果输出到data.out中。
//！！！注意，data.out第一行代表矩阵的size，从1开始计数，若为3x3矩阵，则第一行输出是3
//可以通过。/multi <size>运行，此时程序会自动生成两个相应大小的矩阵进行乘法运算，结果会写入random.out中。第一行是size，计数方法同样是1based
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define Numofthread 4 
#define MAX 100

int **arr1;
int **arr2;
struct thread_arguments
{
	int task;//记录分块矩阵
	int col;//计算行列
	int row;
	int tid;//记录当前id
	int** ans;
};

void *my_function(void *args)//calculate Matrix here
{
	
	struct thread_arguments *a;
	a = (struct thread_arguments *)args;
	int Task = a->task ;//sum
	int Col = a->col;//num
	int Row = a->row;//eachtask[i]
	int id = a->tid;
	int **Ans = a->ans;
	for (int i = 0; i < Row; ++i)
	{
		for (int j = 0; j < Col; ++j)
		{
			int sum = 0;
			for (int k = 0; k < Col; ++k)
			{
				sum += arr1[Task + i][k]* arr2[k][j];
			}
			Ans[i][j] = sum;
		}
	}
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{	
	
	char file_src[50];
	char cwd[1024];
	int num = 0;
	char *line = NULL;
	size_t line_size = 0;
	ssize_t line_len;
	char *ptr;
	
	
	//const int Numofthread = 16;//设置进程数量
	pthread_t mythread[Numofthread];
	pthread_attr_t attr[Numofthread];
	struct thread_arguments myarguments[Numofthread];
	for (int i = 0; i< Numofthread; ++i)
	{
		pthread_attr_init(&attr[i]);
		pthread_attr_setdetachstate(&attr[i], PTHREAD_CREATE_JOINABLE);	
		pthread_attr_setscope(&attr[i], PTHREAD_SCOPE_SYSTEM);
	}
	int eachTask[Numofthread] = {0};
	// 获取当前工作目录的路径
	FILE *src;
	if (argc == 1)
	{
		if (getcwd(cwd, sizeof(cwd)) == NULL) 
		{
			perror("getcwd() error");
			exit(EXIT_FAILURE);
		}
	    
	
	
		// 构造文件名
		strcpy(file_src, cwd);
		strcat(file_src, "/data.in");
		
		// 打开文件
		
		src = fopen(file_src, "r");
		if (src == NULL) 
		{
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
		num = strtol(line, NULL, 0);
		printf("The size is %dx%d.\n", num, num);
	
		// 动态分配数组的空间
		// 声明指向二维 int 数组的指针
 		arr1 = (int **)malloc(num * sizeof(int *));
	    	for (int i = 0; i < num; i++) 
 	   	{
	        	arr1[i] = (int *)malloc(num * sizeof(int));
	    	}
	    	for (int i = 0; i < num; i++) 
	    	{
	        	for (int j = 0; j < num; j++) 
	        	{
	        	    arr1[i][j] = 0;
	        	}
	    	}
	    	arr2 = (int **)malloc(num * sizeof(int *));
	    	for (int i = 0; i < num; i++) 
 	   	{
	        	arr2[i] = (int *)malloc(num * sizeof(int));
	    	}
	    	for (int i = 0; i < num; i++) 
	    	{
	        	for (int j = 0; j < num; j++) 
	        	{
	        	    arr2[i][j] = 0;
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
					arr1[k][i] = strtol(ptr, NULL, 10);
					arr2[k][i] = arr1[k][i];
					ptr = endptr + 1;
				
				}
				else
				{
					arr1[k][i] = strtol(ptr, NULL, 10);
					arr1[k][i] = arr2[k][i];
				}
			}
		}
	

	}//argc = 1;
	else if (argc == 2)
	{
		num = atoi(argv[1]);
		srand((unsigned)time(NULL));   //用时间作种子
		arr1 = (int **)malloc(num * sizeof(int *));
	    	for (int i = 0; i < num; i++) 
 	   	{
	        	arr1[i] = (int *)malloc(num * sizeof(int));
	    	}
	    	for (int i = 0; i < num; i++) 
	    	{
	        	for (int j = 0; j < num; j++) 
	        	{
	        	    arr1[i][j] = rand()%MAX + 1;
	        	}
	    	}
		arr2 = (int **)malloc(num * sizeof(int *));
	    	for (int i = 0; i < num; i++) 
 	   	{
	        	arr2[i] = (int *)malloc(num * sizeof(int));
	    	}
	    	for (int i = 0; i < num; i++) 
	    	{
	        	for (int j = 0; j < num; j++) 
	        	{
	        	    arr2[i][j] = rand()%MAX + 1;
	        	}
	    	}
	}
	else 
	{
	 	printf("Invalid arguments\n");
        	return 1;
	}
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
    	
    	int factor = num % Numofthread;
    	if (factor == 0)
    	{
    		for (int i = 0; i < Numofthread; ++i)
    			eachTask[i] = num/Numofthread;
    	}
    	else 
    	{
    		for (int i = 0; i < Numofthread - 1; ++i)
    			eachTask[i] = num/Numofthread;
    		eachTask[Numofthread - 1] = num/Numofthread + factor;
    	}
    	//准备传入参数
    	for (int i = 0; i < Numofthread; i++)
    	{	
    		int sum = 0;
    		for (int j = 0; j < i; j++)
    			sum += eachTask[j];
    		myarguments[i].task = sum;
    		myarguments[i].row = eachTask[i]; 
    		myarguments[i].col = num;
    		myarguments[i].tid = i;
    		myarguments[i].ans = (int **)malloc(sizeof(int*) * eachTask[i]);
    		for (int j = 0; j < eachTask[i]; ++j)
    			myarguments[i].ans[j] = malloc(sizeof(int) * num);
    		
    	}
    	//创建多线程
    	double start = clock();
    	for (int i = 0; i< Numofthread; ++i)
    	{
    		int rc;
    		rc = pthread_create(&mythread[i], &attr[i], my_function, &myarguments[i]);
    		if (rc)
    		{
    			printf("ERROR:return code error\n");
    			exit(-1);
    		}
    	}
    	//回收多线程
    	for (int i = 0; i< Numofthread; ++i)
    	{
    		void *status;
    		int rc = pthread_join(mythread[i], &status);
    		if (rc)
    		{
    			printf("ERROR: Join error\n");
    			exit(-1);
    		}
    	}
    	//print
    	/*
    	
    	for (int k = 0; k < Numofthread; k++)
    	{
		for (int i =0; i< myarguments[k].row; ++i)
		{
			for (int j = 0; j < myarguments[k].col; ++j) 
			{
				printf("%d ", myarguments[k].ans[i][j]);
			}
			printf("\n");
		}	
	}
	*/
	double end = clock();
	double time_consumed = (end - start)/CLOCKS_PER_SEC * 1000;
	printf("Time consumed: %f milliseconds.\n", time_consumed);
	//输出random
	if (argc == 2)
	{
		FILE *output = fopen("random.out", "w");
		if (output == NULL)
		{
			printf("Filed to create random.out\n");
			return 1;
		}
		fprintf(output,"%d\n",num);
		for(int i = 0; i < num; i++)
		{
			for (int j = 0; j < num; ++j)
			{
				fprintf(output, "%d ", arr1[i][j]);
			}
			fprintf(output, "\n");
		}
		for(int i = 0; i < num; i++)
		{
			for (int j = 0; j < num; ++j)
			{
				fprintf(output, "%d ", arr2[i][j]);
			}
			fprintf(output, "\n");
		}
		for (int k = 0; k < Numofthread; k++)
	    	{
			for (int i =0; i< myarguments[k].row; ++i)
			{
				for (int j = 0; j < myarguments[k].col; ++j) 
				{
					fprintf(output,"%d ", myarguments[k].ans[i][j]);
				}
				fprintf(output,"\n");
			}	
		}
		fclose(output);
		//输出样例
		/*
		FILE *sample = fopen("sample.in", "w");
		if (output == NULL)
		{
			printf("Filed to create random.out\n");
			return 1;
		}
		fprintf(sample,"%d\n",num);
		for(int i = 0; i < num; i++)
		{
			for (int j = 0; j < num; ++j)
			{
				fprintf(sample, "%d ", arr1[i][j]);
			}
			fprintf(sample, "\n");
		}
		fclose(sample);
		*/
	}
	else
	{
		FILE *output = fopen("data.out", "w");
		if (output == NULL)
		{
			printf("Filed to create data.out\n");
			return 1;
		}
		fprintf(output,"%d\n",num);
		for(int i = 0; i < num; i++)
		{
			for (int j = 0; j < num; ++j)
			{
				fprintf(output, "%d ", arr1[i][j]);
			}
			fprintf(output, "\n");
		}
		for(int i = 0; i < num; i++)
		{
			for (int j = 0; j < num; ++j)
			{
				fprintf(output, "%d ", arr2[i][j]);
			}
			fprintf(output, "\n");
		}
		for (int k = 0; k < Numofthread; k++)
	    	{
			for (int i =0; i< myarguments[k].row; ++i)
			{
				for (int j = 0; j < myarguments[k].col; ++j) 
				{
					fprintf(output,"%d ", myarguments[k].ans[i][j]);
				}
				fprintf(output,"\n");
			}	
		}
		fclose(output);
	}	
	
	
	// 释放内存并关闭文件

	for (int i = 0; i < num; i++) 
	{
        	free(ans[i]);
    	}
   	free(ans);
   	for (int i = 0; i < Numofthread; i++) 
	{
        	for(int j = 0; j < eachTask[i]; j++)
        	{
        		free(myarguments[i].ans[j]);
        	}
        	free(myarguments[i].ans);
    	}
	for (int i = 0; i < num; i++) 
	{
        	free(arr1[i]);
 	}
 	free(arr1);
 	for (int i = 0; i < num; i++) 
	{
      		free(arr2[i]);
  	}
	free(arr2);
	free(line);
	return 0;
}

