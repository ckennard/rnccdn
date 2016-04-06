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

#endif
