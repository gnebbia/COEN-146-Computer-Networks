/*Anthony Fenzl
    Friday 2:15p Lab
    Lab 2
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
int main (int, char *[]);
/*********************
*  main
*********************/
int main (int argc, char *argv[]) {
    if(argc != 2){
        printf ("Usage: %s <port> \n",argv[0]);
        return -1;
    }

    int n;
    char file_name[10];
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    char buff[10];

    // set up
    memset (&serv_addr, '0', sizeof (serv_addr));
    memset (buff, '0', sizeof (buff));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl (INADDR_ANY);
    serv_addr.sin_port = htons (atoi(argv[1]));

    // create socket, bind, and listen
    listenfd = socket (AF_INET, SOCK_STREAM, 0);
    bind (listenfd, (struct sockaddr*)&serv_addr, sizeof (serv_addr));
    listen (listenfd, 10);

    // accept and interact
    connfd = accept (listenfd, (struct sockaddr*) NULL, NULL);

    // create new file with name given
    read(connfd, file_name, sizeof(file_name));
    FILE *dest = fopen(file_name, "wb");

    // write file
    while((n = read(connfd, buff, sizeof(buff))) > 0) {
        fwrite(buff, sizeof(char), n, dest);
    }

    // close connection and file
    close (connfd);
    fclose(dest);
}
