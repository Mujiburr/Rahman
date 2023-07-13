#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/signal.h>
#include <signal.h>
 
static volatile int keepRunning = 1;

void closeSocket(int dummy) {
    keepRunning = 0;
}

int main(){
 
  char *ip = "127.0.0.1";
  int port = 5590;
 
  int sock;
  struct sockaddr_in addr;
  socklen_t addr_size;
  char buffer[1024], read_user_input[512],c, username[25], myname[25];
  char read_buffer[1024];
  int n, flag=1;
  pid_t split;
  signal(SIGINT, closeSocket);
 
  printf("Your Name: ");
  scanf("%[^\n]s",myname);
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0){
    perror("[-]Socket error");
    exit(1);
  }
  printf("[+]TCP server socket created.\n");
 
  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = port;
  addr.sin_addr.s_addr = inet_addr(ip);
 
  if ( connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0 )
  {		
	printf("No servers found\n");
	close(sock);
	return 0;
  }
  
printf("Connected to the server %d.\n",sock);
 
  split=fork();
  while (keepRunning) {
  	if( split != 0 ) 
  	{
  		bzero(buffer, 1024);
  		bzero(read_user_input, 512);
		if (flag == 1)
		{
			printf("Chat with : ");
			scanf("\n%[^\n]s",read_user_input);
			strncpy(username,read_user_input,25);
			flag=0;
		}
		else
		{
			printf("%s: ",myname);
		}

		scanf("%[^\n]s",read_user_input);
		scanf("%c",&c);

		if(strncmp(read_user_input,"change",6) == 0)
		{
			flag=1;
			continue;
		}
  		//strcpy(buffer, "HELLO, THIS IS CLIENT.");
  		//printf("Client: %s\n", buffer);
		snprintf(buffer,1024,"%s#%s#%s",username,myname,read_user_input);
  		send(sock, buffer, strlen(buffer), 0);
		if (strncmp(read_user_input,"exit",4) == 0)
		{
			printf("Closing Connection\n");
			close(sock);
			kill(split, SIGKILL);
			return 0;
		}
  	}
  	else 
  	{
  		bzero(read_buffer, 1024);
 		if ( recv(sock, read_buffer, sizeof(read_buffer), 0) > 0)
		{
  			printf("%s\n%s: ", read_buffer,myname);
		}
 	}
  }
  close(sock);
  printf("Disconnected from the server.\n");
 
  return 0;
}
