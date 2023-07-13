/*
 * Filename    : multi_sever_daemon.c
 * Description : Multiple Server - Client Socket programming where a 
 * 		 server can handle multiple clients
 * Author      : mujibur.rahiman@leadsoc.com
 * 
 */


// Header files declaration
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>

// Global declaration

#define PORT 4500
#define MAX_CLIENTS 30

struct client_list {
    int client_id;
    char client_name[25];
	int sent_flag;
};

static struct client_list write_head, read_head;

/*
 * function_name : main.
 * description   : Main function to initialize the server socket dameon and serve
 *                 Multiple clients.
 * parameters    : int tic - total_input_count, char **const iv - input values.
 *
 * return        : zero on success and error value is set on failure.
 * */

int main( int tiv, char **const iv)
{
	// Socket Initializers
	char *ip = "127.0.0.1";
  	int port = 5590;
  	static FILE *client_List, *temp_file;

	int server_daemon=0, read_msg=0, new_client=0, error_flag=0, address_len=0, 
		activity_flag=0, max_fd=0, client_loop=0, client_fd=0, client_connection[MAX_CLIENTS], opt=1;
	struct sockaddr_in server_address;
	//char *message="LeadSOC Chat Box\0";
	//char read_buffer[1024] = {'\0'};
	char send_buffer[1024] = {'\0'};
	fd_set read_sock_desc;
	int sent_flag=1, file_flag=0, read_flag=0, temp_flag=0, n=0;
	char buffer[1024], *dest, *source, *msg_to_send;
	char tmp[50];

	for( client_loop =0; client_loop< MAX_CLIENTS; client_loop++)
	{
		client_connection[client_loop]=0;
	}

    rename("/tmp/client_list.bin","/tmp/tw");
	// Step:1 Create a Master socket daemon
	server_daemon=socket(AF_INET, SOCK_STREAM, 0);

	if( server_daemon < 0 )
	{
		perror("Master_Daemon: Error Creating a Socket\n");
		exit(EXIT_FAILURE);
	}

	// Step:2 Set the socket options 
	if( setsockopt(server_daemon, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt))  < 0 )
	{
		close(server_daemon);
		perror("Master_Daemon: Error Creating a option for Socket\n");
		exit(EXIT_FAILURE);
	}

	// Step:3 Bind the IP address to the created file descriptor
	server_address.sin_family=AF_INET;
	server_address.sin_port=port;
	server_address.sin_addr.s_addr=inet_addr(ip);;
	
	if( bind(server_daemon, (struct sockaddr*)&server_address, sizeof(server_address)) < 0 )
	{
		close(server_daemon);
		perror("Master_Daemon: Error Binding socket address\n");
		exit(EXIT_FAILURE);
	}

	// Step:4 Listen the socket file descriptor
	if( listen(server_daemon,10) < 0 )
	{
		close(server_daemon);
		perror("Master_Daemon: Error Binding socket address\n");
		exit(EXIT_FAILURE);
	}

	// Step:5 create a Select method to get multiple clients

	address_len=sizeof(server_address);

	printf("Master_Daemon: Waiting for Clients\n");

	
	while(1)
	{
		FD_ZERO(&read_sock_desc);
		
		FD_SET(server_daemon, &read_sock_desc);
		max_fd=server_daemon;
		for(client_loop=0; client_loop < MAX_CLIENTS; client_loop++)
		{
			client_fd = client_connection[client_loop];
		
			if( client_fd >  0 )
			{
				FD_SET(client_fd, &read_sock_desc);
			}

			if( client_fd > max_fd )
			{
				max_fd=client_fd;
			}
		}

		activity_flag=select(max_fd+1, &read_sock_desc, NULL, NULL, NULL);

		if(activity_flag < 0 && (errno!=EINTR) )
		{
			printf("Master_Daemon: Select Error\n");
		}

		if(FD_ISSET(server_daemon, &read_sock_desc))
		{
			memset(&server_address, 0, sizeof(server_address));
			if( (new_client=accept(server_daemon, (struct sockaddr*)&server_address, (socklen_t*)&address_len)) < 0 )
			{
				perror("Master_Daemon: Error in accepting client \n");
			}
			printf("[SRV_LOG]: New Client Connected\nClient ID: %d\nClient IP: %s\nClient Port: %d\n", 
					new_client, inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port));
			client_List = fopen("/tmp/client_list.bin", "ab");
        	fseek(client_List, 0, SEEK_END);
        	write_head.client_id = new_client;
        	write_head.sent_flag = 1;
        	file_flag=fwrite(&write_head, sizeof(struct client_list), 1, client_List);
       		fclose(client_List);
        	printf("[SRV_LOG]: Client socket %d updated \n",new_client);

			/*printf("Sending message to client %d\n", new_client);
			if ( send(new_client, message, strlen(message), 0) != strlen(message) )
			{
				printf("Master_Daemon: Error sending message to Client: %d\n", new_client);
			}

			printf("Msg sent to client %d successfully\n", new_client);
			*/

			for(client_loop=0; client_loop < MAX_CLIENTS; client_loop++)
			{
				if( client_connection[client_loop] == 0 )
				{
					client_connection[client_loop] = new_client;
					printf("Master_Daemon Added client %d to the active client list\n", new_client);
					break;
				}
			}
		}

		for(client_loop=0; client_loop < MAX_CLIENTS; client_loop++)
		{
			client_fd=client_connection[client_loop];

			if(FD_ISSET(client_fd, &read_sock_desc))
			{
				bzero(buffer, 1024);
        		recv(client_fd, buffer, sizeof(buffer), 0);
        		printf("Actual Buffer: %s\n", buffer);
        		dest=strtok(buffer, "#");
        		source=strtok(NULL, "#");
        		msg_to_send=strtok(NULL, "#");
        		printf("Debug \ndest : %s\nsource : %s\nmsg_to_send : %s\nclient_ID: %d\n", dest,source,msg_to_send,client_fd);
        		printf("%s : %s\n", source,msg_to_send);
				if (msg_to_send != NULL)
				{
            		client_List = fopen("/tmp/client_list.bin", "rb");
            		if (client_List == NULL) {
                		fprintf(stderr, "\nError opening file\n");
            		}
            			read_flag=fread(&read_head, sizeof(struct client_list), 1, client_List);
            			do
            			{
                			if(read_head.client_id ==  client_fd)
                			{
								sent_flag=read_head.sent_flag;
                			}
                			printf("Debug inside sent flag\n");
                			read_flag=fread(&read_head, sizeof(struct client_list), 1, client_List);
            			} while(read_flag != 0);
            		fclose(client_List);

					if( sent_flag == 1)
        			{
            			printf("Debug inside sent flag\n");
            			client_List = fopen("/tmp/client_list.bin", "rb");
            			if (client_List == NULL) {
                			fprintf(stderr, "\nError opening file\n");
            			}
            			temp_file = fopen("/tmp/temp.bin", "ab+");
            			read_flag=fread(&read_head, sizeof(struct client_list), 1, client_List);
            			do
            			{
                			if(read_head.client_id ==  client_fd)
                			{
								read_head.sent_flag = 0;
                    			strcpy(read_head.client_name, source);
                			}
                			temp_flag = fwrite(&read_head, sizeof(struct client_list), 1, temp_file);
                			printf("Debug inside sent flag\n");
                			read_flag=fread(&read_head, sizeof(struct client_list), 1, client_List);
            			} while(read_flag != 0);
            			fclose(temp_file);
            			fclose(client_List);
            			rename("/tmp/temp.bin", "/tmp/client_list.bin");
            			printf("Debug inside sent flag\n");
            			sent_flag=0;
        			}
					else if ( (strncmp(msg_to_send,"exit",4) == 0) || (strlen(msg_to_send) == 0) )
        			{
            			printf("Debug inside exit function\n");
            			client_List = fopen("/tmp/client_list.bin", "rb");
            			if (client_List == NULL) {
                			fprintf(stderr, "\nError opening file\n");
            			}

            			temp_file = fopen("/tmp/temp.bin", "ab+");
            			read_flag=fread(&read_head, sizeof(struct client_list), 1, client_List);
            			do
            			{
                			if(read_head.client_id !=  client_fd)
                			{
                    			temp_flag = fwrite(&read_head, sizeof(struct client_list), 1, temp_file);
                			}
                			printf("Debug inside exit flag\n");
                			read_flag=fread(&read_head, sizeof(struct client_list), 1, client_List);
            			} while(read_flag != 0);
            			fclose(temp_file);
            			fclose(client_List);
           	 			rename("temp.bin", "client_list.bin");
            			printf("%s disconnected\n",source);
            			close(client_fd);
                    	client_connection[client_loop] = 0;  
        			}
					else
        			{
            			printf("Debug inside msg section \n");
            			if ( (strncmp(msg_to_send,"clist",5) == 0) )
            			{
                			printf("01 inside clist section \n");
                			bzero(send_buffer, 1024);

                			client_List = fopen("/tmp/client_list.bin", "rb");
                			if (client_List == NULL) {
                    			fprintf(stderr, "\nError opening file\n");
                			}
                			strcat(send_buffer,"\nClient_List\n");
                			read_flag=fread(&read_head, sizeof(struct client_list), 1, client_List);
                			do
                			{
                    			bzero(tmp, sizeof(tmp));
                    			snprintf(tmp,50,"%s - %d\n",read_head.client_name,read_head.client_id);
                    			printf("Values inside list %s\n",tmp);
                    			strcat(send_buffer,tmp);
                    			printf("Debug inside clist flag\n");
                    			read_flag=fread(&read_head, sizeof(struct client_list), 1, client_List);
                			} while(read_flag != 0);
                			fclose(client_List);
                			printf("02 inside clist section \n");
                			printf("Buffer from list %s\n",send_buffer);
                			send(client_fd, send_buffer, strlen(send_buffer), 0);
                			printf("03 inside clist section \n");
            			}
						else
            			{
                			printf("inside last section \n");
                			printf("Server: Sending msg from %s to %s\n",source,dest);
                			//printf("Address of head ref is %p\n", head_ref);
                			client_List = fopen("/tmp/client_list.bin", "rb");
                			if (client_List == NULL) {
                    			fprintf(stderr, "\nError opening file\n");
                			}

                			read_flag=fread(&read_head, sizeof(struct client_list), 1, client_List);
                			do
                			{
                    			if(strncmp(read_head.client_name, dest, sizeof(read_head.client_name)) == 0 )
                    			{
                        			n=read_head.client_id;
                        			printf("dest founde Debug inside map flag\n");
                        			break;
                    			}
                    			printf("Debug inside map flag\n");
                    			read_flag=fread(&read_head, sizeof(struct client_list), 1, client_List);
                			} while(read_flag != 0);
                			fclose(client_List);

                			if(n!=0) {
                				bzero(send_buffer, 1024);
								snprintf(send_buffer,1024,"\n%s: %s",source, msg_to_send);
                    			printf("inside last section send\n");
                    			send(n,send_buffer,strlen(send_buffer), 0);
                			}
                			else {
                				bzero(send_buffer, 1024);
								snprintf(send_buffer,1024,"\n%s is not Actice",dest);
                    			send(client_fd,send_buffer,strlen(send_buffer), 0);
								printf("%s is not active\n",dest); 
                			}
            			}
        			}
				}
				//memset(&buffer, 0, sizeof(buffer));
				/*if( (read_msg= read(client_fd, buffer, 1024)) == 0)
				{
					printf("Master_Daemon: Not able to read msg\n");
					getpeername(client_fd , (struct sockaddr*)&server_address, 
                        (socklen_t*)&address_len);  
                    printf("Master_Daemon: Host disconnected , ip %s , port %d \n" , 
                          inet_ntoa(server_address.sin_addr) , ntohs(server_address.sin_port));  
                         
                    //Close the socket and mark as 0 in list for reuse 
                    close( client_fd );  
                    client_connection[client_loop] = 0;  
				}			
				else
				{		
					printf("Master_Daemon: Msg from client %d \n%s\n",client_fd,read_buffer);
					if ( strncmp(read_buffer,"exit",4) == 0 )
					{
						getpeername(client_fd , (struct sockaddr*)&server_address, 
                        	(socklen_t*)&address_len);  
                    	printf("Master_Daemon: Host disconnected , ip %s , port %d \n" , 
                         	inet_ntoa(server_address.sin_addr) , ntohs(server_address.sin_port));  
                         
                    	//Close the socket and mark as 0 in list for reuse 
                    	close( client_fd );  
                    	client_connection[client_loop] = 0;  

					} 
					else 
					{
						memset(&write_buffer, 0, sizeof(write_buffer));
						//printf("Master_Daemon: Message to send to client %d : ",client_fd);
						//scanf("\n%[^\n]s\n",write_buffer);
						strcat(write_buffer, "Msg sent Successfully\0");  
                		send(client_fd , write_buffer , strlen(write_buffer) , 0 );
					}
				}*/
			}
		}

	}
		
	printf("Master_Daemon: End of Main\n");
	return 0;
}
