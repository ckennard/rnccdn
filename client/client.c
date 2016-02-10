#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define BUFFSIZE 32
void Die(char *mess) { perror(mess); exit(1); }
int main(int argc, char *argv[]) {
      int sock;
      struct sockaddr_in echoserver;
      char buffer[BUFFSIZE];
      unsigned int echolen;
      char *filename = "test_file.html";
      int message_size = 19564;  //This will have to be generated dynamically somehow.
      int received = 0;
      int port = 3000;

      /* Catches too many arguments */
      if (argc != 3) {
        fprintf(stderr, "USAGE: client <server_ip> <word>\n");
        exit(1);
      }

      /* Create the TCP socket */
      if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        Die("Failed to create socket\n");
      }

      /* Construct server information */
      memset(&echoserver, 0, sizeof(echoserver));
      echoserver.sin_family = AF_INET;
      echoserver.sin_addr.s_addr = inet_addr(argv[1]);
      echoserver.sin_port = htons(port);

      /* Establish Connection */
      if(connect(sock,(struct sockaddr *) &echoserver, sizeof(echoserver)) < 0){
            Die("Failed to connect with server\n");
      } else {
        printf("Connected to server\n");
      }

      /* Send a word to the server */
      echolen = strlen(argv[2]);
      if (send(sock, argv[2], echolen, 0) != echolen) {
        Die("Mismatch in number of sent bytes\n");
      }

      remove(filename);
      FILE *fp = fopen(filename, "w");
      if (fp == NULL){
        printf("Error opening/creating file.\n");
        exit(1);
      }

      /* Receive the word back from the server */
      fprintf(stdout, "Received: ");
      while (received < message_size) {
            int bytes = 0;
            if ((bytes = recv(sock, buffer, BUFFSIZE-1, 0)) < 1) {
                  Die("Failed to receive bytes from server\n");
            }
            received += bytes;
            buffer[bytes] = '\0';        /* Assure null terminated string */
            fprintf(stdout, buffer);
            fprintf(fp, buffer);
      }
      fclose(fp);
      fprintf(stdout, "\n");
      close(sock);
      exit(0);
}
