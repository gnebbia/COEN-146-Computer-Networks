/*Anthony Fenzl
    Friday 2:15p Lab
    Lab 5
*/

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>
#include <limits.h>

typedef struct {
	char name[50];
	char ip[50];
	int port;
}MACHINE;

#define update_min_wait 10
#define update_max_wait 20
int N = 4; //default
int costs[100][100];
int id;
int port;
int sock;
int in_data[3]; // host1, host2, weight
int out_data[3]; // host1, host2, weight
MACHINE hosts[100];
pthread_mutex_t lock;

void parse_files(FILE* f_costs, FILE* f_hosts);
void *receive_updates();
void *link_state();
void send_data();
int receive_data(int port);
void user_input_cost();


int main(int argc, char* argv[]) {
	if (argc != 5)
		printf ("Usage: %s <id> <n_machines> <costs_file> <hosts_file> \n",argv[0]);

	//scan data into memory
	sscanf(argv[1],"%d",&id);
	sscanf(argv[2],"%d",&N);

	FILE *f_costs;
	f_costs = fopen(argv[3], "r");
	FILE *f_hosts;
	f_hosts = fopen(argv[4], "r");

	pthread_mutex_init(&lock, NULL);

	parse_files(f_costs, f_hosts);
	port = hosts[id].port;

	struct sockaddr_in local_addr;
	socklen_t addr_size;
	
	// init 
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons ((short)port);
	local_addr.sin_addr.s_addr = htonl (INADDR_ANY);
	memset ((char *)local_addr.sin_zero, '\0', sizeof (local_addr.sin_zero));  
	addr_size = sizeof (local_addr);

	// make socket
	if ((sock = socket (AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf ("socket error\n");
		return 1;
	}
	
	// bind
	if (bind (sock, (struct sockaddr *)&local_addr, sizeof (local_addr)) != 0) {
		printf ("bind error\n");
		return 1;
	}
	
	//start threads
	pthread_t receive_thr;
	pthread_create(&receive_thr, NULL, receive_updates, NULL);

	pthread_t link_thr;
	pthread_create(&link_thr, NULL, link_state, NULL);


	for(int i = 0; i < 2;i++) {
		user_input_cost();
		sleep(10);
	}
}


void parse_files(FILE* f_costs, FILE* f_hosts) {
	
	printf("Parsing costs file:\n");
	for (int i = 0; i < N; i++) {	
		for (int j = 0; j < N; j++) {
			if ( fscanf(f_costs,"%d",&costs[i][j]) != 1)
				break;
			printf("%d ", costs[i][j]);
		}
		printf("\n");
	}

	printf("Parsing hosts file:\n");
	for (int i = 0; i < N; i++) {
		if (fscanf(f_hosts,"%s %s %d",&(hosts[i].name), &(hosts[i].ip), &(hosts[i].port)) < 1)
			break;
		printf("%s %s %d \n",(hosts[i].name), (hosts[i].ip), (hosts[i].port));
	}
	return;
}


void *receive_updates() {
	
	while(1) {
		receive_data(port);
		
		int host1 = ntohl(in_data[0]);
		int host2 = ntohl(in_data[1]);
		int weight = ntohl(in_data[2]);
	
		pthread_mutex_lock(&lock);
		costs[host1][host2] = weight;
		costs[host2][host1] = weight;

		for (int i = 0; i < N; i++) {			
			for (int j = 0; j < N; j++) {
				printf("%d ", costs[i][j]);
			}
			printf("\n");
		}
		pthread_mutex_unlock(&lock);
	}
}


void *link_state() {
	time_t last_update;
	last_update = time(NULL);

	while(1) {		
		int threshold = rand()%(update_max_wait - update_min_wait) + update_min_wait;
		if ((time(NULL) - last_update) > threshold) {  // use dijkstra to find best path
			int dist[N];
			int visited[N];
			int tmp_costs[N][N];
			pthread_mutex_lock(&lock);
			for (int source = 0; source < N; source++) { //initialize values
				for (int i = 0; i < N; i++)
					dist[i] = INT_MAX, visited[i] = 0;

				dist[source] = 0; // distance to self is always 0
			
				for (int count = 0; count < N-1; count++) {	
					int u = minDistance(dist, visited);
					visited[u] = 1;
			
					for (int v = 0; v < N; v++)  // assign costs
						if (visited[v]==0 && costs[u][v] && dist[u] != INT_MAX && dist[u]+costs[u][v] < dist[v])
		        			dist[v] = dist[u] + costs[u][v];
				}

				printf("distances computed in dijkstra from node %d: ",source);
				for (int i = 0; i < N; i++) {
					printf("%d ",dist[i]);
					tmp_costs[source][i] = dist[i];
					tmp_costs[i][source] = dist[i];
				}
				printf("\n");
			}
			printf("\n");
			pthread_mutex_unlock(&lock);
			last_update = time(NULL);
		}
	}
}


int minDistance(int dist[], int visited[]) {
   int min = INT_MAX, min_index;
  
   for (int v = 0; v < N; v++)
     if (visited[v] == 0 && dist[v] < min)
         min = dist[v], min_index = v;
  
   return min_index;
}


void send_data() {
	int sock;
	struct sockaddr_in destAddr[N];
	socklen_t addr_size[N];

	for (int i = 0; i < N; i++) {  // configure addresses
		destAddr[i].sin_family = AF_INET;
		destAddr[i].sin_port = htons (hosts[i].port);
		inet_pton (AF_INET, hosts[i].ip, &destAddr[i].sin_addr.s_addr);
		memset (destAddr[i].sin_zero, '\0', sizeof (destAddr[i].sin_zero));  
		addr_size[i] = sizeof destAddr[i];
	}

	sock = socket (PF_INET, SOCK_DGRAM, 0);  // open socket
	
	for (int i = 0; i < N; i++) {
		if (i != id)
			sendto (sock, &out_data, sizeof(out_data), 0, (struct sockaddr *)&(destAddr[i]), addr_size[i]);
	}
}


int receive_data(int port) {
	
	int nBytes = recvfrom (sock, &in_data, sizeof(in_data), 0, NULL,NULL);
	printf("received update\n");
	
	return 0;
}


void user_input_cost() {
	int neighbor;
	int new_cost;

	printf("Update neigbor cost from node %d, format <neighbor> <new cost>:\n", id);
	
	scanf("%d %d",&neighbor,&new_cost);

	pthread_mutex_lock(&lock);
	costs[id][neighbor] = new_cost;
	costs[neighbor][id] = new_cost;
	out_data[0] = htonl(id);
	out_data[1] = htonl(neighbor);
	out_data[2] = htonl(new_cost);
	send_data();


	printf("new matrix after user input:\n");
	for (int i = 0; i < N; i++) {			
		for (int j = 0; j < N; j++) {
			printf("%d ", costs[i][j]);
		}
		printf("\n");
	}
	pthread_mutex_unlock(&lock);
	
}
