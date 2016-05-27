/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[1024];
     struct sockaddr_in serv_addr, cli_addr;
     int n, rv;
     int yes = 1;
     fd_set readfds;
     struct timeval tv;

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

     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);

     newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
     if (newsockfd < 0)
          error("ERROR on accept");

    //clear the file descriptors
    FD_ZERO(&readfds);
    //add our sockfd to the fd set
    FD_SET(newsockfd, &readfds);
    tv.tv_sec = 0;
    tv.tv_usec = 100000;

    //file to temporarily store stuff
    FILE *fp;

    fp = fopen("temp", "w+");
    //loop to accept incoming messages
     while (1) {
         bzero(buffer,1024);
         rv = select(newsockfd+1, &readfds, NULL, NULL, &tv);
         if (rv == -1) {
             perror("select"); // error occurred in select()
         } else if (rv == 0) {
             break;
         } else {
             // if there is data to be read
             if (FD_ISSET(newsockfd, &readfds)) {
                 recv(newsockfd, buffer, 1024, 0);
                 fputs(buffer, fp);
             }
         }
     }
     fclose(fp);
     bzero(buffer,1024);
     //:q!printf("Here is the message: %s\n",buffer);
     n = write(newsockfd,"I got your message",18);
     if (n < 0) error("ERROR writing to socket");
     close(newsockfd);
     close(sockfd);
     return 0;
}
