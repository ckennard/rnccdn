#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// #include <assert.h> 
// google test...
//dont have array of chunks. (IMPORTANT)
//---Have only 1 chunk at a time write it out, then do the next one.

#include "gf.h"
#include "mt64.h"
#include "../common/message_header.h"

#define BUFFSIZE 32
#define MAX_PATH 256

void Die(char *mess) { perror(mess); exit(1); }

//
//
//
struct arguments {
  int num_of_chunks;
  char * input_file_name;
  char * chunks[3];
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

//
// lksd;lk
//
uint16_t
mt_rand16(void)
{
  //uint16_t temp = genrand64_int64() & 0xffff;
  return genrand64_int64() & 0xffff;
  
} // mt_rand16()...

//
// getopts...
//
//void
struct arguments
parse_args(int argc,
      char **argv
      )
{
  /*
   printf("hello\n");
   int c;
   while ((c = getopt(argc, argv, "fn:")) != -1){
   switch (c) {
   case 'f':
   //
   printf("f\n");
   break;
   case 'n':
   //
   printf("n\n");
   break;
   default:
   abort();
   }
   }
   */
  /*
  facts->num_of_chunks = atoi(argv[2]);
  char buffer[75];
  strcpy(buffer, argv[1]);
  facts->input_file_name = buffer;
   */
  struct arguments facts;

  //set name of file
  char *name_buffer = malloc(75 * sizeof(char*));
  strcpy(name_buffer, argv[1]);
  facts.input_file_name = name_buffer;

  //set chunk 1 name
  char * buffer1 = malloc(75 * sizeof(char*));
  //strcpy(buffer1, argv[1]);
  sprintf(buffer1, "%s-%d", facts.input_file_name, 0);
  facts.chunks[0] = buffer1;
  
  //set chunk 2 name
  char * buffer2 = malloc(75 * sizeof(char*));
  //strcpy(buffer2, argv[2]);
  sprintf(buffer2, "%s-%d", facts.input_file_name, 1);
  facts.chunks[1] = buffer2;

  //set chunk 3 name  
  char * buffer3 = malloc(75 * sizeof(char*));
  //strcpy(buffer3, argv[3]);
  sprintf(buffer3, "%s-%d", facts.input_file_name, 2);
  facts.chunks[2] = buffer3;

  //printf("%s\n%s\n%s\n%s\n", facts.input_file_name, facts.chunks[0], facts.chunks[1], facts.chunks[2]);

  return facts;
} // parse_args()...

//
//
//
void
decodeMath(struct chunk *output, long CHUNK_SIZE, FILE *finalFile, int last){
  int i, t;

  uint16_t *final;
  
  //allocate space for final file
  if ((final = malloc(sizeof(uint16_t) * CHUNK_SIZE*3)) == NULL) { // 16bit
    perror("malloc");
    exit(1);
  }
  
  //set up c variables
  uint16_t c0;
  uint16_t c1;
  uint16_t c2;
  uint16_t c3;
  uint16_t c4;
  
  //do math for c variables
  c0 = GF16mul(output[1].coef[0], output[0].coef[1]) ^ GF16mul(output[0].coef[0], output[1].coef[1]);
  c1 = GF16mul(output[1].coef[0], output[0].coef[2]) ^ GF16mul(output[0].coef[0], output[1].coef[2]);
  c2 = GF16mul(output[2].coef[0], output[0].coef[1]) ^ GF16mul(output[0].coef[0], output[2].coef[1]);
  c3 = GF16mul(output[2].coef[0], output[0].coef[2]) ^ GF16mul(output[0].coef[0], output[2].coef[2]);
  c4 = GF16mul(c1, c2) ^ GF16mul(c0, c3);
  
  uint16_t *t0 = malloc(CHUNK_SIZE * sizeof *t0);
  uint16_t *t1 = malloc(CHUNK_SIZE * sizeof *t1);
  
  for(i = 0 ; i < CHUNK_SIZE ; i++){
    t0[i] = GF16mul(output[1].coef[0], output[0].output[i]) ^ GF16mul(output[0].coef[0], output[1].output[i]);
    t1[i] = GF16mul(output[2].coef[0], output[0].output[i]) ^ GF16mul(output[0].coef[0], output[2].output[i]);
  }
  
  uint16_t *x1 = malloc(CHUNK_SIZE * sizeof *x1);
  uint16_t *x2 = malloc(CHUNK_SIZE * sizeof *x2);
  uint16_t *x3 = malloc(CHUNK_SIZE * sizeof *x3);
  
  for(i = 0 ; i < CHUNK_SIZE ; i++){
    x2[i] = GF16div((GF16mul(c2, t0[i]) ^ GF16mul(c0, t1[i])),c4);
    x1[i] = GF16div((t0[i] ^ GF16mul(c1, x2[i])),c0);
    x3[i] = GF16div((output[0].output[i] ^ GF16mul(output[0].coef[1], x1[i]) ^ GF16mul(output[0].coef[2], x2[i])),output[0].coef[0]);
  }
  
  for(i = 0, t = 0 ; i < CHUNK_SIZE*3 ; i +=3, t++){
    //final[i] = x1[t];
    //final[i+1] = x2[t];
    //final[i+2] = x3[t];
    final[i] = x3[t];
    final[i+1] = x1[t];
    final[i+2] = x2[t];
  }

  if(last == 0){
    fwrite(final, 2, CHUNK_SIZE*3, finalFile);
  } else {
    if(output[0].oddBytes == 0){
      fwrite(final, 2, (CHUNK_SIZE*3) - output[0].numEmpty, finalFile);
    } else {
      fwrite(final, 2, (CHUNK_SIZE*3) - output[0].numEmpty - 1, finalFile);
      //write out first byte of uint16 
      //subtract whitespace we dont want, -1 to get to last one
      fwrite(final+((CHUNK_SIZE*3)-output[0].numEmpty - 2), 1, 1, finalFile);
    }
  }

  free(final);
  free(t0);
  free(t1);
  free(x1);
  free(x2);
  free(x3);

  //printf("chunk length passed = %ld\n", CHUNK_SIZE);
  //printf("write length = %ld\n", CHUNK_SIZE*3);

}

//
//
//
void
decodeFile(struct arguments facts){
  long file_size_bytes = 0;
  long CHUNK_LENGTH = 0;
  //long DATA_LENGTH = 0; //16bit elements
  long REMAINDER = 0; //set to same size as bufsize, changed later when needed
  
  //buffer size set to 1MB for 16-bit
  //1 extra 16bit so divisible by 3
  long bufsize = 500001;
  
  //start of file has 3 16bit numbers and an int for whitespace and int for oddbyte
  long count = 0;
  //long countOther = sizeof(uint16_t)*3 + sizeof(int) + sizeof(int);
  long countOther = 0;
  
  //int whitespace = 0;
  
  struct chunk output[3];
  int i;
  
  //set up 3 chunks
  for(i = 0 ; i < 3 ; i++){
    struct chunk out;
    out.numEmpty = 0;
    out.oddBytes = 0;
    out.output = malloc(sizeof(uint16_t) * (bufsize));
    output[i] = out;
  }
  
  //printf("Starting read\n");

  FILE *ch1, *ch2, *ch3, *ff;
  ch1 = fopen(facts.chunks[0], "r");
  ch2 = fopen(facts.chunks[1], "r");
  ch3 = fopen(facts.chunks[2], "r");
  ff = fopen("final_file" , "w" );

  FILE *clist[3];
  clist[0] = ch1;
  clist[1] = ch2;
  clist[2] = ch3;

    
  if(ch1 != NULL && ch2 != NULL && ch3 != NULL){
    if (fseek(ch1, 0L, SEEK_END) == 0 && fseek(ch2, 0L, SEEK_END) == 0 && fseek(ch3, 0L, SEEK_END) == 0) {
      //Get the size of the file. Gets in single bytes
      file_size_bytes = ftell(ch1);
      if (file_size_bytes == -1) { /* Error */ }
      
      //subtract coefficients and whitespace here
      //CHUNK_LENGTH = (file_size_bytes - (sizeof(uint16_t)*3) - sizeof(int) - sizeof(int))/2;
      //DATA_LENGTH = CHUNK_LENGTH*3;
      
      //go to beginning of file
      if (fseek(ch1, 0, SEEK_SET) != 0) { /* Error */ }
      if (fseek(ch2, 0, SEEK_SET) != 0) { /* Error */ }
      if (fseek(ch3, 0, SEEK_SET) != 0) { /* Error */ }
      
      //getting header info
      for(i = 0 ; i < 3 ; i++){
        size_t newLen;

        //get file name
        char namebuf[30];
        newLen = fread(namebuf, sizeof(char), 30, clist[i]);
        /*
        char namebuf[30];
        for(x = 0 ; x < 30 ; x++){
          char temp[1];
          newLen = fread(temp, sizeof(char), 1, clist[i]);
          namebuf[x] = temp[0];
          if(temp[0] == '\0'){
            break;
          }
        }
        */
        //printf("File name = %s\n", namebuf);

        //get file size
        long *fs = malloc(sizeof(long));
        newLen = fread(fs, sizeof(long), 1, clist[i]);
        
        //get chunk info from file length
        //countOther = (sizeof(char)*(x+1)) + sizeof(long) + (sizeof(uint16_t)*3) + (sizeof(int)*2); 
        //CHUNK_LENGTH = (file_size_bytes - (sizeof(char)*(x+1)) - sizeof(long) - (sizeof(uint16_t)*3) - (sizeof(int)*2))/2;
        countOther = (sizeof(char)*30) + sizeof(long) + (sizeof(uint16_t)*3) + (sizeof(int)*2); 
        CHUNK_LENGTH = (file_size_bytes - (sizeof(char)*30) - sizeof(long) - (sizeof(uint16_t)*3) - (sizeof(int)*2))/2;

        //printf("File size = %ld\n", fs[0]);
        //printf("Chunk length = %ld\n", CHUNK_LENGTH);

        //get coefficients
        newLen = fread(output[i].coef, sizeof(uint16_t), 3, clist[i]);
        
        //getting whitespace count
        int *p = malloc(sizeof(int));
        newLen = fread(p, sizeof(int), 1, clist[i]);
        output[i].numEmpty = *p;
        
        if (newLen == 0) {
          fputs("Error reading file", stderr);
        }

        //getting oddbytes variable
        int *b = malloc(sizeof(int));
        newLen = fread(b, sizeof(int), 1, clist[i]);
        output[i].oddBytes = *b;
        

        if (newLen == 0) {
          fputs("Error reading file", stderr);
        }


        /*
        printf("coef %d %d %d and whitespace %d and odd bytes %d\n",
            output[i].coef[0],
            output[i].coef[1],
            output[i].coef[2],
            output[i].numEmpty,
            output[i].oddBytes);
         */
      }
      
      uint16_t *buffer = malloc(sizeof(uint16_t) * (bufsize));
      
      while(1){
        if((count/2) >= CHUNK_LENGTH){
          //Done reading file
          break;
        } else {
          for(i = 0 ; i < 3 ; i++){
            if (fseek(clist[i], count+countOther, SEEK_SET) != 0) { /* Error */ }
            
            //Read the bufsize lenght of file into memory.
            //size_t newLen = fread(input, sizeof(uint16_t), bufsize, fp);
            size_t newLen = fread(buffer, sizeof(uint16_t), bufsize, clist[i]);
            
            if (newLen == 0) {
              fputs("Error reading file", stderr);
            }/* else {
             source[newLen++] = '\0'; //Just to be safe.
             }*/
            
            //add buffer to input
            //count is in bytes so need to divide by 2 to get 16bit
            //long offset = (count/2);
            //memcpy(output[i].output+offset, buffer, sizeof(uint16_t));
            
            if(count + (bufsize*2) > CHUNK_LENGTH*2){
              //int remain = bufsize;
              //remain = (CHUNK_LENGTH*2 - count);
              REMAINDER = (CHUNK_LENGTH*2 - count);
              //memcpy(output[i].output+offset, buffer, remain);
              memcpy(output[i].output, buffer, REMAINDER);
            } else {
              //memcpy(output[i].output+offset, buffer, bufsize*2);
              memcpy(output[i].output, buffer, bufsize*2);
            }

          }
          
          //increment count
          //fseek offset is in bytes. bufsize is in 16bit so * by 2
          count = count + (bufsize*2);

          //do math
          //...
          if(REMAINDER == 0){
            //printf("BUFSIZE  = %ld\n", bufsize);
            decodeMath(output, bufsize, ff, 0);
          } else {
            //printf("REMAINDER = %ld\n", REMAINDER);
            decodeMath(output, REMAINDER/2, ff, 1);
          }
        }
      }
    }
  }
  
  fclose(ch1);
  fclose(ch2);
  fclose(ch3);
  fclose(ff);
}

void receiveFile(char *filename, char *chunk_name, char *ipaddress, int port, int chunk_num){
  int sock, result;
  struct sockaddr_in echoserver;
  char buffer[BUFFSIZE];
  //unsigned int echolen;
  int received = 0;
  int message_size = 0;
  struct MessageHeader message_header, response_message_header;
  memset(&message_header, 0, sizeof(struct MessageHeader));

  /* Create the TCP socket */
  if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
    Die("Failed to create socket\n");
  }

