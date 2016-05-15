#ifndef CLIENT_SERVER_COMMMON
#define CLIENT_SERVER_COMMMON

#include<stdint.h>

struct ChunkHeader {
  /*
  long file_size;
  uint16_t coef[3];
  int whitespace;
  int oddByte;
  */
  uint16_t coef[3];
  uint32_t f_size;
};

#endif
