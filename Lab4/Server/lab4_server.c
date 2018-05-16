/*Anthony Fenzl
    Friday 2:15p Lab
    Lab 4
*/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    int sequence_ack;  // sequence number for data and for ACK
    int length;  // length of the data in bytes (zero for ACK)
    int check;  // checksum calculated by byte
    int fin;  // 1 for the last packet
} HEADER;

typedef struct {
    HEADER header;
    char data[10];
} PACKET;

int checksum (PACKET* pack) {
    (*pack).header.check = 0;
    char* head = (char*)pack;
    char sum = 0;

    for (int i = 0; i < sizeof (PACKET); i++) {
        sum = sum ^ (*head);
        ++head;
    }

    return (int)sum;
}

int error () {
    return rand()%100 > 80;
}

/********************
 * main
 ********************/
int main(int argc, char *argv[]) {
	int sock, check, state;
    FILE *dest;
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

    // init state
    state = 0;

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
    do {
        recvfrom(sock, &receive_pack, sizeof(PACKET), 0, (struct sockaddr *)&serverStorage, &addr_size);
        if (error()) {
            printf("OPEN FILE ACK DROP\n");
            continue;
        }
        check = receive_pack.header.check;
        if (check == checksum(&receive_pack) && receive_pack.header.sequence_ack == state) {
            dest = fopen(receive_pack.data, "wb");
            ack_pack.header.sequence_ack = state;
            sendto(sock, &ack_pack, sizeof(PACKET), 0, (struct sockaddr *)&serverStorage, addr_size);
            state = (state + 1 ) % 2;
            break;
        } else {
            ack_pack.header.sequence_ack = (state + 1) % 2;
            sendto(sock, &ack_pack, sizeof(PACKET), 0, (struct sockaddr *)&serverStorage, addr_size);
        }
    } while(1);


	while (1) {
		// receive  datagrams
	    recvfrom(sock, &receive_pack, sizeof(PACKET), 0, (struct sockaddr *)&serverStorage, &addr_size);

        if (error()) {
            printf("DATA ACK DROP\n");
            continue;
        }

		// ack
        check = receive_pack.header.check;
        if (check == checksum(&receive_pack) &&
            receive_pack.header.sequence_ack == state) {
            ack_pack.header.sequence_ack = state;
            sendto(sock, &ack_pack, sizeof(PACKET), 0, (struct sockaddr *)&serverStorage, addr_size);
            // if close pack, break
            if (receive_pack.header.fin == 1)
                break;
            state = (state + 1) % 2;
            fwrite(receive_pack.data, sizeof(char), receive_pack.header.length, dest);
        } else {
            printf("BAD CHECKSUM OR SEQ\n");
            ack_pack.header.sequence_ack = (receive_pack.header.sequence_ack + 1) % 2;
            sendto(sock, &ack_pack, sizeof(PACKET), 0, (struct sockaddr *)&serverStorage, addr_size);
        }
	}

    close(sock);
    fclose(dest);

	return 0;
}
