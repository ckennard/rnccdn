#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <unistd.h>
#include <semaphore.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "../common/chunk_header.h"
#include "../common/message_header.h"

#define LISTEN_PORT 3001
#define RECV_BUF_SIZE 256
#define MAX_PENDING 1
#define CHUNK_BUF_SIZE 10000
#define MAX_PATH 256

char *chunk_directory = "./out";
int listen_port = 3000;

void Die(char *str) {
  char *errno_str = strerror(errno);
  
  printf(str);
  printf("errno(%d):\n", errno);
  printf(errno_str);

  //free(errno_str);
  exit(-1);
}

int open_and_read_chunk(char *chunk_name, char **out_chunk_buf, int *out_chunk_size) {
  int result = 0;
  char full_path[MAX_PATH];
  FILE *fp;
  memset(full_path, 0, MAX_PATH); //zero out the buffer

  strcat(full_path, chunk_directory);
  strcat(full_path, "/"); 
  strcat(full_path, chunk_name);

  if((result = access(full_path, F_OK)) < 0) {
    printf("chunk with given name doesn't exist\n");
    return -1;
  }
  
  fp = fopen(full_path, "r");
  if(!fp) {
    printf("failed to open chunk file for reading");
    return -1;
  }

  //get file size
  fseek(fp, 0L, SEEK_END);
  *out_chunk_size = ftell(fp);
  fseek(fp, 0L, SEEK_SET);

  *out_chunk_buf = malloc(*out_chunk_size);

  fread(*out_chunk_buf, *out_chunk_size, 1, fp);
  fclose(fp);

  return 0;
}

int send_chunk(int sock_fd, char *chunk_buf, int chunk_size) {
  int result = 0;
  int num_sent = 0;

  struct MessageHeader chunk_post_header;
  memset(&chunk_post_header, 0, sizeof(struct MessageHeader));

  chunk_post_header.Type = TYPE_POST_CHUNK;
  memcpy((void *)chunk_post_header.params, (void *)&chunk_size, sizeof(int));

  //send the header 
  if((result = send(sock_fd, (void *)&chunk_post_header, sizeof(struct MessageHeader), 0)) != sizeof(struct MessageHeader)) {
    printf("failed to send chunk post header");
    return -1;
  }

  while((result = send(sock_fd, (void *)chunk_buf, chunk_size - num_sent, 0)) > 0) {
    num_sent += result;
  }

  if(result == 0) {
    if(num_sent == chunk_size) {
      return 0;
    } else {
      printf("full chunk not sent");
      return -1;
    }
  } else if (result < 0) {
    return -1;
    printf("Error sending chunk\n");
  }

  return 0;
}

int main(int argc, char **argv) {
  //listen for connections from client
  //when a connection has been established send the chunk to the client 
  //over TCP
  int result = -1;
  struct sockaddr_in server_sock, client_sock;
  int client_sock_fd = -1;
  int server_sock_fd = -1;
  unsigned int clientlen = sizeof(client_sock);
  int done = 0;

  char *local_host_ip = "127.0.0.1";
  int listen_port = 3000;

  struct MessageHeader message_header;

  char *chunk_buf = NULL;
  int chunk_size;

  char chunk_name[MAX_PATH];
  memset(chunk_name, 0, MAX_PATH);

  memset(&server_sock, 0, sizeof(client_sock));
  server_sock.sin_family = AF_INET;
  server_sock.sin_addr.s_addr = inet_addr(local_host_ip);
  server_sock.sin_port = htons(listen_port);

  printf(">> Listening for connections on port %d...", listen_port);
  fflush(stdout);

  if((server_sock_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    Die("failed to create socket fd");
  }

  if((result = bind(server_sock_fd, (struct sockaddr *)&server_sock, sizeof(server_sock))) < 0) {
    Die("failed to bind server socket");
  }

  if((result = listen(server_sock_fd, MAX_PENDING)) < 0) {
    Die("Failed to listen on server socket");
  }

  //listen for connections
  while(!done) {
    //when a connection has been made, receive chunk and store it on disk
    if((client_sock_fd = accept(server_sock_fd, (struct sockaddr *) &client_sock, &clientlen)) < 0) {
      Die("accept client connection failed");
    }

    //receive and process a message
    if((result = receive_message_header(client_sock_fd, (void *)&message_header)) < 0) {
      Die("failed to receive message_header");
    }

    printf("received message header\n");
    fflush(stdout);

    switch(message_header.Type) {
      case TYPE_REQUEST_CHUNK:
        strcpy(chunk_name, message_header.params);

        //open the file, read into a buf and get the size
        if((result = open_and_read_chunk(chunk_name, &chunk_buf, &chunk_size)) < 0) {
          Die("failed to open and read chunk file");
        }

        if((result = send_chunk(client_sock_fd, chunk_buf, chunk_size)) < 0) {
          Die("failed to send chunk contents");
        }
        break;
    }
  }
}
