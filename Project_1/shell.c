//通过./shell <Port>运行
//支持pwd，cd，ls，wc等指令
//支持管道，数量上限为5

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_COMMANDS 300
#define MAX_COMMANDS_LENGTH 100
#define MAX_PIPES 5

void delchar(char* str, const size_t max_len)
{
	char strtmp[max_len];
	const size_t nlen = strlen(str);
	size_t ptr = 0;
	for (size_t i = 0; i < nlen; i++)
		if (str[i] != 10 && str[i] != 13)
			strtmp[ptr++] = str[i];
	for (size_t i = 0; i < ptr; i++)
		str[i] = strtmp[i];
	str[ptr] = 0;
}


int parseLine(char*line, char*command_array[],int *num_pipe_commands)
{
    char *p;
    int count = 0;
    *num_pipe_commands = 0;

    p = strtok(line," ");
    while(p)
    {
        if (strcmp(p, "|") == 0) 
        {
            *num_pipe_commands += 1;
            command_array[count] = p;
            count++;
        } 
        else 
        {
            command_array[count] = p;
            count++;
        }
        p = strtok(NULL, " ");
    }
    
    return count;
}

int main(int argc, char  **argv)
{
	int sockfd, portno, client_num = 0;
	portno = atoi(argv[1]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("ERROR opening socket");
		exit(2);
	}
	
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(portno);
	int bindfd;
	if( bindfd = bind (sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("ERROR bind");
		exit(2);
	}
	
	listen(sockfd, 5);
	
	int client_sockfd;
	struct sockaddr_in client_addr;
	int len = sizeof(client_addr);
	
	while(1)
	{	

		client_sockfd = accept(sockfd, (struct sockaddr*)&client_addr, &len);
		client_num++;
		if(client_sockfd < 0)
		{
			perror("ERROR client_socket");
			exit(2);
		}		
		
		pid_t pid = fork();//接受client，创建子进程
		if (pid < 0)
		{
			perror("ERROR on fork.");
			exit(2);
		}
		else if (pid == 0)//子进程
		{
			close (sockfd);
			printf("Get a new connection, client number: %d.\n", client_num);
			const char *message1 = "I can hear youuuuu!\n";
			send (client_sockfd, message1, strlen(message1),0);
			
			while(1)
			{
				char buf[1024];
				int nread = read(client_sockfd, buf, 1024);
				buf[nread] = '\0';				
				const size_t nread_len = strlen(buf);
				delchar(buf, nread_len + 1);
				//处理exit
				
				const char *message_exit = "GOODBYE!\n";
				if (strcmp(buf, "exit") == 0) 
				{
					printf("One client is disconnected.\n");
					send (client_sockfd, message_exit, strlen(message_exit),0);
    					close(client_sockfd);
    					exit(0);
				}			
				
				
				
				char *commands[MAX_COMMANDS];
				int *pipe_count = malloc(sizeof(int));
				*pipe_count = 0;

				int num_commands = parseLine(buf, commands, pipe_count);

				
				commands[num_commands] = NULL;
				
				
				if (num_commands == -1)
					continue;
				
				//没有管道符号
				if (*pipe_count == 0)
				{
					
					printf("No pipe commands dectected.\n");
					if (commands[0] != NULL)
					{
						if (strcmp(commands[0], "cd") == 0)//cd
						{
							int error = 0;
							if (commands[1] == NULL)//cd 没有参数
							{
								 const char *home = getenv("HOME"); // 获取 HOME 环境变量
								if (home == NULL) 
								{
									send(client_sockfd, "Chdir failed.\n", strlen("Chdir failed.\n"), 0);
									error = 1;
								}
							    	int ret = chdir(home); // 将工作目录更改为 HOME 目录
								if (ret == -1) 
								{
									send(client_sockfd, "Chdir failed.\n", strlen("Chdir failed.\n"), 0);
									error = 1;
								}
							}
							else
							{
								if (chdir(commands[1]) == -1)
								{
									send(client_sockfd, "Invalid path.\n", strlen("Invalid Path.\n"), 0);
									error = 1;
								}
							}
							if(error == 0) //正常执行， 返回当前目录给用户
							{
								char cwd[1024];
								char *ret = getcwd(cwd, sizeof(cwd));
								if (ret == NULL) 
								{
	    								send(client_sockfd, "Failed to get path\n.", strlen("Failed to get path\n."), 0);
								}
								else
								{
									send(client_sockfd, "Your path now:.\n", strlen("Your path now:.\n"), 0);
									send(client_sockfd, ret, strlen(ret), 0);
									send(client_sockfd, "\n", strlen("\n"), 0);
								}
							}
						}
						else
						{
							//建立子进程执行命令
							int output[2];
							if (pipe(output) == -1)
							{
								perror("pipe create error");
								exit(2);
							}
							
							pid_t sub_pid = fork();
							if (sub_pid < 0)
							{
								perror("ERROR on the secondfork.");
								exit(2);
							}
							else if (sub_pid > 0)//父进程
							{
								close(output[1]);
								int status;
								waitpid(sub_pid, &status, 0);
								int ret = WEXITSTATUS(status);
								int sign = 0;
								if (ret != 0)
								{
									sign = 1;
									send(client_sockfd, "Commands Error\n",strlen("Commands Error\n"), 0);
								}
								if (!sign)
								{
									char buffer[8192]={0};
									if (read (output[0], buffer, 8191) <= 0)
										send(client_sockfd, "Commands Error\n",strlen("Commands Error\n"), 0);
									else
										send (client_sockfd, buffer, strlen(buffer), 0);
								}
								close(output[0]);
								
							}
							else//子进程
							{
								close(output[0]);
								close(1);
								dup2(output[1], STDOUT_FILENO);
								//close(output[1]);
								if( execvp(commands[0], commands) == -1)
								{
									perror("execvp");
									exit(2);
								}
							}
						}
					}
					else
					{
						send(client_sockfd, "Commands Error\n",strlen("Commands Error\n"), 0);
					}
				}
				//存在管道符号 ls -l | wc | wc
				else
				{	
					int begin = 0;
					int pipenow = 0;
					char* pipe_commands[6][10];
					int pc = *pipe_count;
					for(int i =0;i < pc + 1; i++)
					{			
						for (int j = 0; j < 30; j++)
							pipe_commands[i][j] = malloc(sizeof(char)*101);
					}
					
					memset(pipe_commands, 0, sizeof(pipe_commands));
					
					int invalid =0;
					int* eachpipe = malloc(*pipe_count * sizeof(int));
					for (int i =0;i< *pipe_count;i++)
						eachpipe[i] = 0;
					for (int i  =0; i < num_commands; ++i)
					{
						if ( strcmp(commands[i], "|") == 0)
						{
							if (i == 0 || i == num_commands-1)
							{
								invalid = 1;
								const char *message_pipe = "Invalid pipe\n";
								send (client_sockfd, message_pipe, strlen(message_pipe),0);
								break;
							}
							for (int j = 0; j < i - begin; ++j)//end此时为|符号
							{
								pipe_commands[pipenow][j] = commands[begin + j];
								pipe_commands[pipenow][j + 1] = NULL;
								eachpipe[pipenow] ++;
							}
							pipenow++;
							begin = i + 1;
							
						}
					}
					if (!invalid)
					{
						for(int i = 0;i < num_commands - begin; ++i)
						{
							pipe_commands[pipenow][i] = commands[begin + i];
							pipe_commands[pipenow][i+1] = NULL;
						}
						
						//管道命令处理完毕
						int *pid_pipe =malloc( (*pipe_count +2) * sizeof(int) );
						pid_pipe[0] = getpid();
						// ls -l | wc | wc; pipe_count = 2, pid_pipe[0][1][2][3]
						int output[2];
						if (pipe(output) == -1)
						{
							perror("pipe create error");
							exit(2);
						}
						int previous_out_fd = dup(STDIN_FILENO);
						int fds[2];
						for (int i = 1; i < *pipe_count + 2; ++i)
						{
							if (pipe(fds) == -1)
							{
								perror("pipe create error");
								exit(2);
							}
							int ret = fork();
							if (ret < 0)
							{
								perror("pipe create error");
								exit(2);
							}
							else if (ret == 0) // child process
							{
								pid_pipe[i] = getpid();
								
								dup2(previous_out_fd, STDIN_FILENO);
								close(previous_out_fd);
								
								close(fds[0]);
								
								if (i != *pipe_count + 1)//not the last process
									dup2(fds[1], STDOUT_FILENO);
								else//the last
								{
									close(output[0]);
									dup2(output[1], STDOUT_FILENO);
								}
								close(fds[1]);
								
								if (execvp(pipe_commands[i-1][0], pipe_commands[i-1]) == -1)
								{
									perror("execvp");
									exit(2);
								}
								exit(0);
							}
							else//parent
							{
								previous_out_fd = fds[0];
								close(fds[1]);
							}
						}		
						int status;
						for (int i =1; i< *pipe_count +2; ++i)
						{
							waitpid(pid_pipe[1], &status, 0);
							int ret = WEXITSTATUS(status);
							if (ret != 0)
							{
								send(client_sockfd, "Commands Error\n",strlen("Commands Error\n"), 0);
								break;
							}
						}
						char buffer[8192] = {0};
						if (read (output[0], buffer, 8191) <= 0)
							send(client_sockfd, "Commands Error\n",strlen("Commands Error\n"), 0);
						else
							send (client_sockfd, buffer, strlen(buffer), 0);
						close(output[0]);
						free(pid_pipe);
					}
					free(eachpipe);
					/*
					for (int i = 0; i < pc + 1; ++i) 
					{
 	   					for (int j = 0; j < 30; ++j) 
 	   					{
	        					free(pipe_commands[i][j]);
    						}
					}
					*/
	
				}
				free(pipe_count);
				
			}
		}
		else//父进程
		{	

			close(client_sockfd);
		}
	}
				
	return 0;
}
