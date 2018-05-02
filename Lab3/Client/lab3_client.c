/*Anthony Fenzl
    Friday 2:15p Lab
    Lab 3
*/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

typedef struct {
    int sequence_ack;  // 4B
    int length;  // 4B
} HEADER;

typedef struct {
    HEADER header;
    char data[10];
} PACKET;

/***********
 *  main
 ***********/
int main (int argc, char *argv[]) {
	int sock;
	char buffer[1024];
    PACKET send_pack;
    PACKET ack_pack;
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

    // make packet to send filename
    strcpy(send_pack.data, argv[4]);
    send_pack.header.sequence_ack = 0;

    do {  // send filename to server, repeat if wrong ack

		// send filename
		printf ("Sending: %s\n", send_pack.data);
		sendto (sock, &send_pack,  sizeof(PACKET), 0, (struct sockaddr *)&serverAddr, addr_size);

        //  receive ack
        recvfrom (sock, &ack_pack, sizeof(PACKET), 0, NULL, NULL);
        if (ack_pack.header.sequence_ack == send_pack.header.sequence_ack)  // if ack, break
            break;
    } while (1);

    // open file to read
    FILE *src = fopen(argv[3], "rb");

    // step sequence
    send_pack.header.sequence_ack = (send_pack.header.sequence_ack + 1) % 2;

    // read in data
    fread(send_pack.data, sizeof(char), 10, src);

    // set size
    send_pack.header.length = sizeof(char) * 10;

	do {
        printf("seinding: %s\n", send_pack.data);
        sendto (sock, &send_pack,  sizeof(PACKET), 0, (struct sockaddr *)&serverAddr, addr_size);

        recvfrom (sock, &ack_pack, sizeof(PACKET), 0, NULL, NULL);
        if (ack_pack.header.sequence_ack == send_pack.header.sequence_ack) {// if ack, send next
            fread(send_pack.data, sizeof(char), 10, src);
            send_pack.header.sequence_ack = (send_pack.header.sequence_ack + 1) % 2;
        }
    } while(!feof(src));

    // set packet for close
    strcpy(send_pack.data, "");
    send_pack.header.length = 0;
    send_pack.header.sequence_ack = (send_pack.header.sequence_ack + 1) % 2;

    do {  // send close packet to server, repeat if wrong ack
        printf("seinding: %s\n", send_pack.data);
		sendto (sock, &send_pack,  sizeof(PACKET), 0, (struct sockaddr *)&serverAddr, addr_size);

        //  receive ack
        recvfrom (sock, &ack_pack, sizeof(PACKET), 0, NULL, NULL);
        if (ack_pack.header.sequence_ack == send_pack.header.sequence_ack)  // if ack, break
            break;
    } while (1);

    close(sock);
    fclose(src);

	return 0;
}
