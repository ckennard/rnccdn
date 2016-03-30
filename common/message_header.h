#define TYPE_POST_CHUNK 1
//post chunk followed by ChunkHeader and payload

#define TYPE_REQUEST_CHUNK 2
//request chunk followed by null-terminated chunk file name str

#define TYPE_ACKNOWLEDGE 3
//message followed by nothing

#define TYPE_ERROR 4
//message followed by null-terminated error string

struct MessageHeader {
  int Type;
  int MessageLen; //the length of the data that follows this header
};
