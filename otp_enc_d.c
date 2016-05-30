/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void handleIncoming(int connection);

char* getFile(int connection, char* name);


int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int pid;
    int yes = 1;

    if (argc < 2) {
      fprintf(stderr,"ERROR, no port provided\n");
      exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
      error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    //allows us to reuse the socket
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    //listen for up to 5 incoming connection
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    //fork off child processes for incoming connections
    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");

        if (!fork()) {
            //child doens't need the listener
            close(sockfd);
            //receive incoming files and store filenames
            handleIncoming(newsockfd);
            exit(0);
        }
    }


    // FILE *fp;
    // fp = fopen(plaintext, "r");
    // char ch;
    //
    // while ((ch = getc(fp)) != EOF) {
    //   if (!isupper(ch) && ch != ' ') {
    //     printf("found a bad character");
    //   }
    // }

    return 0;
}


void handleIncoming(int connection){

    char *plaintext, *ciphertext;
    //char buffer[1024];
    int n;

    plaintext = getFile(connection, "temp_plaintext");
    //bzero(buffer,1024);
    n = write(connection,"confirmed",18);
    if (n < 0) error("ERROR writing to socket");
    ciphertext = getFile(connection, "temp_ciphertext");
    close(connection);
}

char* getFile(int connection, char* name){
    char buffer[1024];

    //file to temporarily store stuff
    FILE *fp;
    //will use for temporary file
    int size = 22;
    char *tempname = malloc(size);
    int pid = getpid();
    snprintf(tempname, size, "%s%d", name, pid);
    fp = fopen(tempname, "a");
    //loop to accept incoming messages
    int numbytes = 0;
    while ((numbytes = recv(connection, buffer, 1024, 0)) > 0) {
        printf("%s\n", buffer);
        fwrite(buffer,sizeof(char), numbytes, fp);
        bzero(buffer, 1024);
        //if less than 1024 we are on the last chunk, so break
        if (numbytes < 1024) {
          break;
        }
    }
    fclose(fp);
    return tempname;
}
