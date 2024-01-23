#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <SDL2/SDL.h>
#define PORT 12345
#define GRID_SIZE 10
#define DEBRIS_COUNT 5
struct Debris {
    int x, y;
};
void sendDebrisPositions(int sockfd, struct sockaddr_in *clientAddr) {
    struct Debris debris[DEBRIS_COUNT];
    for (int i = 0; i < DEBRIS_COUNT; i++) {
        debris[i].x = rand() % GRID_SIZE;
        debris[i].y = rand() % GRID_SIZE;
    }
    sendto(sockfd, debris, sizeof(debris), 0, (struct sockaddr *)clientAddr, sizeof(*clientAddr));
    }

int main() {
    int sockfd;
    struct sockaddr_in cliaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    memset(&cliaddr, 0, sizeof(cliaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_addr.s_addr = inet_addr("192.168.1.213"); // Imposta l'indirizzo IP del client
    cliaddr.sin_port = htons(PORT);

    srand(time(0)); // Inizializzazione del generatore di numeri casuali

    while (1) {
        printf("Sending debris positions...\n");
        sendDebrisPositions(sockfd, &cliaddr);
        sleep(2); // Wait for 2 seconds
    }
    close(sockfd);
    return 0;
}
