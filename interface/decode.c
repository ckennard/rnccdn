#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// #include <assert.h>
// google test...
//dont have array of chunks. (IMPORTANT)
//---Have only 1 chunk at a time write it out, then do the next one.

#include "gf.h"
#include "mt64.h"

#define SPACE  600000
//#define DATA_LENGTH   SPACE / sizeof(uint16_t)

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

    char * buffer1 = malloc(75 * sizeof(char*));
    strcpy(buffer1, argv[1]);
    facts.chunks[0] = buffer1;
    
    char * buffer2 = malloc(75 * sizeof(char*));
    strcpy(buffer2, argv[2]);
    facts.chunks[1] = buffer2;
    
    char * buffer3 = malloc(75 * sizeof(char*));
    strcpy(buffer3, argv[3]);
    facts.chunks[2] = buffer3;

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
                        //printf("BUFSIZE   = %ld\n", bufsize);
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

//
// Use getopts()
//
int main(int argc, char **argv){
    //struct that holds the passed arguments
    struct arguments facts;
    //input format
    //2 argument
    //--file to read
    //--number of chunks to create
    //check input
    if(argc == 4){
        //3 arguments
        facts = parse_args(argc, argv);
    } else {
        //too many arguments
        printf("Invalid input. Valid arguments:\n");
        printf("Chunk names to decode.\n");
        return 0;
    }
    
    // Initialize GF
    GF16init();
    
    //gets file and encodes
    decodeFile(facts);
    
    //printf("Done\n");
    
    return 0;
    
}
