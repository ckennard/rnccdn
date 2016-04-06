#ifndef MESSAGE_HEADER_H
#define MESSAGE_HEADER_H

#define TYPE_POST_CHUNK 1
//post chunk.  file size (unsigned int) goes in params

#define TYPE_REQUEST_CHUNK 2
//request chunk with by null-terminated chunk file name str in params

#define TYPE_ACKNOWLEDGE 3
//message followed by nothing

#define TYPE_ERROR 4
//message followed by null-terminated error string

struct MessageHeader {
  int Type;
  char params[30];
};

//TODO put this in its own source file and keep the function header in this file
int receive_message_header(int sock_fd, struct MessageHeader *out_message_header) {
  int num_received = 0;
  int result = 0;
  struct MessageHeader message_header;
  
  if((result = recv(sock_fd, (void *)out_message_header, sizeof(struct MessageHeader), 0)) < 0) {
    return -1;
  }
  
  num_received += result;
  
  while(num_received < sizeof(struct MessageHeader)) {
    result = recv(sock_fd, (void *)(&message_header + num_received), sizeof(struct MessageHeader) - num_received, 0);

    if(result < 0) {
      return -1;
    } else if (result == 0) {
      break;
    } else if (result > 0) {
      num_received += result;
      continue;
    }
  }

  //memcpy(out_message_header, &message_header, sizeof(struct MessageHeader));
  return 0;
}
#endif
