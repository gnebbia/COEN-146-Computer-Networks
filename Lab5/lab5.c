/*Anthony Fenzl
    Friday 2:15p Lab
    Lab 5
*/

#include <stdio.h>
#include <string.h>
#include <pthread.h>
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
int N = 4;
int costs[100][100];
int id;
int port;
int sock;
int in_data[3];  // host, host, weight
int out_data[3];  // host, host, weight
MACHINE hosts[100];
pthread_mutex_t lock;

void parse_files(FILE* f_costs, FILE* f_hosts_);
void *receive_updates();
void *link_state();
void send_data();
int receive_data(int port);
void user_input_cost();

int main(int argc, char* argv[]) {
    if (argc != 5)
        printf ("Usage: %s <id> <num_machines> <costs_file> <machines_file> \n", argv[0]);

    sscanf(argv[1], "%d", &id);
    sscanf(argv[2], "%d", &N);

    FILE *f_costs;
    f_costs = fopen(argv[3], "r");
    FILE *f_hosts;
    f_hosts = fopen(argv[4], "r");

    pthread_mutex_intit(&lock, NULL);

    parse_files(f_costs, f_hosts);
    port = hosts[id].port;

    struct sockaddr_in local_addr, addr;
    struct sockaddr_storage storage;
    socklen_t local_addr_size, addr_size;

    // init
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons((short)port);
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset((char *)local_addr.sin_zero, '\0', sizeof(local_addr.sin_zero));
    addr_size = sizeof(storage);

    // make socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("socket error\n");
        return -1;
    }

    // bind
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("socket error\n");
        return -1;
    }

    // thread start
    pthread_t receive_thr;
    pthread_create(&receive_thr, NULL, receive_update, NULL);

    pthread_t link_thr;
    pthread_create(&link_thr, NULL, link_state, NULL);

    for (int i = 0; i < 2; i++) {
        user_input_cost();
        sleep(10);
    }
}


void parse_files (FILE* f_costs, FILE* f_hosts) { 
    printf("parsing costs file:\n");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++){
            if (fscan(f_costs, "%d", &costs[i][j]) != 1)
                break;
            printf("d", costs[i][j]);
        }
        printf("\n");
    }

    printf("parsing hosts file:\n");
    for(int i = 0; i < N; i++) {
        if (fscanf(f_hosts, "%s %s %d", &(hosts[i].name), &(hosts[i].ip), &(hosts[i].port)) < 1)
            break;
        printf("%s %s %d \n", &(hosts[i].name), &(hosts[i].ip), &(hosts[i].port));
    }
    return;
}


void *receive_updates() {
    while (1) {
        reveive_data(port);

        int host1 = ntohl(in_data[0]);
        int host2 = ntohl(in_data[1]);
        int weight = ntohl(in_data[2]);

        pthread_mutex_lock(&lock);
        costs[host1][host2] = weight;
        costs[host2][host1] = weight;

        for (int i = 0; i < N; i++) {
            for (int j = 0; i < N; j++) {
                printf("%d", costs[i][j]);
            }
            printf("\n");
        }
        pthread_mutex_unlock(&lock);
    }
}


void *link_state() {
    time_t last_update;
    last_update = time(NULL);

    while (1) {
        int threashold = rand()%(update_max_wait - update_min_wait) + update_min_wait;
        if ((time(NULL) - last_update) > threashold) {
            int dist[N];
            int visited[N];
            int tmp_costs[N][N];
            pthread_mutex_lock(&lock);
            for (int source = 0; source < N; source++) {

            }
        }
    }
}
