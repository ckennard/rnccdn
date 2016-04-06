#ifndef CLIENT_SERVER_COMMMON
#define CLIENT_SERVER_COMMMON

#include<stdint.h>

//total size should be 64 bytes (probably subject to change)
struct ChunkHeader {
  char file_name[30];
  long file_size;
  uint16_t coef[3];
  int whitespace;
  int oddByte;
};

#endif
