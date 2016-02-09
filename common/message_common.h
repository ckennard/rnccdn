#ifndef CLIENT_SERVER_COMMMON
#define CLIENT_SERVER_COMMMON

#define MESSAGE_BODY_SIZE 3000 - 32

/* message types available to both user and admin */
#define MESSAGE_TYPE_LIST 1

#define MESSAGE_TYPE_QUIT 2

#define MESSAGE_TYPE_GET_CHUNK 3
/*
  header fields:
    file name: char [16] - 16 bytes (ended with null terminator)
  Message body unused
*/

/* admin process only message types */
#define MESSAGE_TYPE_ADD 4
  /*
   header fields:
     file name: char[16] - 16 bytes (ended with null terminator)
     file size: unsigned int_16 - 4 bytes
     padding: 11 bytes
   Message body:
    Nothing (first add message).
    Once server has responded with an "ACK" message,
      Message body will contain file contents.
    Subsequent CONT messages will be sent with the contents of the file
  */

#define MESSAGE_TYPE_REMOVE 5
  /*
   header fields:
     file name: char[16] - 16 bytes (ended with null terminator)
     padding: 15 bytes
   Message body unused
  */

#define MESSAGE_TYPE_AUTH_ADMIN 6
  /*
   header fields:
     admin username: char[15] (ended with null terminator)
     admin password: char[16] (ended with null terminator)
   Message body unused
  */

/* server responses */
#define MESSAGE_TYPE_ACK 7
  /*
   header unused
   contents unused
  */

#define MESSAGE_TYPE_ERR 8
  /*
    header unused
    Message Body:
      A string describing the error
  */

#define MESSAGE_TYPE_CONT 9
/* CONT message used to indicate that
   the contents of the message are a continuation of the previous 
   message(s) ADD in this case
   CONT has no message header other than the Type field
*/

#define MESSAGE_TYPE_DATA 10 
/*
  Message used for server to send data (file chunks) to the client
  Header data:
    file name: char[15] (ended with null terminator)
    file size: unsigned int_16 (4 bytes)
*/

struct MessageHeader {
  unsigned short Type;
  char header[31];
}

struct Message {
  struct MessageHeader header;
  char contents[MESSAGE_BODY_SIZE];
}

#endif