#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>

struct stu
{
	int len;
	char filename[128];
	char buff[1024];
};

int filename_recv(int sockfd,struct stu stu1)
{
    while(1)
    {   
        bzero(stu1.buff,sizeof(stu1.buff));
        int ret = recv(sockfd,&stu1,sizeof(stu1),0);
        if(ret  < 0)
        {   
            perror("1recv");
            return -1;
        }
		printf("%s",stu1.buff);
        if(stu1.len == 0)
        {
            break;
        }
    }


}
int file_put(int sockfd,struct stu *stu1,int len)
{
	int fd = open(stu1->filename,O_RDONLY);
	if(fd < 0)
	{
		perror("open");
		return -1;
	}
	while(1)
	{
		bzero(stu1->buff,1024);
		ssize_t ret = read(fd,stu1->buff,sizeof(stu1->buff));
		stu1->len = ret;
		if(ret < 0)
		{
			perror("read");
			return -2;
		}
		if(stu1->len == 0) 
		{
			stu1->len = 0;
			bzero(stu1->buff,sizeof(stu1->buff));
			send(sockfd,stu1,len,0);
			sleep(1);
			printf("send ok!\n");
			break;
		}
		if(ret > 0)
		{
			send(sockfd,stu1,len,0);
		}
	}
	close(fd);
}
int file_get(int sockfd,struct stu stu1)
{
    int fd = open(stu1.filename,O_CREAT|O_TRUNC|O_RDWR,0644);
    if(fd < 0)
    {   
        perror("open");
        return -2;
    }
    while(1)
    {   
        bzero(stu1.buff,sizeof(stu1.buff));
        int ret = recv(sockfd,&stu1,sizeof(stu1),0);
				sleep(1);
        if(ret  < 0)
        {   
            perror("1recv");
            return -1;
        }
        if(stu1.len == 0)
        {
            break;
        }
        if(stu1.len > 0);
        {   
            write(fd,stu1.buff,stu1.len);
        }
    }

    close(fd);
    return 0;
}
int main(int argc,char *argv[])
{
	if(argc != 3)
	{
		printf("Usage:%s ip port\n",argv[0]);
		return 0;
	}

	struct stu stu1;
	char buf[128] = {0};

	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0)
	{
		perror("socket");
		return -1;
	}

	struct sockaddr_in addr;
	addr.sin_family      = AF_INET;
	addr.sin_port        = htons(atoi(argv[2]));
	addr.sin_addr.s_addr = inet_addr(argv[1]);
	socklen_t len        = sizeof(addr);

	int ret = connect(sockfd,(struct sockaddr *)&addr,len);
	if(ret < 0)
	{
		perror("connect");
		return -2;
	}
	else
	{
		while(1)
		{
			printf("please input: ls get put by\n");
			fgets(buf,sizeof(buf),stdin);
			if(strncmp(buf,"ls",2) == 0)
			{
				strcpy(stu1.buff,buf);
				send(sockfd,&stu1,sizeof(stu1),0);
				filename_recv(sockfd,stu1);
			}
			else if(strncmp(buf,"get",3) == 0)
			{
				char chpr1[128] = {0};
				sscanf(buf,"%s %s",chpr1,stu1.filename);
				strcpy(stu1.buff,buf);
				send(sockfd,&stu1,sizeof(stu1),0);
				sleep(1);
				file_get(sockfd,stu1);
			}
			else if(strncmp(buf,"put",3) == 0)
			{
				char chpro[128] = {0};
				sscanf(buf,"%s %s",chpro,stu1.filename);
				strcpy(stu1.buff,buf);
				send(sockfd,&stu1,sizeof(stu1),0);
				sleep(1);
				file_put(sockfd,&stu1,sizeof(stu1));
			}
			else if(strncmp(buf,"by",2) == 0) 
			{
				strcpy(stu1.buff,buf);
				send(sockfd,&stu1,sizeof(stu1),0);
				break;
			}
			else
			{
				printf("please input again\n");
			}
		}
	}
}