  /* Construct server information */
  memset(&echoserver, 0, sizeof(echoserver));
  echoserver.sin_family = AF_INET;
  echoserver.sin_addr.s_addr = inet_addr(ipaddress);
  echoserver.sin_port = htons(port);

  /* Establish Connection */
  if(connect(sock,(struct sockaddr *) &echoserver, sizeof(echoserver)) < 0){
    Die("Failed to connect with server\n");
  } else {
    printf("Connected to server\n");
  }
  
  message_header.Type = TYPE_REQUEST_CHUNK;
  memcpy(message_header.params, filename, strlen(filename)+1);

  if (send(sock, (void *)&message_header, sizeof(struct MessageHeader), 0) != sizeof(struct MessageHeader)) {
    Die("Mismatch in number of sent bytes\n");
  }

  remove(chunk_name);
  FILE *fp = fopen(chunk_name, "w");
  if (fp == NULL){
    printf("Error opening/creating file.\n");
    exit(1);
  }

  //receive message header back
  if((result = receive_message_header(sock, &response_message_header)) < 0) {
    Die("failed to receive response header:\n");
  }

  //expect TYPE_POST_CHUNK as response
  if(response_message_header.Type != TYPE_POST_CHUNK) {
    Die("message response incorrect type");
  }

  memcpy((void *)&message_size, (void *)response_message_header.params, sizeof(int)); 

