/*Anthony Fenzl
    Friday 2:15p Lab
    Lab 2
*/

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
int main (int, char *[]);
/********************
*  main
********************/
int main (int argc, char *argv[]) {
    int size;
    int sockfd;
    char buff[10];
    struct sockaddr_in serv_addr;
    if (argc != 5) {
        printf ("Usage: %s <input filename> <output filename> <ip> <port> \n",argv[0]);
        return -1;
    }

    // set up
    memset (buff, '0', sizeof (buff));
    memset (&serv_addr, '0', sizeof (serv_addr));

    // open socket
    if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
        printf ("Error : Could not create socket \n");
        return -1;
    }

    // set address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons (atoi(argv[4]));
    if (inet_pton (AF_INET, argv[3], &serv_addr.sin_addr) <= 0) {
        printf ("inet_pton error occured\n");
        return -1;
    }

    // connect
    if (connect (sockfd, (struct sockaddr *)&serv_addr, sizeof (serv_addr)) < 0) {
        printf ("Error : Connect Failed \n");
        return -1;
    }

    // send name of file to create
    write (sockfd, argv[2], strlen(argv[2]) + 1);

    // open file to read
    FILE *src = fopen(argv[1], "rb");

    // send file
    while(!feof(src)) {
        size = fread(buff, sizeof(char), 10, src);
        write(sockfd, buff, size);
    }

    // close file and connection
    close (sockfd);
    fclose(src);
    return 1;
}
