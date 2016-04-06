CC= gcc
CC_FLAGS= -Wall -O2 -g
OUTPUT_DIR= ./bin
COMMON_HEADERS = ../common/*.h

media_admin:
	$(CC) $(CC_FLAGS) $(COMMON_HEADERS) media_admin.c -o $(OUTPUT_DIR)/media_admin

media_client:
	$(CC) $(CC_FLAGS) $(COMMON_HEADERS) media_client.c -o $(OUTPUT_DIR)/media_client

all: media_admin media_client

clean:
	rm -rf $(OUTPUT_DIR)
