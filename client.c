#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/signal.h>
#include <signal.h>
#include <netdb.h>
 
static volatile int keepRunning = 1;


void closeSocket(int dummy) {
    keepRunning = 0;
}

int resolveHost(char * hostname , char* ip)
{
    struct hostent *hent;
    struct in_addr **addr_list;
    int i;
    if ( (hent = gethostbyname( hostname ) ) == NULL)
    {
        herror("gethostbyname error");
        return 1;
    }
    addr_list = (struct in_addr **) hent->h_addr_list;
    for(i = 0; addr_list[i] != NULL; i++)
    {
        strcpy(ip , inet_ntoa(*addr_list[i]));
        return 0;
    }
    return 1;
}

int main(){
 
  char *hostname = "messenger.ddns.net";
  char ip[100] = {'\0'};
  int port = 5590;
 
  int sock;
  struct sockaddr_in addr;
  socklen_t addr_size;
  char buffer[1024], read_user_input[512],c, username[25], myname[25];
  char read_buffer[1024];
  int n, flag=1, icheck=0;
  pid_t split;
  signal(SIGINT, closeSocket);
 
  printf("Your Name: ");
  //icheck=scanf("%s[^\n]",myname);
  /*if (fgets(, sizeof myname, stdin) == NULL) 
  {
	printf("Please Enter your name next time\n");
	exit(EXIT_FAILURE);
  }
  sscanf();*/
  icheck=scanf("%[^\n]s",myname);
  scanf("%c",&c);
  if (icheck == 0) 
  {
	printf("Please Enter your name next time\n");
	exit(EXIT_FAILURE);
  }

  if(resolveHost(hostname, ip))
  {
    printf("Failed to resolve Hostname\n");
    return 0;
  }
  printf("Hostname resolved successfully\n");
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
			//icheck=scanf("%s[^\n]",read_user_input);
  			/*if (fgets(read_user_input, sizeof(char) *25 , stdin) == NULL)
			{
				printf("Please provide valid name to\n");
				continue;
			}*/ 
			icheck=scanf("%[^\n]s",read_user_input);
  			scanf("%c",&c);
			if( icheck == 0 )
			{
				printf("Please provide valid name to\n");
				continue;
			}
			strncpy(username,read_user_input,25);
			flag=0;
			printf("%s: ",myname);
		}
		else
		{
			printf("%s: ",myname);
		}

		//icheck=scanf("%s[^\n]",read_user_input);
  		/*if (fgets(read_user_input, sizeof read_user_input, stdin) == NULL)
		{
			printf("change - to change user to chat with\n");
			printf("exit - to close chat\n");
			continue;
		}*/
		icheck=scanf("%[^\n]s",read_user_input);
  		scanf("%c",&c);
		if( icheck == 0)
		{
			printf("change - to change user to chat with\n");
			printf("exit - to close chat\n");
			continue;
		}

		if(strncmp(read_user_input,"change",6) == 0)
		{
			flag=1;
			continue;
		}
  		//strcpy(buffer, "HELLO, THIS IS CLIENT.");
  		//printf("Client: %s\n", read_user_input);
		snprintf(buffer,1024,"%s#%s#%s",username,myname,read_user_input);
		//snprintf(buffer,1024,"%[^\n]%*c#%[^\n]%*c#%[^\n]%*c",username,myname,read_user_input);
  		send(sock, buffer, strlen(buffer), 0);
  		//printf("Client: %s\n", buffer);
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
  			printf("%s\n", read_buffer);
		}
 	}
  }
  kill(split, SIGKILL);
  send(sock, "exit", strlen("exit"), 0);
  close(sock);
  printf("Disconnected from the server.\n");
 
  return 0;
}
