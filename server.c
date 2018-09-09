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


int write_file(int connfd,struct stu stu1)
{
	strcat(stu1.filename,"_aaa");
	int fd = open(stu1.filename,O_CREAT|O_TRUNC|O_RDWR,0644);
	if(fd < 0)
	{
		perror("open");
		return -2;
	}
	while(1)
	{
		bzero(stu1.buff,sizeof(stu1.buff));
		int ret = recv(connfd,&stu1,sizeof(stu1),0);
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
int read_file(int connfd,struct stu stu1)
{
    int fd = open(stu1.filename,O_RDONLY);
    if(fd < 0)
    {
        perror("open");
        return -1;
    }
    while(1)
    {
        bzero(stu1.buff,1024);
        ssize_t ret = read(fd,stu1.buff,sizeof(stu1.buff));
        stu1.len = ret;
        if(ret < 0)
        {
            perror("read");
            return -2;
        }
        if(ret == 0)
        {
            bzero(stu1.buff,1024);
            send(connfd,&stu1,sizeof(stu1),0);
			sleep(1);
            printf("send ok\n");
            break;
        }
        if(ret > 0)
        {
            send(connfd,&stu1,sizeof(stu1),0);
        }
    }
    close(fd);

}
int filename_send(int connfd,struct stu stu1)
{
	char buf[1024]={0};
	bzero(buf,sizeof(buf));
	char filename[32]="file_name.txt";
	sprintf(buf,"ls -l -a > %s",filename);
	system(buf);
	int fd=open(filename,O_RDONLY);
	if(fd < 0)
	{
		perror("open");
		return -1;
	}
	while(1)
	{
		bzero(stu1.buff,sizeof(stu1.buff));
		int ret  = read(fd,stu1.buff,sizeof(stu1.buff));
		stu1.len = ret;
		if(ret<0)
		{
			perror("read\n");
			break;
		}		
		int ret1=send(connfd,&stu1,sizeof(stu1),0);
		if(ret1<0)
		{	
			perror("send");
			break;
		}
		if(stu1.len==0)
		{
			break;
		}
	}
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

	int ret = bind(sockfd,(struct sockaddr *)&addr,len);
	if(ret < 0)
	{
		perror("connect");
		return -2;
	}
	
	int ret1 = listen(sockfd,100);
	if(ret1 < 0)
	{
		perror("listen");
		return -3;
	}

	int connfd = accept(sockfd,NULL,NULL);
	if(connfd < 0)
	{
		perror("connfd");
		return -4;
	}

	while(1)
	{
		int ret2 = recv(connfd,&stu1,sizeof(stu1),0);
			sleep(1);
		if(ret2 < 0)
		{
			perror("recv");
			return -5;
		}
		else
		{
		if(strncmp(stu1.buff,"put",3) == 0)
		{
			write_file(connfd,stu1);
			bzero(&stu1,sizeof(stu1));
		}
		if(strncmp(stu1.buff,"get",3) == 0)
		{
			read_file(connfd,stu1);
			bzero(&stu1,sizeof(stu1));
		}
		if(strncmp(stu1.buff,"ls",2) == 0)
		{
			filename_send(connfd,stu1);
			bzero(&stu1,sizeof(stu1));
		}
		if(strncmp(stu1.buff,"by",2) == 0)
		{
			break;
		}
		}
	
	}
}
