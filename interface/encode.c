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
    char * buffer = malloc(30 * sizeof(char*));
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
		fwrite(output[x].output, 2, (SIZE+last)/3, clist[x]);

		/*
		if(last == 1){
			//write out whitespace
			uint16_t *ws = malloc(sizeof(uint16_t) * output[x].numEmpty);
			fwrite(ws, 2, output[x].numEmpty, clist[x]);

			free(ws);
		}
		*/
	}

	//printf("size = %ld\n", SIZE+last);
}


//
//
//
void
encodeFile(struct arguments facts){
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

		        char namebuf[30];
		        int name_counter = sprintf(namebuf, "%s-%d", facts.input_file_name, i);
		        
		        clist[i] = fopen(namebuf , "w"); //w for write, may need append later

		        //write out file name with appended id, file size, coefficients
		        fwrite(namebuf, sizeof(char), name_counter+1, clist[i]);

		        long chunk_size_w_header = (((DATA_LENGTH + whitespace)/3)*2) + (sizeof(char)*(name_counter+1)) + sizeof(long) + (sizeof(uint16_t)*3) + (sizeof(int)*2);
		        long *fs = &chunk_size_w_header;
		        fwrite(fs, sizeof(long), 1, clist[i]);

		        fwrite(output[i].coef, 2, 3, clist[i]);

		        //printf("file size = %ld\n", chunk_size_w_header);
        		//printf("chunk_size = %ld\n", ((DATA_LENGTH + whitespace)/3)*2);

		        int *p = &whitespace;
		        fwrite(p, sizeof(int), 1, clist[i]);

		        int *b = &oddBytes;
		        fwrite(b, sizeof(int), 1, clist[i]);
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
    
    //printf("Done\n");
    
    return 0;
}




