#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

//
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

//

// #include <assert.h>
// google test...
//dont have array of chunks. (IMPORTANT)
//---Have only 1 chunk at a time write it out, then do the next one.

#include "../common/chunk_header.h"
#include "../common/message_header.h"
#include "gf.h"
#include "mt64.h"

#define PORT_NUMBER 3000

#define BUF_SIZE 8192

#define SPACE 600000
//#define DATA_LENGTH  SPACE / sizeof(uint16_t)

//
//
//
struct arguments {
  int num_of_chunks;
  char * input_file_name;
};

//
//
//
struct chunk {
  //uint16_t a1, a2, a3;
  uint16_t coef[3];
  int numEmpty; //number of 16bit 0s at end of final file
  int oddBytes; //non-zero if odd number of bytes in original file
  uint16_t *output;
  
};

//sauce: http://man7.org/tlpi/code/online/dist/sockets/sendfile.c.html

#define min(X, Y) (((X) < (Y)) ? (X) : (Y))

ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count) {
  off_t orig;
  char buf[BUF_SIZE];
  size_t toRead, numRead, numSent, totSent;

  if (offset != NULL) {

    /* Save current file offset and set offset to value in '*offset' */

    orig = lseek(in_fd, 0, SEEK_CUR);
    if (orig == -1)
      return -1;
    if (lseek(in_fd, *offset, SEEK_SET) == -1)
      return -1;
  }

  totSent = 0;

  while (count > 0) {
    toRead = min(BUF_SIZE, count);

    numRead = read(in_fd, buf, toRead);
    if (numRead == -1)
      return -1;
    if (numRead == 0)
      break;           /* EOF */

    numSent = write(out_fd, buf, numRead);
    if (numSent == -1)
      return -1;
    if (numSent == 0) {       /* Should never happen */
      printf("sendfile: write() transferred 0 bytes");
    }

    count -= numSent;
    totSent += numSent;
  }

  if (offset != NULL) {

    /* Return updated file offset in '*offset', and reset the file offset
      to the value it had when we were called. */

    *offset = lseek(in_fd, 0, SEEK_CUR);
    if (*offset == -1)
      return -1;
    if (lseek(in_fd, orig, SEEK_SET) == -1)
      return -1;
  }

  return totSent;
}

uint16_t mt_rand16(void) {
  //uint16_t temp = genrand64_int64() & 0xffff;
  return genrand64_int64() & 0xffff;
  
} // mt_rand16()...

struct arguments parse_args(int argc, char **argv) {
 //int i;
  //for(i = 0 ; i < argc ; i++){
  // printf("%s\n", argv[i]);
  //}
  
  struct arguments facts;

  facts.num_of_chunks = atoi(argv[2]);
  //char buffer[75];
  char * buffer = malloc(30 * sizeof(char*));
  strcpy(buffer, argv[1]);
  facts.input_file_name = buffer;

  //printf("Hello|%s|\n", facts.input_file_name);

  return facts;
} // parse_args()...

void encodeMath(struct chunk *output, uint16_t *input, long SIZE, struct arguments facts, FILE **clist, int last){
  int i, t, x;

  for(x = 0 ; x < facts.num_of_chunks ; x++){
    for(i = 0, t = 0; i < SIZE + last ; t++, i += 3){
      output[x].output[t] = GF16mul(output[x].coef[0], input[i]) ^ GF16mul(output[x].coef[1], input[i+1]) ^ GF16mul(output[x].coef[2], input[i+2]);
    }

    fwrite(output[x].output, 2, (SIZE+last)/3, clist[x]);
  }
}

