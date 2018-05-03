/*Anthony Fenzl
    Friday 2:15p Lab
    Lab 3
*/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    int sequence_ack;  // 4B
    int length;  // 4B
} HEADER;

typedef struct {
    HEADER header;
    char data[10];
} PACKET;

/********************
 * main
 ********************/
int main(int argc, char *argv[]) {
	int sock;
    PACKET receive_pack, ack_pack;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;

    if(argc != 2) {
        printf("need the port number\n");
        return 1;
    }

	// init
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons((short)atoi (argv[1]));
	serverAddr.sin_addr.s_addr = htonl (INADDR_ANY);
	memset((char *)serverAddr.sin_zero, '\0', sizeof (serverAddr.sin_zero));
	addr_size = sizeof (serverStorage);

	// create socket
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf ("socket error\n");
		return 1;
	}

	// bind
	if (bind(sock, (struct sockaddr *)&serverAddr, sizeof (serverAddr)) != 0) {
		printf ("bind error\n");
		return 1;
	}

    // Accept and open file
    recvfrom(sock, &receive_pack, sizeof(PACKET), 0, (struct sockaddr *)&serverStorage, &addr_size);
    FILE *dest = fopen(receive_pack.data, "wb");
    ack_pack.header.sequence_ack = receive_pack.header.sequence_ack;
    sendto(sock, &ack_pack, sizeof(PACKET), 0, (struct sockaddr *)&serverStorage, addr_size);


	while (1) {
		// receive  datagrams
	    recvfrom(sock, &receive_pack, sizeof(PACKET), 0, (struct sockaddr *)&serverStorage, &addr_size);

		// ack
        ack_pack.header.sequence_ack = receive_pack.header.sequence_ack;
        sendto(sock, &ack_pack, sizeof(PACKET), 0, (struct sockaddr *)&serverStorage, addr_size);

        // if close pack, break
        if (receive_pack.header.length == 0)
            break;

        // if data, write to file
        fwrite(receive_pack.data, sizeof(char), receive_pack.header.length, dest);
	}

    close(sock);
    fclose(dest);

	return 0;
}
