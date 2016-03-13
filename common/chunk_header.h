#ifndef CLIENT_SERVER_COMMMON
#define CLIENT_SERVER_COMMMON

//total size should be 64 bytes (probably subject to change)
struct ChunkHeader {
  char file_name[30];
  int file_size;
}

#endif
