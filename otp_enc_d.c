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
char* encode(int connection, char *plainfile, char *cipherfile);
char* getFile(int connection, char* name);
void send_file(char* filename, int connectionFd);


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

    return 0;
}


void handleIncoming(int connection){

    //identify yourself!!
    write(connection, "enc", 3);

    char *plaintext, *keytext, *enciphered;
    //char buffer[1024];
    int n;

    //first call to getFile stores plaintext in a temp file
    plaintext = getFile(connection, "temp_plaintext");
    //let client know we got the file
    n = write(connection,"confirmed", 9);
    if (n < 0) error("ERROR writing to socket");
    //second call to getFile stores the keytext
    keytext = getFile(connection, "temp_keytext");

    //call encode and pass it the new tempfiles and the connection
    enciphered = encode(connection, plaintext, keytext);

    send_file(enciphered, connection);

    close(connection); //no longer need this

    //*clean up*
    remove(plaintext);
    remove(keytext);
    remove(enciphered);

}

/* func: encode
 * takes int for socket, 2 strings for filenames of plain text and the key text
 * returns filename for newly created encoded textfile
 */
char* encode(int connection, char *plainfile, char *cipherfile){
    FILE *fp;
    fp = fopen(plainfile, "r");
    char ch;

    FILE *fl;
    fl = fopen(cipherfile, "r");
    char ch2;

    //set up one last file to hold the enciphered text
    FILE *temp;
    int size = 22;
    char *tempname = malloc(size);
    int pid = getpid();
    snprintf(tempname, size, "%s%d", "enciphered", pid);
    temp = fopen(tempname, "a");
    char ch3;

    //start with first chars from both files
    ch = getc(fp);
    ch2 = getc(fl);
    ch3 = getc(temp);

    char alpha[27];
    sprintf(alpha, "ABCDEFGHIJKLMNOPQRSTUVWXYZ ");
    int i;
    //we don't really need to check for ch2 end of file since key should be longer
    while (ch != EOF) {
        //get the index of the current ch
        int msgkey = 0;
        for (i = 0; i < 27; i++) {
            if (ch == alpha[i]) {
                msgkey = msgkey + i + 1;
            }
            if (ch2 == alpha[i]) {
                msgkey = msgkey + i + 1;
            }
        }//end of for loop
        //perform modulo
        int r = msgkey % 27;
        if (r > 27) {
          r = r - 27;
        }
        //printf("%c\n", alpha[r]);
        fputc(alpha[r], temp);

        //move the chain
        ch = getc(fp);
        ch2 = getc(fl);
        ch3 = getc(temp);
    }

    fclose(fp);
    fclose(fl);
    fclose(temp);

    return tempname;
}

/* func: getFile
 * rcvs connection socket, string for name of temp file to creat
 * this function is used twice since it is generic enough
 * returns name of newly created file containing text
 */
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
        //printf("%s\n", buffer);
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

/* func: send_file
 * rcvs: string for filename, int for socket
 * opens file and sends its contents over the socket
 */
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
        while ((nbytes = fread(ret, sizeof(ret[0]), 1024, fl)) > 0) {
        send(connectionFd, ret, nbytes, 0);
        //printf(ret);
        }
        fclose(fl);
        //printf("done sending the goods\n");
    }
}
