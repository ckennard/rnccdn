#include "common/chunk_header.h"
#include <stdlib.h>
#include <stdio.h>

#define RECV_BUF_SIZE 256

//GLOBALS
char *chunk_directory = NULL;
int listen_port = -1;


int parse_args(int argc, char **argv) {
  int c;
  opterr = 0;
  while(( c = getopt(argc, argv, "pd:")) != -1){
    switch(c){
      case 'p':
        pvalue = atoi(optarg);
        break;
      case 'd':
        chunk_directory = malloc(strlen(optarg));
        memcpy(chunk_directory, optarg, strlen(optarg));
        break;
      case '?':
        return -1;
      default:
        return -1;
    }
  }

  if(chunk_directory == NULL || listen_port == -1)
    return -1;

  return 0;
}


//receive data from client socket, parse it out into a header and the payload (the actual chunk data)
//TODO: check for data validity using header fields
//TODO: instead of keeping entire chunk data in memory at once, periodically write parts of it to disk
void receive_chunk(client_sock_fd, out_fd, ChunkHeader **out_chunk_header, char **out_chunk_payload) {
  int result = 0;
  char recv_buf[256];
  ChunkHeader *chunk_header;
  char *chunk_payload = NULL;

  unsigned int num_received = 0;
  unsigned int num_processed = 0;

  unsigned int num_payload = 0;
  unsigned int num_header = 0;

  while((result = recv(client_sock_fd, RECV_BUF_SIZE)) > 0) {

    num_received += result;
    if(num_processed <= sizeof(ChunkHeader)) { //header is still being received
      if(num_received >= sizeof(ChunkHeader)) {
        //data received contains both header and payload bytes
        num_payload = num_received - sizeof(ChunkHeader);
        num_header = sizeof(ChunkHeader) - num_processed;
        
        memcpy(chunk_header+num_processed, recv_buf, num_header);
        memcpy(chunk_payload, recv_buf+num_header, num_payload);
      } else {
        //header is still being received
        memcpy(chunk_header+num_processed, recv_buf, result);
      }
    } else {
      //payload is being received
      memcpy(chunk_payload+(num_processed-sizeof(ChunkHeader)), recv_buf, result;
    }

    num_processed = num_received;
  }
}

int receive_message_header(int sock_fd, struct MessageHeader *out_message_header) {
  int num_received = 0;
  int result = 0;
  struct MessageHeader message_header;
  
  if((result = recv(sock_fd, *out_message_header, sizeof(struct MessageHeader))) < 0) {
    return -1;
  }
  
  num_received += result;
  
  while(num_received < sizeof(struct MessageHeader)) {
    result = recv(sock_fd, &(message_header + num_received), sizeof(struct MessageHeader) - num_received);

    if(result < 0) {
      return -1;
    } else if (result == 0) {
      return 0;
    } else if (result > 0) {
      num_received += result;
      continue;
    }
  }

  memcpy(out_message_header, &message_header, sizeof(struct MessageHeader));
}

int receive_chunk_header(int sock_fd, struct ChunkHeader **out_header) {
  int num_received = 0;
  int result = 0;
  struct MessageHeader message_header;
  
  if((result = recv(sock_fd, *out_message_header, sizeof(struct MessageHeader))) < 0) {
    return -1;
  }
  
  num_received += result;
  
  while(num_received < sizeof(struct MessageHeader)) {
    result = recv(sock_fd, &(message_header + num_received), sizeof(struct MessageHeader) - num_received);

    //consider case where recv retrieves full transmission in first attempt
    //consider case where recv retrieves ransmission in first attempt

    if(result < 0) {
      printf("recv error");
      return -1;
    } else if (result == 0) {
      if(num_received == sizeof(struct MessageHeader)) {
        break;
      } else {
        printf("full header not received");
        return -1;
      }
    } else if (result > 0) {
      num_received += result;
      continue;
    }
  }

  memcpy(out_message_header, &message_header, sizeof(struct MessageHeader));
}

int receive_chunk_contents(int sock_fd, struct ChunkHeader *header, char *out_chunk_data) 
{
  int result = 0;
  int num_received = 0;

  while(1) {
    result = recv(sock_fd, out_chunk_data + num_received, header->file_size);

    if(result < 0) {
      printf("recv error");
      return -1;
    } else if (result == 0) {
      if(num_received == header->chunk_size) {
        return 0; //received correct amt of data
      } else {
        printf("header chunk_size not same as size of data received.");
        return -1;
      }
    } else {
      num_received += result;
      if(num_received == header->chunk_size) {
        return 0;
      }
    }
  }
}

int store_chunk_contents(struct ChunkHeader header, char *chunk_buf, int buf_size) {
  FILE *fp = NULL;
  //see if file for chunk already exists
  if(access(strcat(chunk_directory, chunk_header->file_name)) != -1) {
    if(remove(file_path) < 0) {
      printf("failed to remove existing chunk file");
      return -1;
    }
  }

  fp = fopen(file_path, "r+");

  if((result = fwrite(header, 1, sizeof(struct ChunkHeader), fp)) < sizeof(struct ChunkHeader)) {
    printf("error writing header to file");
    return -1;
  }

  if((result = fwrite(chunk_buf, 1, buf_size, fp)) < buf_size) {
    printf("error writing chunk contents to file");
    return -1;
  }
  return 0;
}

int validate_chunk_header_pre_receive(const ChunkHeader * chunk_header) {

}

int validate_chunk_header_post_receive(const ChunkHeader *chunk_header) {

}

int main(int argc, char **argv) {
  int result = -1;
  struct sockaddr_in server_sock, client_sock;
  int client_sock_fd = -1;
  int server_sock_fd = -1;
  unsigned int clientlen = sizeof(client_sock);

  struct MessageHeader message_header;
  struct ChunkHeader chunk_header;

  char *chunk_directory = NULL;
  FILE *chunk_fp = NULL;
  int chunk_file_size = 0;

  if((result = parse_args(argc, argv)) < 0) {
    Die("invalid arguments");
  }

  if((chunk_fp = fopen("...", "w+")) == 0) {
    Die("failed to open file to store chunk");
  };

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
  while(TRUE) {
    //when a connection has been made, receive chunk and store it on disk
    if((client_sock_fd = accept(server_sock_fd, (struct sockaddr *) &client_sock, &clientlen)) < 0) {
      Die("accept client connection failed");
    }

    //receive and process a message
    if((result = receive_message_header(&message_header)) < 0) {
      Die("failed to receive message_header");
    }

    switch(message_header.Type) {
      case TYPE_POST_CHUNK:
        if((result = receive_chunk_header(&chunk_header)) < 0) {
          Die("failed to receive chunk header");
        }

        if((result = validate_chunk_header_pre_receive(&chunk_header)) < 0) {
          Die("invalid chunk header");
        }

        if((result = receive_chunk_contents(&chunk_buf)) < 0) {
          Die("failed to receive chunk contents");
        }

        if((result = validate_chunk_header_post_receive(&chunk_header)) < 0) {
          Die("invalid chunk header");
        }

        if((result = store_chunk_contents(chunk_buf)) < 0) {
          Die("failed to store chunk");
        }
        break;
      default:

        break;
    }
  }
}
