#include "common/chunk_header.h"

#define LISTEN_PORT 3001
#define RECV_BUF_SIZE 256

//receive data from client socket, parse it out into a header and the payload (the actual chunk data)
//TODO: check for data validity using header fields
void receive_chunk(client_sock_fd, ChunkHeader **out_chunk_header, char **out_chunk_payload) {
  int result = 0;
  char recv_buf[256];
  ChunkHeader *chunk_header;
  char *chunk_payload = NULL;

  unsigned int num_received = 0;
  unsigned int num_processed = 0;

  while((result = recv(client_sock_fd, RECV_BUF_SIZE)) > 0) {

    num_received += result;

    if(num_processed <= sizeof(ChunkHeader)) {
      //header is still being received
      memcpy(chunk_header+num_processed, recv_buf, result);
    } else {
      //body is being received
      memcpy(chunk_payload+(num_processed-sizeof(ChunkHeader)), recv_buf, result;
    }

    num_processed = num_received;
  }

  *out_chunk_payload = chunk_payload;
  *out_chunk_header = chunk_header;
}

int main(int argc, char **argv) {
  int result = -1;
  struct sockaddr_in server_sock, client_sock;
  int client_sock_fd = -1;
  int server_sock_fd = -1;
  unsigned int clientlen = sizeof(client_sock);

  FILE *chunk_fp = NULL;
  int chunk_file_size = 0;

  if((chunk_fp = fopen("...", "w+")) == 0) {
    Die("failed to open file to store chunk");
  };

  memset(&server_sock, 0, sizeof(client_sock));
  server_sock.sin_family = AF_INET;
  server_sock.sin_addr.s_addr = inet_addr(local_host_ip);
  server_sock.sin_port = htons(port);

  printf(">> Listening for connections on port 3001...");
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

    //receive the message header
  }
}