  /* Receive the contents of the chunk*/
  fprintf(stdout, "Received: ");
  while (received < message_size) {
    int bytes = 0;
    if ((bytes = recv(sock, buffer, BUFFSIZE-1, 0)) < 1) {
      Die("Failed to receive bytes from server\n");
    }
    received += bytes;
    buffer[bytes] = '\0';    /* Assure null terminated string */
    //fprintf(stdout, buffer);
    fprintf(fp, "%s", buffer);
  }
  fclose(fp);
  fprintf(stdout, "\n");
  close(sock);
}


void fetchFile(struct arguments facts){
  char filename[MAX_PATH];  //set up charbuffer for filename
  memset(filename, 0, MAX_PATH);

  //strcpy(filename, facts.chunks[0]);  //assign filename from args to filename v0ariable
  //filename[strlen(filename)-2] = '\0';   //trim off the -1

  printf("Fetching %s...\nchunk 1/3...\n", filename);
  //receiveFile(filename, "127.0.0.1", 3000, 0);
  receiveFile(facts.input_file_name, facts.chunks[0], "127.0.0.1", 3000, 0);

  return;

  printf("Chunk 2/3...\n");
  //receiveFile(filename, "127.0.0.1", 3000, 1);
  receiveFile(facts.input_file_name, facts.chunks[1], "127.0.0.1", 3000, 1);

  printf("Chunk 3/3...\n");
  //receiveFile(filename, "127.0.0.1", 3000, 2);
  receiveFile(facts.input_file_name, facts.chunks[2], "127.0.0.1", 3000, 2);
}

//
// Use getopts()
//
int main(int argc, char **argv){
  //struct that holds the passed arguments
  struct arguments facts;
  //input format
  //1 argument
  //--file to fetch and decode
  //check input
  if(argc == 2){
    //3 arguments
    facts = parse_args(argc, argv);
  } else {
    //too many arguments
    printf("Invalid input. Valid arguments:\n");
    printf("File name to fetch and decode.\n");
    return 0;
  }
  
  // Initialize GF
  GF16init();
  
  //fetches files from media server
  fetchFile(facts);

  return 0;

  //gets file and encodes
  decodeFile(facts);
  
  //printf("Done\n");
  
  return 0;
  
}
