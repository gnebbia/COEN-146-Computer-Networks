/*****************************
 * COEN 146, UDP, client
 *****************************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

/***********
 *  main
 ***********/
int main (int argc, char *argv[])
{
	int sock, portNum, nBytes;
	char buffer[1024];
	struct sockaddr_in serverAddr;
	socklen_t addr_size;

	if (argc != 3)
	{
		printf ("need the port number and machine\n");
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

	while (1)
	{
		printf ("String:");
		fgets(buffer, 1024, stdin);

		nBytes = strlen (buffer) + 1;
    
		// send
		printf ("Sending: %s", buffer);
		sendto (sock, buffer, nBytes, 0, (struct sockaddr *)&serverAddr, addr_size);

		// receive
		nBytes = recvfrom (sock, buffer, 1024, 0, NULL, NULL);
		printf ("Received: %s\n", buffer);

	}

	return 0;
}
