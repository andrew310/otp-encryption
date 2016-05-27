#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_BUFFER 1024

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void send_file(char** args, int connectionFd);

int main(int argc, char *argv[])
{

    //input check
    if (argc < 4) {
        //print to standard error
       fprintf(stderr,"usage %s <text file> <keyfile> <daemon port>\n", argv[0]);
       exit(0);
    }

    //socket variables
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[MAX_BUFFER];
    portno = atoi(argv[3]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    printf("Please enter the message: ");
    //bzero(buffer,256);

    //READ INPUT FILE
    //fgets(buffer,MAX_BUFFER,stdin);
    send_file(argv, sockfd);
    //n = write(sockfd,buffer,strlen(buffer));
    // if (n < 0)
    //      error("ERROR writing to socket");
    bzero(buffer,MAX_BUFFER);
    n = read(sockfd,buffer,MAX_BUFFER);
    if (n < 0)
         error("ERROR reading from socket");
    printf("%s\n",buffer);
    close(sockfd);
    return 0;
}

void send_file(char** args, int connectionFd){
    printf("opening file: %s\n", args[1]);
    char* filename = args[1];
    FILE *fl = fopen(filename, "r");
    if (fl == NULL) {
        fprintf(stderr,"Error opening '%s': No such file or directory\n", args[1]);
        exit(1);
    } else {
        printf("opened %s successfully, reading...\n", args[1]);
        fseek(fl, 0, SEEK_END);
        long len = ftell(fl);
        char *ret = (char*)malloc(len);
        fseek(fl, 0, SEEK_SET);
        printf("sending the goods...\n");
        size_t nbytes = 0;
        //send the file 500 bytes at a time
        while ((nbytes = fread(ret, sizeof(ret[0]), MAX_BUFFER, fl)) > 0) {
        send(connectionFd, ret, nbytes, 0);
        printf(ret);
        }
        fclose(fl);
        printf("done sending the goods\n");
    }
}
