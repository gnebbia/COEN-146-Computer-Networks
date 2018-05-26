/*Anthony Fenzl
    Friday 2:15p Lab
    Lab 4
*/


#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
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
    return rand()%100 < 20;
}

/***********
 *  main
 ***********/
int main (int argc, char *argv[]) {
	int sock, state, fin_count;
    PACKET send_pack, ack_pack;
	struct sockaddr_in serverAddr;
	socklen_t addr_size;

	if (argc != 5)
	{
		printf ("Usage: a.out <port> <ip> <read filename> <write filename>\n");
		return 1;
	}

	// configure address
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons (atoi (argv[1]));
	inet_pton (AF_INET, argv[2], &serverAddr.sin_addr.s_addr);
	memset (serverAddr.sin_zero, '\0', sizeof (serverAddr.sin_zero));
	addr_size = sizeof serverAddr;

	/*Create UDP socket*/
	sock = socket (PF_INET, SOCK_DGRAM, 0);

    /*Setup select for timeouts*/
    struct timeval tv;  // timer
    int rv;  // select returned value
    fd_set readfds;
    fcntl (sock, F_SETFL, O_NONBLOCK);

    // init state
    state = 0;

    // make packet to send filename
    strcpy(send_pack.data, argv[4]);
    send_pack.header.sequence_ack = state;
    send_pack.header.length = sizeof(send_pack.data);
    send_pack.header.fin = 0;

    do {  // send filename to server, repeat if wrong ack
        send_pack.header.check = checksum(&send_pack);

        if (error()) {
            printf("SENDING WRONG FILENAME CHECKSUM\n");
            send_pack.header.check = 0;
        }
        // send filename
		sendto (sock, &send_pack,  sizeof(PACKET), 0, (struct sockaddr *)&serverAddr, addr_size);

        // start timer
        FD_ZERO (&readfds);
        FD_SET (sock, &readfds);
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        rv = select (sock + 1, &readfds, NULL, NULL, &tv);
        //  receive ack
        if (rv) {
            recvfrom (sock, &ack_pack, sizeof(PACKET), 0, NULL, NULL);
            if (ack_pack.header.sequence_ack == state){  // if ack, break
                state = (state + 1) % 2;
                break;
            } else {
                printf("PACKET DROP\n");
            }
        }
    } while (1);

    // open file to read
    FILE *src = fopen(argv[3], "rb");

	do {
        send_pack.header.length = fread(send_pack.data, sizeof(char), 10, src);
        send_pack.header.sequence_ack = state;

        while (1) {  // if ack, set up new pack
            send_pack.header.check = checksum(&send_pack);

            if (error()) {
                printf("SENDING WRONG DATA CHECKSUM\n");
                send_pack.header.check = 0;
            }

            sendto (sock, &send_pack,  sizeof(PACKET), 0, (struct sockaddr *)&serverAddr, addr_size);

            // start timer
            FD_ZERO (&readfds);
            FD_SET (sock, &readfds);
            tv.tv_sec = 2;
            tv.tv_usec = 0;
            rv = select (sock + 1, &readfds, NULL, NULL, &tv);
            if (rv) {
                recvfrom (sock, &ack_pack, sizeof(PACKET), 0, NULL, NULL);
                if (ack_pack.header.sequence_ack == state)
                    break;
            } else {
                printf("PACKET DROP\n");
            }
        }
        state = (state + 1) % 2;

    } while(!feof(src));

    // set packet for close
    send_pack.header.length = 0;
    send_pack.header.sequence_ack = (send_pack.header.sequence_ack + 1) % 2;
    send_pack.header.fin = 1;
    fin_count = 0;

    do {  // send close packet to server, repeat if wrong ack
        send_pack.header.check = checksum(&send_pack);

        if (error()) {
            printf("SENDING WRONG FIN CHECKSUM\n");
            send_pack.header.check = 0;
        }

		sendto (sock, &send_pack,  sizeof(PACKET), 0, (struct sockaddr *)&serverAddr, addr_size);
        ++fin_count;
        if (fin_count == 3)
            break;
        // start timer
        FD_ZERO (&readfds);
        FD_SET (sock, &readfds);
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        rv = select (sock + 1, &readfds, NULL, NULL, &tv);

        //  receive ack
        if (rv){
            recvfrom (sock, &ack_pack, sizeof(PACKET), 0, NULL, NULL);
            if (ack_pack.header.sequence_ack == state)  // if ack, break
                break;
        } else {
            printf("PACKET DROP\n");
        }
    } while (1);

    close(sock);
    fclose(src);

	return 0;
}
