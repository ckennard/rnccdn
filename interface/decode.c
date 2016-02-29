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
decodeFile(struct arguments facts){
    long file_size_bytes = 0;
    long CHUNK_LENGTH = 0;
    long DATA_LENGTH = 0; //16bit elements
    
    //buffer size set to 1MB
    //using 16bit units, so half of what would be in byte
    //long bufsize = 1000000;
    long bufsize = 500000;
    //start of file has 3 16bit numbers and an int for whitespace
    //long count = sizeof(uint16_t)*3 + sizeof(int) + sizeof(int);
    long count = 0;
    long countOther = sizeof(uint16_t)*3 + sizeof(int) + sizeof(int);
    
    //int whitespace = 0;
    
    struct chunk output[3];
    int i, t;
    
    //set up 3 chunks
    for(i = 0 ; i < 3 ; i++){
        struct chunk out;
        out.numEmpty = 0;
        out.oddBytes = 0;
        out.output = NULL;
        output[i] = out;
    }
    
    printf("Starting read\n");
    
    for(i = 0 ; i < 3 ; i++){
        FILE *fp;
        fp = fopen(facts.chunks[i], "r");

        count = 0;
        
        //printf("chunk %s\n", facts.chunks[i]);
        
        if(fp != NULL){
            if (fseek(fp, 0L, SEEK_END) == 0) {
                //Get the size of the file. Gets in single bytes
                file_size_bytes = ftell(fp);
                if (file_size_bytes == -1) { /* Error */ }
                
                //subtract coefficients and whitespace here
                CHUNK_LENGTH = (file_size_bytes - (sizeof(uint16_t)*3) - sizeof(int) - sizeof(int))/2;
                DATA_LENGTH = CHUNK_LENGTH*3;
                
                //go to beginning of file
                if (fseek(fp, 0, SEEK_SET) != 0) { /* Error */ }
                
                //printf("before coef\n");
                
                //getting coefficients
                size_t newLen = fread(output[i].coef, sizeof(uint16_t), 3, fp);
                
                if (newLen == 0) {
                    fputs("Error reading file", stderr);
                }
                
                //printf("after coef\n");
                
                //getting whitespace count
                int *p = malloc(sizeof(int));
                newLen = fread(p, sizeof(int), 1, fp);
                output[i].numEmpty = *p;
                
                if (newLen == 0) {
                    fputs("Error reading file", stderr);
                }

                //getting oddbytes variable
                int *b = malloc(sizeof(int));
                newLen = fread(b, sizeof(int), 1, fp);
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
                
                output[i].output = malloc(sizeof(uint16_t) * CHUNK_LENGTH);
                uint16_t *buffer = malloc(sizeof(uint16_t) * (bufsize));
                
                //printf("starting array\n");
                
                while(1){
                    if((count/2) >= CHUNK_LENGTH){
                        //Done reading file
                        break;
                    } else {
                        //Go back to the start of the file.
                        //if (fseek(fp, 0L, SEEK_SET) != 0) { /* Error */ }
                        //Go to offset of file which is stored in count
                        //if (fseek(fp, count, SEEK_SET) != 0) { /* Error */ }
                        if (fseek(fp, count+countOther, SEEK_SET) != 0) { /* Error */ }
                        
                        //Read the bufsize lenght of file into memory.
                        //size_t newLen = fread(input, sizeof(uint16_t), bufsize, fp);
                        size_t newLen = fread(buffer, sizeof(uint16_t), bufsize, fp);
                        
                        if (newLen == 0) {
                            fputs("Error reading file", stderr);
                        }/* else {
                          source[newLen++] = '\0'; //Just to be safe.
                          }*/
                        
                        //add buffer to input
                        //count is in bytes so need to divide by 2 to get 16bit
                        long offset = (count/2);
                        //memcpy(output[i].output+offset, buffer, sizeof(uint16_t));
                        
                        if(count + (bufsize*2) > CHUNK_LENGTH*2){
                            int remain = bufsize;
                            remain = (CHUNK_LENGTH*2 - count);
                            /*
                            if(input[i].oddBytes == 0){
                                remain = (file_size_bytes - count);
                            } else {
                                remain = (file_size_bytes - count + 1);
                            }
                            */
                            memcpy(output[i].output+offset, buffer, remain);
                        } else {
                            memcpy(output[i].output+offset, buffer, bufsize*2);
                        }
                        
                        //increment count
                        //fseek offset is in bytes. bufsize is in 16bit so * by 2
                        count = count + (bufsize*2);
                    }
                    
                }
            }
        }
        
        fclose(fp);
    }
    
    printf("Done reading chunks\n");
    
    printf("Starting decode\n");
    
    //variables
    uint16_t *final;
    
    //allocate space for final file
    if ((final = malloc(sizeof(uint16_t) * DATA_LENGTH)) == NULL) { // 16bit
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
    
    uint16_t *t0 = malloc(DATA_LENGTH/3 * sizeof *t0);
    uint16_t *t1 = malloc(DATA_LENGTH/3 * sizeof *t1);
    
    for(i = 0 ; i < DATA_LENGTH/3 ; i++){
        t0[i] = GF16mul(output[1].coef[0], output[0].output[i]) ^ GF16mul(output[0].coef[0], output[1].output[i]);
        t1[i] = GF16mul(output[2].coef[0], output[0].output[i]) ^ GF16mul(output[0].coef[0], output[2].output[i]);
    }
    
    uint16_t *x1 = malloc(DATA_LENGTH/3 * sizeof *x1);
    uint16_t *x2 = malloc(DATA_LENGTH/3 * sizeof *x2);
    uint16_t *x3 = malloc(DATA_LENGTH/3 * sizeof *x3);
    
    for(i = 0 ; i < DATA_LENGTH/3 ; i++){
        x2[i] = GF16div((GF16mul(c2, t0[i]) ^ GF16mul(c0, t1[i])),c4);
        x1[i] = GF16div((t0[i] ^ GF16mul(c1, x2[i])),c0);
        x3[i] = GF16div((output[0].output[i] ^ GF16mul(output[0].coef[1], x1[i]) ^ GF16mul(output[0].coef[2], x2[i])),output[0].coef[0]);
    }
    
    for(i = 0, t = 0 ; i < DATA_LENGTH ; i +=3, t++){
        //final[i] = x1[t];
        //final[i+1] = x2[t];
        //final[i+2] = x3[t];
        final[i] = x3[t];
        final[i+1] = x1[t];
        final[i+2] = x2[t];
    }
    
    printf("Done decoding\n");
    
    printf("Starting write\n");
    
    FILE *finalFile;
    finalFile = fopen("final_file" , "w" ); //change file name later
    
    //write out final array
    //fwrite(final, 2, (DATA_LENGTH)-output[0].numEmpty, finalFile); //working one

    //test to strip last byte
    if(output[0].oddBytes == 0){
        fwrite(final, 2, (DATA_LENGTH)-output[0].numEmpty, finalFile);
    } else {
        fwrite(final, 2, (DATA_LENGTH) - output[0].numEmpty - 1, finalFile);
        //write out first byte of uint16 
        //subtract whitespace we dont want, -1 to get to last one
        fwrite(final+((DATA_LENGTH)-output[0].numEmpty - 1), 1, 1, finalFile);
    }

    fclose(finalFile);

/*
    FILE * chunkCheck = fopen("fchunkCheck-0" , "w+" );
    fwrite(output[0].output, 2, CHUNK_LENGTH, chunkCheck);
    fclose(chunkCheck);

    chunkCheck = fopen("fchunkCheck-1" , "w+" );
    fwrite(output[1].output, 2, CHUNK_LENGTH, chunkCheck);
    fclose(chunkCheck);

    chunkCheck = fopen("fchunkCheck-2" , "w+" );
    fwrite(output[2].output, 2, CHUNK_LENGTH, chunkCheck);
    fclose(chunkCheck);
    */
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
    
    printf("Done\n");
    
    return 0;
    
    //intput
    //uint16_t *input;
    
    /*
     //output chunks
     struct chunk output[facts.num_of_chunks];
     int i;
     for(i = 0 ; i < facts.num_of_chunks ; i++){
     struct chunk out;
     out.numEmpty = 0;
     out.output = NULL;
     output[i] = out;
     }
     */
    
    
    
    
    /*
     //get coefficients (new)
     for (i = 0; i < 3; i++) {
     for(j = 0; j < 3; j++) {
     //make sure coefficients aren't 0
     while(1) {
     output[i].coef[j] = mt_rand16();
     if(output[i].coef[j] != 0)
     break;
     }
     }
     }
     
     //    uint16_t    a[3][3], b[3], x[3];
     //b[0] = a[0][0] * x[0] + a[0][1] * x[1] + a[0][2] * x[2];
     //b[1] = a[1][0] * x[0] + a[1][1] * x[1] + a[1][2] * x[2];
     //b[2] = a[2][0] * x[0] + a[2][1] * x[1] + a[2][2] * x[2];
     
     printf("Encoding\n");
     
     //encode section
     for(i = 0, t = 0; i < DATA_LENGTH ; t++, i += 3){
     output[0].output[t] = GF16mul(output[0].coef[0], input[i]) ^ GF16mul(output[0].coef[1], input[i+1]) ^ GF16mul(output[0].coef[2], input[i+2]);
     output[1].output[t] = GF16mul(output[1].coef[0], input[i]) ^ GF16mul(output[1].coef[1], input[i+1]) ^ GF16mul(output[1].coef[2], input[i+2]);
     output[2].output[t] = GF16mul(output[2].coef[0], input[i]) ^ GF16mul(output[2].coef[1], input[i+1]) ^ GF16mul(output[2].coef[2], input[i+2]);
     }
     */
    /*
     printf("Decoding\n");
     
     //variables
     uint16_t *final;
     
     //allocate space for final file
     if ((final = malloc(DATA_LENGTH * sizeof *input)) == NULL) { // 16bit
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
     
     uint16_t *t0 = malloc(DATA_LENGTH/3 * sizeof *t0);
     uint16_t *t1 = malloc(DATA_LENGTH/3 * sizeof *t1);
     
     for(i = 0 ; i < DATA_LENGTH/3 ; i++){
     t0[i] = GF16mul(output[1].coef[0], output[0].output[i]) ^ GF16mul(output[0].coef[0], output[1].output[i]);
     t1[i] = GF16mul(output[2].coef[0], output[0].output[i]) ^ GF16mul(output[0].coef[0], output[2].output[i]);
     }
     
     uint16_t *x1 = malloc(DATA_LENGTH/3 * sizeof *x1);
     uint16_t *x2 = malloc(DATA_LENGTH/3 * sizeof *x2);
     uint16_t *x3 = malloc(DATA_LENGTH/3 * sizeof *x3);
     
     for(i = 0 ; i < DATA_LENGTH/3 ; i++){
     x2[i] = GF16div((GF16mul(c2, t0[i]) ^ GF16mul(c0, t1[i])),c4);
     x1[i] = GF16div((t0[i] ^ GF16mul(c1, x2[i])),c0);
     x3[i] = GF16div((output[0].output[i] ^ GF16mul(output[0].coef[1], x1[i]) ^ GF16mul(output[0].coef[2], x2[i])),output[0].coef[0]);
     }
     
     for(i = 0, t = 0 ; i < DATA_LENGTH ; i +=3, t++){
     //final[i] = x1[t];
     //final[i+1] = x2[t];
     //final[i+2] = x3[t];
     final[i] = x3[t];
     final[i+1] = x1[t];
     final[i+2] = x2[t];
     }
     
     printf("Checking...\n");
     
     //check
     for( i = 0 ; i < DATA_LENGTH ; i++){
     if(input[i] != final[i]){
     printf("Not equal at %d\n", i);
     printf("%d and %d\n", input[i], final[i]);
     //printf("%d and %d\n", input[i+1], final[i+1]);
     //printf("%d and %d\n", input[i+2], final[i+2]);
     break;
     }
     }
     */
    
    //printf("Done\n");
    
    /*
     free(input);
     free(output[0].output);
     free(output[1].output);
     free(output[2].output);
     */
    /*
     free(final);
     free(x1);
     free(x2);
     free(x3);
     */
    
    //return 0;
}