void encodeFile(struct arguments facts){
  long file_size_bytes = 0;
  long DATA_LENGTH = 0; //16bit elements
  long REMAINDER = 0;

  struct chunk *output = malloc(sizeof(struct chunk) * facts.num_of_chunks);
  
  //buffer size set to 1MB in 16bit
  //need extra 16bit so its divisble by 3
  long bufsize = 500001;
  long count = 0;
  int whitespace = 0;
  int oddBytes = 0;
  
  int i, x;
  
  //get file input
  FILE *fp;
  fp = fopen(facts.input_file_name, "r+");

  FILE **clist = NULL;
  
  if(fp != NULL){
    if (fseek(fp, 0, SEEK_END) == 0) {
      //Get the size of the file. Gets in single bytes
      file_size_bytes = ftell(fp);
      if (file_size_bytes == -1) { /* Error */ }
        Die("Some error");
      if( file_size_bytes % 2 == 0){
        DATA_LENGTH = file_size_bytes/2;
      } else {
        //not divisible by 2
        //Which makes it a problem when outputting 16bits
        //temporary solution. store somewhere to remember
        DATA_LENGTH = (file_size_bytes + 1)/2;
        oddBytes = 1;
      }

      if(DATA_LENGTH%3 == 0){
        whitespace = 0;
      } else {
        whitespace = 3 - (DATA_LENGTH%3);
      }
      
      //Allocate our buffer
      uint16_t *buffer = malloc(sizeof(uint16_t) * (bufsize));

      //get chunks ready
      clist = malloc(sizeof(FILE*) * facts.num_of_chunks);

      for(i = 0 ; i < facts.num_of_chunks ; i++){
      //make coefficients
      for(x = 0; x < 3; x++) {
        //make sure coefficients aren't 0
        while(1) {
          output[i].coef[x] = mt_rand16();
          if(output[i].coef[x] != 0)
            break;
        }
      }
      output[i].output = malloc(sizeof(uint16_t) * bufsize);
      output[i].numEmpty = whitespace;

      //open file for writing out header
      char namebuf[30];
      sprintf(namebuf, "%s-%d", facts.input_file_name, i);
      
      clist[i] = fopen(namebuf , "w"); //w for write, may need append later

      //long chunk_size_w_header = (((DATA_LENGTH + whitespace)/3)*2) + (sizeof(char)*30) + sizeof(long) + (sizeof(uint16_t)*3) + (sizeof(int)*2);
      uint32_t chunk_size_w_header = (((DATA_LENGTH + whitespace)/3)*2) + (sizeof(uint16_t)*3) + sizeof(uint32_t);
      
      //write out 3 16bit coefficients
      fwrite(output[i].coef, 2, 3, clist[i]);

      //write out 16bit padding
      uint16_t none = 0;
      fwrite(output[i].coef, 2, 1, clist[i]);

      uint32_t *fs = &chunk_size_w_header;
      fwrite(&fs, sizeof(uint32_t), 1, clist[i]);

      printf("file size = %" PRIu32 "\n", chunk_size_w_header);
      printf("chunk_size = %ld\n", ((DATA_LENGTH + whitespace)/3)*2);

      //int *p = &whitespace;
      //fwrite(p, sizeof(int), 1, clist[i]);

      //int *b = &oddBytes;
      //fwrite(b, sizeof(int), 1, clist[i]);
    }

    //printf("Before while loop\n");

      while(1){
        if(count >= file_size_bytes){
          //end of file
          break;
        } else {
          //Go to offset of file which is stored in count
          if (fseek(fp, count, SEEK_SET) != 0) { /* Error */ }
          
          //Read the bufsize lenght of file into memory.
          //size_t newLen = fread(input, sizeof(uint16_t), bufsize, fp);
          size_t newLen = fread(buffer, sizeof(uint16_t), bufsize, fp);
          
          if (newLen == 0) {
            fputs("Error reading file", stderr);
          }
          
          //long offset = (count/2);
          if(count + (bufsize*2) > file_size_bytes){
           REMAINDER = bufsize;
           if(oddBytes == 0){
            REMAINDER = (file_size_bytes - count);
           } else {
       REMAINDER = (file_size_bytes - count + 1);
           }
           //memcpy(input+offset, buffer, REMAINDER);
           //add whitespace to end
           //THIS IS EXPERIMENTAL CODE. NEED TO CONFIRM
           for(i = whitespace ; i > 0 ; i--){
             buffer[REMAINDER+whitespace-i] = 0;
           }
           
           if(oddBytes == 1){
            REMAINDER += 1;
           }
          } else {
           //memcpy(input+offset, buffer, bufsize*2);
          }
          

          //increment count
          //fseek offset is in bytes. bufsize is in 16bit so * by 2
          count = count + (bufsize*2);

          if(REMAINDER == 0){
           encodeMath(output, buffer, bufsize, facts, clist, 0);
          } else {
           encodeMath(output, buffer, REMAINDER/2, facts, clist, whitespace);
          }
        }
      }
    }
  }
  
  fclose(fp);
  for(i = 0 ; i < facts.num_of_chunks ; i++){
   fclose(clist[i]);
   free(output[i].output);
  }
  free(clist);
  free(output);
}


