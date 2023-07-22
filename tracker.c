#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
	fd_set master;
	fd_set read_fds;
	struct sockaddr_in serveraddr;
	struct msghdr msg,msgr;
	struct sockaddr_in clientaddr;
	int fdmax;
	int listener;
	int newfd;
	int port=10010;
	if(argv[1])port=atoi(argv[1]);
	int addrlen;
	int i, j;
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	if((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Failed to create socket!"); 
		exit(1);
	}
	
	if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int)) == -1)
	{
		perror("setsockopt() failed!");
		exit(1);
	}
	
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	serveraddr.sin_port = htons(port);
	memset(&(serveraddr.sin_zero), '\0', 8);
	if(bind(listener, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
	{
		perror("Bind to port failed!");
		exit(1);
	}
	if(listen(listener, 10) == -1)
	{
		perror("listen failed!");
		exit(1);
	}
	printf("Listening on port %d\n",port);
	FD_SET(listener, &master);
	fdmax = listener;
	memset(&msg, 0, sizeof(msg));
	memset(&msgr, 0, sizeof(msgr));
	char buf1[8],buf2[4],buf3[6];
	struct iovec iovr[2]={{buf1, 8},{buf2,4}};
	struct iovec iov[1]={{buf3, 6}};
	msgr.msg_iov=(struct iovec *)&iovr;
	msgr.msg_iovlen=2;
	msg.msg_iov=(struct iovec *)&iov;
	msg.msg_iovlen=1;
	buf3[3]='J'+1;
	buf3[4]=6;
	buf3[1]='P'-1;
	buf3[5]=0;
	buf3[0]='E'+1;
	buf3[2]='R';

	while(1)
	{
		read_fds = master;
		if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
		{
			perror("Select failed!");
			exit(1);
		}
		for(i = 0; i <= fdmax; i++)
		{
			if(FD_ISSET(i, &read_fds))
			{ 
				if(i == listener)
				{
					addrlen = sizeof(clientaddr);
					if((newfd = accept(listener, (struct sockaddr *)&clientaddr, &addrlen)) == -1)
					{
						perror("accept failed!");
					}
					else
					{
						FD_SET(newfd, &master);
						if(newfd > fdmax) fdmax = newfd;
						printf("New connection from %s on socket %d\n", inet_ntoa(clientaddr.sin_addr), newfd);
					}
				}
				else
				{
					int r=recvmsg(i, &msgr,0);
					
					if(r>0)
					{
						printf("Received: 0x%08x(%d) 0x%08x(%d)\n",*(int*)(buf1+4),*(int*)(buf1+4),*(int*)buf2,*(int*)buf2);
						sendmsg(i, &msg,MSG_NOSIGNAL);
					}
					else 
					{
						printf("Connection %d closed\n",i);
						close(i);
						FD_CLR(i, &master);
					}
				}
			}
		}
	}
	return 0;
}
