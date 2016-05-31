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

void send_file(char* filename, int connectionFd);

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

    //open keyfile and get its length
    FILE *fl = fopen(argv[2], "r");
    fseek(fl, 0, SEEK_END);
    long len = ftell(fl);
    fseek(fl, 0, SEEK_SET);
    fclose(fl);

    //open plaintext and get its length
    FILE *fp;
    fp = fopen(argv[1], "r");
    fseek(fp, 0, SEEK_END);
    long len2 = ftell(fl);
    fseek(fp, 0, SEEK_SET);

    //compare two lengths
    if (len2 > len) {
      printf("ERROR, key is too short\n");
      exit(1);
    }

    //error checking
    char ch = getc(fp);
    while (ch != EOF) {
      if (!isupper(ch) && !isspace(ch)) {
          printf("ERROR: bad char %c\n", ch);
          exit(1);
      }

      ch = getc(fp);
    }
    fclose(fp);

    n = read(sockfd,buffer,MAX_BUFFER);
    if (n < 0)
         error("ERROR reading from socket");
    //printf("%s\n",buffer);
    if (strncmp(buffer, "enc", 4) != 0) {
        printf("Only use otp_enc with otp_enc_d \n");
        close(sockfd);
        exit(0);
    }

    //SEND PLAINTEXT FILE
    send_file(argv[1], sockfd);
    bzero(buffer,MAX_BUFFER);
    n = read(sockfd,buffer,MAX_BUFFER);
    if (n < 0)
         error("ERROR reading from socket");
    //printf("%s\n",buffer);


    //SEND CIPHER FILE
    send_file(argv[2], sockfd);
    bzero(buffer,MAX_BUFFER);
    int numbytes = 0;
    //loop to print out enciphered text in case it is a big one
    while ((numbytes = recv(sockfd, buffer, 1024, 0)) > 0) {
        printf("%s", buffer);
        bzero(buffer, 1024);
        //if less than 1024 we are on the last chunk, so break
        if (numbytes < 1024) {
          break;
        }
    }

    return 0;
}

void send_file(char* filename, int connectionFd){
    //printf("opening file: %s\n", filename);
    FILE *fl = fopen(filename, "r");
    if (fl == NULL) {
        fprintf(stderr,"Error opening '%s': No such file or directory\n", filename);
        exit(1);
    } else {
        //printf("opened %s successfully, reading...\n", filename);
        fseek(fl, 0, SEEK_END);
        long len = ftell(fl);
        char *ret = (char*)malloc(len);
        fseek(fl, 0, SEEK_SET);
        //printf("sending the goods...\n");
        size_t nbytes = 0;
        //send the file 500 bytes at a time
        while ((nbytes = fread(ret, sizeof(ret[0]), MAX_BUFFER, fl)) > 0) {
        send(connectionFd, ret, nbytes, 0);
        //printf(ret);
        }
        fclose(fl);
        //printf("done sending the goods\n");
    }
}