void Die(char *mess) { perror(mess); exit(1); }
//
//
//
void connectAndSend(char *SERVER_ADDRESS, char *FILE_TO_SEND){
  //server code
  int server_socket;
  ssize_t len;
  int fd;
  int sent_bytes = 0;
  char file_size[256];
  struct stat file_stat;
  off_t offset;
  int remain_data;

  struct sockaddr_in echoserver;

 // Create the TCP socket 
 if ((server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
  Die("Failed to create socket");
 }

 // Construct server information
 memset(&echoserver, 0, sizeof(echoserver));
 echoserver.sin_family = AF_INET;
 echoserver.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
 echoserver.sin_port = htons(PORT_NUMBER);

 // Establish Connection
 if(connect(server_socket,(struct sockaddr *) &echoserver, sizeof(echoserver)) < 0){
   Die("Failed to connect with server");
 } else {
  printf("Connected to server");
 }

  fd = open(FILE_TO_SEND, O_RDONLY);
  if (fd == -1){
    fprintf(stderr, "Error opening file --> %s", strerror(errno));

    exit(EXIT_FAILURE);
  }

  // Get file stats 
  if (fstat(fd, &file_stat) < 0){
    fprintf(stderr, "Error fstat --> %s", strerror(errno));

    exit(EXIT_FAILURE);
  }

  fprintf(stdout, "File Size: \n%d bytes\n", (int)file_stat.st_size);

  sprintf(file_size, "%d", (int)file_stat.st_size);

  //send message
  struct MessageHeader message_header;
  message_header.Type = TYPE_POST_CHUNK;
  memcpy(message_header.params, &file_stat.st_size, sizeof(int));
  len = send(server_socket, (void *)&message_header, sizeof(struct MessageHeader), 0);

  if (len < 0){
   fprintf(stderr, "Error on sending message --> %s", strerror(errno));

   exit(EXIT_FAILURE);
  }

  fprintf(stdout, "Server sent %d bytes for the size\n", (int)len);

  offset = 0;
  remain_data = file_stat.st_size;
  // Sending file data 
  while (((sent_bytes = sendfile(server_socket, fd, &offset, BUFSIZ)) > 0) && (remain_data > 0)){
    fprintf(stdout, "1. Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, (int)offset, remain_data);
    remain_data -= sent_bytes;
    fprintf(stdout, "2. Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, (int)offset, remain_data);
  }

  close(server_socket);
}

//
//
//
void sendChunks(struct arguments facts){
  int ports[3] = {3000, 30001, 3002};
  char *address[3] = {"127.0.0.1", "127.0.0.1", "127.0.0.1"};

  int i;
  for(i = 0 ; i < facts.num_of_chunks; i++){
    char name_buf[30];
    sprintf(name_buf, "%s-%d", facts.input_file_name, i);
    connectAndSend(address[i], name_buf);
  }
}



//
//
//
int main(int argc, char **argv){
  //struct that holds the passed arguments
  struct arguments facts;
  //input format
  //2 argument
  //--file to read
  //--number of chunks to create
  //check input
  if(argc == 3){
    //2 arguments
    facts = parse_args(argc, argv);
  } else {
    //too many arguments
    printf("Invalid input. Valid arguments:\n");
    printf("Filename and number of chunks to make.\n");
    return 0;
  }

  // Initialize GF
  GF16init();
  
  //gets file and encodes
  encodeFile(facts);

  //sends chunks to media server
  sendChunks(facts);
  
  //printf("Done\n");
  
  return 0;
}




