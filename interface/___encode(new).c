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
//
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
	//int i;
    //for(i = 0 ; i < argc ; i++){
    //	printf("%s\n", argv[i]);
    //}
    
    struct arguments facts;

    facts.num_of_chunks = atoi(argv[2]);
    //char buffer[75];
    char * buffer = malloc(75 * sizeof(char*));
    strcpy(buffer, argv[1]);
    facts.input_file_name = buffer;

    //printf("Hello|%s|\n", facts.input_file_name);

    return facts;
} // parse_args()...

//
//
//
void
encodeMath(struct chunk *output, uint16_t *input, long SIZE, struct arguments facts, FILE **clist, int last){
	int i, t, x;

	for(x = 0 ; x <  facts.num_of_chunks ; x++){
		for(i = 0, t = 0; i < SIZE + last ; t++, i += 3){
			output[x].output[t] = GF16mul(output[x].coef[0], input[i]) ^ GF16mul(output[x].coef[1], input[i+1]) ^ GF16mul(output[x].coef[2], input[i+2]);
		}

		//write it out
		fwrite(output[x].output, 2, SIZE/3, clist[x]);

		/*
		if(last == 1){
			//write out whitespace
			uint16_t *ws = malloc(sizeof(uint16_t) * output[x].numEmpty);
			fwrite(ws, 2, output[x].numEmpty, clist[x]);

			free(ws);
		}
		*/
	}

	//printf("size = %ld and last = %d\n", SIZE, last);
}


//
//
//
void
encodeFile(struct arguments facts){
    //uint16_t *input = NULL;
    long file_size_bytes = 0;
    long DATA_LENGTH = 0; //16bit elements
	//long chunk_length;
	long REMAINDER = 0;

	struct chunk *output = malloc(sizeof(struct chunk) * facts.num_of_chunks);
    
    //buffer size set to 1MB in 16bit
    //need extra 16bit so its divisble by 3
    long bufsize = 500001;
    long count = 0;
    int whitespace = 0;
    int oddBytes = 0;
    
    int i, x;

    printf("Starting read\n");

    //printf("Hello|%s|\n", facts.input_file_name);
    
    //get file input
    FILE *fp;
    fp = fopen(facts.input_file_name, "r+");

    FILE **clist = NULL;

    //if(fp == NULL){
    //	printf("Hello|%s|\n", facts.input_file_name);
    //}
    
    if(fp != NULL){
        if (fseek(fp, 0, SEEK_END) == 0) {
            //Get the size of the file. Gets in single bytes
            file_size_bytes = ftell(fp);
            if (file_size_bytes == -1) { /* Error */ }
            
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
            
            //set off-byte element to 0 so math can work out
            //if(oddBytes == 1){
            //	input[DATA_LENGTH - whitespace - 1] = 0;
            //}

            printf("before chunks\n");

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

		        char namebuf[100];
		        sprintf(namebuf, "%s-%d", facts.input_file_name, i);
		        
		        //FILE *tempfp = fopen(namebuf , "w" ); //w for write, may need append later
		        clist[i] = fopen(namebuf , "w"); //w for write, may need append later

		        //write out coef, whitespace, and oddbyte
		        fwrite(output[i].coef, 2, 3, clist[i]);
        
		        int *p = &whitespace;
		        fwrite(p, sizeof(int), 1, clist[i]);

		        int *b = &oddBytes;
		        fwrite(b, sizeof(int), 1, clist[i]);
		    }

		    printf("Before while loop\n");

            while(1){
                if(count >= file_size_bytes){
                    //end of file
                    //set empty bytes to end
                    /*
                    for(i = whitespace ; i > 0 ; i--){
                        input[DATA_LENGTH-i] = 0;
                    }
                    */
                    break;
                } else {
                    //Go back to the start of the file.
                    //if (fseek(fp, 0L, SEEK_SET) != 0) { /* Error */ }
                    //Go to offset of file which is stored in count
                    if (fseek(fp, count, SEEK_SET) != 0) { /* Error */ }
                    
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
                    //long offset = (count/2);
                    //memcpy(input+offset, buffer, sizeof(uint16_t));

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
                    	for(i = 0 ; i < whitespace ; i++){
	                        buffer[REMAINDER+i] = 0;
	                    }
	                    REMAINDER += whitespace;
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
    

    return;
/*
    printf("Done reading\n");
    printf("Starting encode and write\n");
    
    //set up chunk memoryto be used for each chunk
    
    chunk_length = (DATA_LENGTH + whitespace)/3;
    if ((out.output = malloc( chunk_length * sizeof(out.output))) == NULL) { // 16bit
        perror("malloc");
        exit(1);
    }
    
    
    //for loop:
    //--makes coefficients
    //--encodes
    //--writes out
    for(j = 0 ; j < facts.num_of_chunks ; j++){
        //make coefficients
        
        for(x = 0; x < 3; x++) {
            //make sure coefficients aren't 0
            while(1) {
                out.coef[x] = mt_rand16();
                if(out.coef[x] != 0)
                    break;
            }
        }
        
        out.numEmpty = whitespace;
        
        
        //for(i = 0, t = 0; i < DATA_LENGTH ; t++, i += 3){
        for(i = 0, t = 0; i < (DATA_LENGTH + whitespace) ; t++, i += 3){
            out.output[t] = GF16mul(out.coef[0], input[i]) ^ GF16mul(out.coef[1], input[i+1]) ^ GF16mul(out.coef[2], input[i+2]);
        }
        
        
        
        char namebuf[100];
        sprintf(namebuf, "%s-%d", facts.input_file_name, j);
        
        fp = fopen(namebuf , "w" ); //w for write, may need append later
        fwrite(out.coef, 2, 3, fp);
        
        int *p = &whitespace;
        fwrite(p, sizeof(int), 1, fp);

        int *b = &oddBytes;
        fwrite(b, sizeof(int), 1, fp);
        
        //write out output array
        //while(1){
        fwrite(out.output, 2, (DATA_LENGTH+whitespace)/3, fp);
        //}
        
        
        fclose(fp);

        
        printf("coef %d %d %d and whitespace %d and odd bytes %d\n",
                       out.coef[0],
                       out.coef[1],
                       out.coef[2],
                       *p,
                       *b);
		
        //printf("%d\n", sizeof(int));
                       
        
    }
    
    printf("Done encoding\n");

*/
    //fp = fopen("fileCheck" , "w+" );
    //fwrite(input, 2, DATA_LENGTH, fp);

    //for(i = 0 ; i < 10 ; i++){
    //	printf("%x\n", input[i]);
    //}
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
    
    printf("Done\n");
    
    return 0;
}




